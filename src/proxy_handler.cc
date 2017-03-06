#include "proxy_handler.h"

#include <string>

RequestHandler::Status ProxyHandler::Init(const std::string& uri_prefix,
    const NginxConfig& config)
{
  uri_prefix_ = uri_prefix;

  int server_location = 0;
  if (config.statements_[0]->tokens_[0] == "remote_port") {
    server_location = 1;
  }
  remote_host_ =
    config.statements_[server_location]->tokens_[1];
  remote_port_ =
    config.statements_[1-server_location]->tokens_[1];

  return RequestHandler::OK;
}

RequestHandler::Status ProxyHandler::HandleRequest(const Request& request,
    Response* response)
{
  if (remote_host_.size() == 0 || remote_port_.size() == 0) {
    std::cerr << "ProxyHandler::HandleRequest: remote_host_ or remote_port_ ";
    std::cerr << "are empty. Cannot serve request." << std::endl;
    return RequestHandler::Error;
  }

  Request proxied_req(request);
  proxied_req.set_uri(request.uri().substr(uri_prefix_.size()));
  if (proxied_req.uri().size() == 0) {
    proxied_req.set_uri("/");
  }
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

  // copy parsed response into given response
  *response = ParseRawResponse(response_str);

  if (response->status() == Response::code_302_found ||
      response->status() == Response::code_301_moved) {
    Request redirect_request(request);
    redirect_request.set_uri(redirect_uri_);
    redirect_request.remove_header("Host");
    redirect_request.add_header("Host", redirect_host_);
    uri_prefix_ = "";
    return HandleRequest(redirect_request, response);
  }

  return RequestHandler::OK;
}

// TODO: this assume raw_response is valid HTTP
// TODO: factor this out into a more appropriate location
Response ProxyHandler::ParseRawResponse(const std::string& raw_response)
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

    case 301:
      rc = Response::code_301_moved;
      break;
    case 302:
      rc = Response::code_302_found;
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

    if ((rc == Response::code_302_found || rc == Response::code_301_moved) &&
        header_name == "Location") {
      if (header_value[0] == '/') { // new URI on same host
        redirect_uri_ = header_value;
      } else { // new host
        // extract new host, uri
        size_t uri_pos = 0;
        remote_host_ = header_value;
        if (remote_host_.find("http://") == 0) {
          remote_host_ = remote_host_.substr(strlen("http://"));
          uri_pos += strlen("http://");
        }
        if (remote_host_[remote_host_.size()-1] == '/') {
          remote_host_.resize(remote_host_.size()-1);
        }
        uri_pos = header_value.find("/", uri_pos);
        redirect_host_ = header_value.substr(0, uri_pos);
        redirect_uri_ = header_value.substr(uri_pos);
      }
    }

    response.AddHeader(header_name, header_value);
  }

  response.SetBody(raw_response.substr(start, raw_response.size() - start));

  return response;
}
