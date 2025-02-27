#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <iostream>
#include <thread>

int main(){
    boost::asio::io_context context;
    boost::system::error_code ec;
    boost::asio::ip::tcp::acceptor acceptor_(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),8088));

    boost::asio::io_context::work fakeWork(context);

    std::thread thrContext = std::thread( [&]() { 
      context.run();
    });

    boost::asio::ip::tcp::socket socket(context);
    acceptor_.async_accept(socket,
      [&](std::error_code ec, boost::asio::ip::tcp::socket sock) {
        if(!ec) {
          std::cout << "Are we here yet?" << std::endl;
        }
      }
    );

    if(socket.is_open()){
        socket.wait(socket.wait_read);

        size_t bytes = socket.available();

        if(bytes > 0) {
            std::cout << "Bytes received: " << bytes << std::endl;
            std::vector<char> vBuffer(bytes);
            socket.read_some(boost::asio::buffer(vBuffer.data(),bytes),ec);

            for(auto data:vBuffer){
                std::cout << data;
            }
        }
        else {
            std::cout << "No bytes received" << std::endl;
        }
        // context.stop();
        if (thrContext.joinable()) thrContext.join();
    }
    socket.close();
}