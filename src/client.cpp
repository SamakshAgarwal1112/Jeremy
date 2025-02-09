#include <bits/stdc++.h>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cin;
using std::cout;
using std::endl;

int main() {
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);
    socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 8088 ));
    string msg = "";
    cout << "Enter your message: ";
    getline(cin, msg);
    msg += "\n";

    boost::system::error_code error;
    boost::asio::write( socket, boost::asio::buffer(msg), error );
    if( !error ) {
        cout << "Client sent message successfully to server!" << endl;
    }
    else {
        cout << "Send failed: " << error.message() << endl;
    }
    boost::asio::streambuf receive_buffer;
    boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
    if( error && error != boost::asio::error::eof ) {
        cout << "Receive failed: " << error.message() << endl;
    }
    else {
        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        cout << data << endl;
    }
    return 0;
}
