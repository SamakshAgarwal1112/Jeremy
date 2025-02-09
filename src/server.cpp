#include <bits/stdc++.h>
#include <boost/asio.hpp>
using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cin;
using std::cout;
using std::endl;
#include "con_handler.cpp"

class Server 
{
private:
   tcp::acceptor acceptor_;
   void start_accept()
   {
    // socket
     con_handler::pointer connection = con_handler::create(acceptor_.get_io_service());

    // asynchronous accept operation and wait for a new connection.
     acceptor_.async_accept(connection->socket(),
        boost::bind(&Server::handle_accept, this, connection,
        boost::asio::placeholders::error));
  }
public:
//constructor for accepting connection from client
  Server(boost::asio::io_service& io_service): acceptor_(io_service, tcp::endpoint(tcp::v4(), 8088))
  {
     start_accept();
  }
  void handle_accept(con_handler::pointer connection, const boost::system::error_code& err)
  {
    if (!err) {
      connection->start();
    }
    start_accept();
  }
};

string read_(tcp::socket & socket) {
       boost::asio::streambuf buf;
       boost::asio::read_until( socket, buf, "\n" );
       string data = boost::asio::buffer_cast<const char*>(buf.data());
       return data;
}
void send_(tcp::socket & socket, const string& message) {
       const string msg = message + "\n";
       boost::asio::write( socket, boost::asio::buffer(message) );
}

int main(){
//     boost::asio::io_service io_service;
//     tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 8088 ));
//     tcp::socket socket_(io_service);
//     acceptor_.accept(socket_);
//     string message = read_(socket_);
//     cout << message << endl;
//     send_(socket_, "Hello From Server!");
//     cout << "Server sent message successfully to Client!" << endl;
//     return 0;
  try
    {
    boost::asio::io_service io_service;  
    Server server(io_service);
    io_service.run();
    }
  catch(std::exception& e)
    {
    std::cerr << e.what() << endl;
    }
  return 0;   
}