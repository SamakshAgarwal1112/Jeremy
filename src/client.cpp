#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <iostream>
#include <thread>

std::vector<char> vBuffer(1*1024);

void AsyncGrabData(boost::asio::ip::tcp::socket &socket){
    socket.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()),
        [&](std::error_code ec, std::size_t bytes){
            if(!ec) {
                std::cout<< "Bytes received: " << bytes << std::endl;
                for(int i=0; i<bytes; i++){
                    std::cout << vBuffer[i];
                }
                AsyncGrabData(socket);
            }
            else {
                socket.close();
            }
        }
    );
}

int main(){
    boost::asio::io_context context;
    boost::system::error_code ec;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1",ec),8088);

    boost::asio::io_context::work fakeWork(context);

    std::thread thrContext = std::thread( [&]() { 
        context.run();
    });

    boost::asio::ip::tcp::socket socket(context);
    socket.connect(endpoint,ec);
    if(!ec) {
        std::cout<<"Yayyy! Connected!!"<<std::endl;
    }
    else {
        std::cout<<"Good lord!!\n"<<ec.message()<<std::endl;
    }

    if(socket.is_open()){

        AsyncGrabData(socket);

        std::string sRequest = "Hello";
            // "GET /index.html HTTP/1.1\r\n"
            // "Host: google.com\r\n"
            // "Connection: close\r\n\r\n";
        
        socket.write_some(boost::asio::buffer(sRequest.data(),sRequest.size()),ec);

        context.stop();
        if (thrContext.joinable()) thrContext.join();
    }
    socket.close();
}