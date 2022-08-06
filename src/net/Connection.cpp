#include "Connection.hpp"
#include <iostream>



/* Connection::Connection(std::shared_ptr<EventLoop> loop)//:
   // loop_(loop),
   // local_sock_(kInvalid_) 
   {
    } */

int Connection::Identifier() const {
        return local_sock_;
}
bool Connection::HandleReadEvent() {
    std::cout << "HandleReadEvent" << std::endl;
    return true;
    
}

bool Connection::HandleWriteEvent() {
    std::cout << "HandleWriteEvent" << std::endl;
    return true;
}

void Connection::HandleErrorEvent() {
    std::cout << "HandleErrorEvent" << std::endl;
}