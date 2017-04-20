//
//  gsc.cpp
//  Generic Server Class
//
//  Created by Xiwen Li on 1/15/17.
//  Copyright © 2017 Xiwen Li. All rights reserved.
//

#include "gsc.hpp"

//SocketError BASICS:
SocketError::SocketError(int error_index, int errnum):_type(error_index), _errn(errnum){
	std::string msg = "SocketError:";
	switch(_type){
		case 1:msg += "Invalid Socket construction.";break;
		case 2:msg += "Invalid Socket destruction/closure.";break;
		case 3:msg += "Socket failed to bind.";break;
		case 4:msg += "Socket failed to accept.";break;
		case 5:msg += "Socket failed to connect.";break;
		case 6:msg += "Socket failed to start to listen.";break;
		case 7:msg += "Socket failed to receive.";break;
		case 8:msg += "Socket failed to send.";break;
		case 9:msg += "Socket failed to recvfrom.";break;
	}
	if(_errn!=0){
		msg+="(";
		msg+=strerror(_errn);
		msg+=")";
	}
    std::cout<<msg<<std::endl;
}
const char *SocketError::what() const noexcept{
	std::string msg = "SocketError:";
	switch(_type){
		case 1:msg += "Invalid Socket construction.";break;
		case 2:msg += "Invalid Socket destruction/closure.";break;
		case 3:msg += "Socket failed to bind.";break;
		case 4:msg += "Socket failed to accept.";break;
		case 5:msg += "Socket failed to connect.";break;
		case 6:msg += "Socket failed to start to listen.";break;
		case 7:msg += "Socket failed to receive.";break;
		case 8:msg += "Socket failed to send.";break;
		case 9:msg += "Socket failed to recvfrom.";break;
	}
	if(_errn!=0){
		msg+="(";
		msg+=strerror(_errn);
		msg+=")";
	}
	return msg.c_str();
}
int SocketError::type(void){
	return _type;
}
int SocketError::errn(void){
	return _errn;
}

//Socket BASICS:
Socket::Socket(int address_family, int socket_type, int protocol, unsigned int life){
    _life = life;
    _descriptor = socket(address_family, socket_type, protocol);
    if(_descriptor==-1)throw SocketError(1);
}
Socket::Socket(int fd, unsigned int life):_descriptor(fd), _life(life){
    if(fcntl(fd, F_GETFL)==0) throw SocketError(1);
}
Socket::Socket(Socket&& other){
    int medium = _descriptor;
    _descriptor = other._descriptor;
    _life = other._life;
    other._descriptor = -1;
    other._life = 0;
}
Socket& Socket::operator=(Socket&& other){
    if(this!=&other){
        _descriptor = other._descriptor;
        _life = other._life;
        other._descriptor = -1;
        other._life = 0;
    }
    return *this;
}
Socket::~Socket(void){
    if(_descriptor<3)return;
    if(::close(_descriptor)==-1){
        throw SocketError(2, errno);
    }
}
//setting:
bool Socket::bind(const std::string& address, unsigned short port){
	sockaddr_storage addr;
    addr = strtoip(address, port);
    socklen_t size = sizeof(sockaddr_in);
    if(addr.ss_family==AF_INET6)size = sizeof(sockaddr_in6);
    if(::bind(_descriptor, (sockaddr *)&addr, sizeof(sockaddr))==0)return true;
    throw SocketError(3, errno);
}
int Socket::fd(void){
	return _descriptor;
}
bool Socket::setnonblock(bool value){
    int current_flags = fcntl(_descriptor, F_GETFL);
    if((current_flags&O_NONBLOCK)==(value?O_NONBLOCK:0)) return true;
    if(fcntl(_descriptor, F_SETFL, value?(current_flags|O_NONBLOCK):(current_flags^O_NONBLOCK))==-1)return false;
    return true;
}
bool Socket::setreuseaddr(bool value){
    int setting;
    socklen_t setting_size = sizeof setting;
    if(getsockopt(_descriptor, SOL_SOCKET, SO_REUSEADDR, &setting, &setting_size)==-1) return false;
    if(setting==(value?1:0)) return true;
    setting = value?1:0;
    if(setsockopt(_descriptor, SOL_SOCKET, SO_REUSEADDR, &setting, setting_size)==-1) return false;
    return true;
}
//connection:
int Socket::accept(sockaddr_storage *addr){
	socklen_t s = sizeof(sockaddr_storage);
	int sfd=::accept(_descriptor, (sockaddr *)addr, &s);
	if(sfd==-1) SocketError(4, errno);
	return sfd;
}
void Socket::close(void){
	if(_descriptor<3)return;
	if(::close(_descriptor)==-1){
		throw SocketError(2, errno);
	}
	_descriptor = -1;
}
void Socket::connect(const sockaddr_storage& addr){
	if(::connect(_descriptor, (sockaddr *)&addr, sizeof(addr))==-1)throw SocketError(5, errno);
}
void Socket::connect(const std::string& addrstr, unsigned short port){
	sockaddr_storage addr = strtoip(addrstr, port);
	connect(addr);
}
bool Socket::listen(int backlog){
	if(::listen(_descriptor, backlog)==0)return true;
	throw SocketError(6, errno);
}
/* *
 * About recv()
 * 
 * */
ssize_t Socket::recv(void *buffer, size_t length, int flags){
	return ::recv(_descriptor, buffer, length, flags);
}
std::string Socket::recv(size_t length){
	char buf[length];
	std::cout<<"rrrr"<<_descriptor<<std::endl;
	if(::recv(_descriptor, buf, length, 0)==-1){
		throw SocketError(7, errno);
	}
	return std::string(buf);
}
ssize_t Socket::send(const void *buffer, size_t length, int flags){
	return ::send(_descriptor, buffer, length, flags);
}
void Socket::send(const std::string& content){
	if(send(content.c_str(), content.size(), 0)==-1)throw SocketError(8, errno);
}

//packet:
ssize_t Socket::recvfrom(void *buffer, size_t length, int flags, sockaddr *addr){
	socklen_t addr_size = sizeof(sockaddr);
	return ::recvfrom(_descriptor, buffer, length, flags, addr, &addr_size);
}
std::string Socket::recvfrom(size_t length, sockaddr *addr){
	char buf[length];
    memset(buf, 0, length);
    ssize_t recvbytes;
	try{recvbytes = recvfrom(buf, length, 0, addr);}
	catch(const std::runtime_error& err){
		throw SocketError(9, errno);
	}
	return std::string(buf, recvbytes);
}
ssize_t Socket::sendto(const std::string& msg, const std::string& address, unsigned short port, int flags){
    sockaddr_storage addr = strtoip(address, port);
    return ::sendto(_descriptor, msg.c_str(), msg.size(), flags, (sockaddr *)&addr, (addr.ss_family!=AF_INET)?sizeof(sockaddr_in6):sizeof(sockaddr_in));
}

//INDIVIDUAL FUNCTIONS
sockaddr_storage strtoip(const std::string& address, unsigned short port){
    sockaddr_storage addr;
    void *addr_value_ptr;
    if(address.find(':')!=address.npos){
	    addr.ss_family = AF_INET6;
	    addr_value_ptr = &(((sockaddr_in6 *)&addr)->sin6_addr);
	    ((sockaddr_in6 *)&addr)->sin6_port=htons(port);
	} else if(address.find('.')!=address.npos){
	    addr.ss_family = AF_INET;
	    addr_value_ptr = &(((sockaddr_in *)&addr)->sin_addr);
	    ((sockaddr_in *)&addr)->sin_port=htons(port);
	} else throw std::runtime_error("strtoip():invalid address string.");
    if((addr.ss_family==AF_INET6&&address.size()>39)||(addr.ss_family==AF_INET&&address.size()>15))throw std::runtime_error("strtoip():invalid address, address string too long.");
    if(!inet_pton(addr.ss_family, address.c_str(), addr_value_ptr))throw std::runtime_error("strtoip():bad ip address");
    //std::cout<<(((sockaddr_in *)&addr)->sin_addr).s_addr<<" "<<((sockaddr_in *)&addr)->sin_port<<std::endl;
    return addr;
}
