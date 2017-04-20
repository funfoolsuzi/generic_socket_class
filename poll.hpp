
#ifndef _POLL_HPP

#define _POLL_HPP

#include "gsc.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <poll.h>

class Poll{
protected:
    std::vector<Socket> sock;
    std::thread mthread;
    std::thread timer_thread;
    std::mutex pmutex;
    bool _stop;
    int ptimeout;
    short default_poll_event = POLLIN;
    void (*callback)(int, Socket&, Poll *);
public:
    Poll(int timeout = 1);
    ~Poll(void);
    void add(Socket&& s);
    void add(const Socket& s);
    void listcurrent(void);
    void setfunc(void (*cb)(int, Socket&, Poll *));
    void stop(void);
};

class Listener:public Poll{
    Socket mainsock;
    std::thread lthread;
public:
    Listener(const std::string addr, unsigned short port, int backlog = 10, int timeout = 1);
};

//individual functions:
std::string translateRevents(short re);

#endif
