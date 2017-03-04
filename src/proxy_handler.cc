#include "proxy_handler.h"

#include <string>

RequestHandler::Status ProxyHandler::Init(const std::string& uri_prefix,
    const NginxConfig& config)
{
  uri_prefix_ = uri_prefix;

  for (auto const& stmt : config.statements_) {
    if (stmt->tokens_.size() == 3 && stmt->tokens_[2] == "ProxyHandler" &&
        stmt->tokens_[1] == uri_prefix) {
      int server_location = 0; // which statement in the child block the
          // server is in
      if (stmt->child_block_->statements_[0]->tokens_[0] == "remote_port") {
        server_location = 1;
      }
      remote_host_ =
        stmt->child_block_->statements_[server_location]->tokens_[1];
      remote_port_ =
        stmt->child_block_->statements_[1-server_location]->tokens_[1];
    }
  }
  return RequestHandler::OK;
}

RequestHandler::Status ProxyHandler::HandleRequest(const Request& request,
    Response* response)
{
  Request proxied_req(request);
  proxied_req.set_uri(request.uri().substr(uri_prefix_.size(),
        request.uri().size()));
  // make sure keep alive is off
  proxied_req.remove_header("Connection");
  proxied_req.add_header("Connection", "Close");
  // update host
  proxied_req.remove_header("Host");
  proxied_req.add_header("Host", remote_host_ + ":" + remote_port_);

  client_ = new (std::nothrow) SyncClient();
  if (client_ == nullptr) {
    std::cerr << "ProxyHandler::HandleRequest failed to allocate AsyncClient.";
    std::cerr << std::endl;
    return RequestHandler::Error;
  }

  if (!client_->Connect(remote_host_, remote_port_)) {
    std::cerr << "ProxyHandler::HandleRequest failed to connect to ";
    std::cerr << remote_host_ << ":" << remote_port_ << std::endl;
    return RequestHandler::Error;
  }

  if (!client_->Write(proxied_req.raw_request())) {
    std::cerr << "ProxyHandler::HandleRequest failed to write to ";
    std::cerr << remote_host_ << ":" << remote_port_ << std::endl;
    return RequestHandler::Error;
  }

  std::string response_str;
  if (!client_->Read(response_str)) {
    std::cerr << "ProxyHandler::HandleRequest failed to read from ";
    std::cerr << remote_host_ << ":" << remote_port_ << std::endl;
    return RequestHandler::Error;
  }

  return RequestHandler::OK;
}

// TODO: this assume raw_response is valid HTTP
Response ParseRawResponse(const std::string& raw_response)
{
  Response response;

  // for a given substring, start points to first char, end points to 1
  // position past the end
  size_t start = 0, end;

  // get response code
  start = raw_response.find(" ") + 1; // skip version
  end = raw_response.find(" ", start);
  int raw_response_code = std::stoi(raw_response.substr(start, end - start));
  Response::ResponseCode rc;
  switch(raw_response_code) {
    case 200:
      rc = Response::code_200_OK;
      break;

    case 302:
      // TODO: carry out redirect
      break;

    case 400:
      rc = Response::code_400_bad_request;
      break;
    case 401:
      rc = Response::code_401_unauthorized;
      break;
    case 403:
      rc = Response::code_403_forbidden;
      break;
    case 404:
      rc = Response::code_404_not_found;
      break;

    case 500:
      rc = Response::code_500_internal_error;
      break;
  }
  response.SetStatus(rc);

  // get all headers
  std::string header_name, header_value;
  end = raw_response.find("\r\n", start);
  while (true) {
    start = end + 2;

    // if next \r\n is right after current one, we're done with the headers
    if (raw_response.find("\r\n", start) == start) {
      start += 2;
      break;
    }

    end = raw_response.find(" ", start);
    header_name = raw_response.substr(start, end - start);
    if (header_name[header_name.size() - 1] == ':') {
      header_name.resize(header_name.size() - 1);
    }

    start = end + 1;
    end = raw_response.find("\r\n", start);
    header_value = raw_response.substr(start, end - start);

    response.AddHeader(header_name, header_value);
  }

  response.SetBody(raw_response.substr(start, raw_response.size() - start));

  return response;
}
