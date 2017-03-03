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
  tcp::resolver::query query(server, port);
  tcp::resolver::iterator endpoint_iterator =
    boost_guts_->resolver_->resolve(query);
  tcp::resolver::iterator end;

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
  // Read until EOF.
  // TODO: this probably won't work if the remote host doesn't close the socket
  // (e.g. with 'Connection: keep-alive' set in HTTP headers). Should look for
  // content-length and behave appropriately.
  boost::asio::read(*boost_guts_->socket_, boost_response);

  // thanks to Alex Hesselgrave for solving the mystery of turning a boost
  // streambuf into a string
  std::istream is(&boost_response);
  is >> response;

  return true;
}
