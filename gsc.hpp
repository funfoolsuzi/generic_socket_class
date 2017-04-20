//
//  gsc.hpp
//  Generic Server Class
//
//  Created by Xiwen Li on 1/15/17.
//  Copyright © 2017 Xiwen Li. All rights reserved.
//

#ifndef gsc_hpp
#define gsc_hpp

#include <iostream>
#include <string>
#include <exception>

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

class SocketError: std::exception{
	private:
	int _type;
	int _errn;
	public:
	SocketError(int error_index, int errnum = 0);
	const char* what() const noexcept;
	int type(void);
	int errn(void);
};

class Socket{
    int _descriptor;
    unsigned int _life;
    Socket(const Socket&);
    Socket& operator=(const Socket&);
public:
    Socket(int address_family, int socket_type, int protocol, unsigned int life = 5);
    Socket(int fd, unsigned int life = 5);
    Socket(Socket&& other);
    Socket& operator=(Socket &&);
    ~Socket(void);
    
    //setting
    bool bind(const std::string& addr, unsigned short port);
    int fd(void);
    bool setnonblock(bool value);
    bool setreuseaddr(bool value);
    
    //connection:
    int accept(sockaddr_storage *addr=NULL);
    void close(void);
    void connect(const sockaddr_storage& addr);
    void connect(const std::string& addrstr, unsigned short port);
    bool listen(int backlog = 50);
    ssize_t recv(void *buffer, size_t length, int flags);
    std::string recv(size_t length);
    ssize_t send(const void *buffer, size_t length, int flags);
    void send(const std::string& content);
	
	//packet:
    ssize_t recvfrom(void *buffer, size_t length, int flags, sockaddr *addr);
    std::string recvfrom(size_t length, sockaddr *addr = NULL);
    ssize_t sendto(const std::string& msg, const std::string& address, unsigned short port, int flags = 0);
    //life:
    void addlife(unsigned int sec){_life+=sec;}
    void live(void){_life--;}
    unsigned int life(void){return _life;}
    void setlife(unsigned int sec){_life=sec;}
};

sockaddr_storage strtoip(const std::string& address, unsigned short port);




#endif /* gsc_hpp */
