#include "sync_client.h"

SyncClient::BoostGuts::BoostGuts()
{
  io_service_ = std::unique_ptr<boost::asio::io_service>(new
      boost::asio::io_service());
  resolver_ = std::unique_ptr<tcp::resolver>(new
      tcp::resolver(*io_service_));
  socket_ = std::unique_ptr<tcp::socket>(new tcp::socket(*io_service_));
}

SyncClient::BoostGuts::BoostGuts(std::unique_ptr<boost::asio::io_service> io_service,
    std::unique_ptr<tcp::resolver> resolver,
    std::unique_ptr<tcp::socket> socket)
{
  io_service_ = std::move(io_service);
  resolver_ = std::move(resolver);
  socket_ = std::move(socket);
}

bool SyncClient::Connect(const std::string& server, const std::string& port)
{
  std::cerr << "server:port: " << server << ":" << port << std::endl;
  tcp::resolver::query query(server, port);
  tcp::resolver::iterator endpoint_iterator =
    boost_guts_->resolver_->resolve(query);
  tcp::resolver::iterator end;

  boost_guts_->socket_->close();

  // Try each endpoint until we successfully establish a connection.
  boost::system::error_code error = boost::asio::error::host_not_found;
  while (error && endpoint_iterator != end)
  {
    boost_guts_->socket_->close();
    boost_guts_->socket_->connect(*endpoint_iterator++, error);
  }

  if (error) {
    std::cerr << "Failed to connect to server. Reason: " << error.message();
    std::cerr << std::endl;
    return false;
  }

  return true;
}

bool SyncClient::Write(const std::string& message)
{
  boost::asio::streambuf request;
  std::ostream request_stream(&request);
  request_stream << message;

  std::cerr << "sending msg:" << std::endl;
  std::cerr << message;

  // TODO: stream the response if it doesn't all send at first, and give up
  // after a reasonable amount of time / number of attempts.
  size_t bytes_sent = boost::asio::write(*boost_guts_->socket_, request);
  if (bytes_sent != message.size()) {
    std::cerr << "Warning: SyncClient failed to send entire message to ";
    std::cerr << "remote host. Sent " << bytes_sent << " out of ";
    std::cerr << message.size() << " bytes." << std::endl;
    return false;
  }

  return true;
}

bool SyncClient::Read(std::string& response)
{
  boost::asio::streambuf boost_response;
  boost::system::error_code error;

  // Read until EOF.
  std::ostringstream response_stream;
  while (boost::asio::read(*boost_guts_->socket_, boost_response,
        boost::asio::transfer_at_least(1), error)) {
    // thanks to Alex Hesselgrave for finding the only way to convert a Boost
    // streambuf into a string known to man or elf.
    response_stream << &boost_response;

    if (error == boost::asio::error::eof) {
      break;
    } else if (error) {
      std::cerr << "Got error while reading: " << error.message() << std::endl;
      return false;
    }
  }

  response = response_stream.str();

  return true;
}
