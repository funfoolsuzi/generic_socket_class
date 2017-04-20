
#include "poll.hpp"

//Poll BASICS
Poll::Poll(int timeout):_stop(false), callback(NULL){
ptimeout = timeout;
	mthread = std::thread([&](){
        
        pmutex.lock();
        std::cout<<"Poll main thread starts"<<std::endl;
        pmutex.unlock();
		while(!_stop){
			pmutex.lock();
			int poll_size = sock.size();
			pmutex.unlock();
			pollfd poll_collect[poll_size];
			for(int idx=0;idx<poll_size;idx++){
				poll_collect[idx].fd = sock[idx].fd();
                poll_collect[idx].events = default_poll_event;
			}
			int poll_return = poll(poll_collect, poll_size, ptimeout);
			if(poll_return==-1)throw std::runtime_error(std::string("poll:")+=strerror(errno));
			if(poll_return>0){
				int jdx = 0;
				for(int idx=0;idx<poll_return;idx++){
					while(poll_collect[jdx].revents==0&&jdx<poll_size){jdx++;}
					  ///
                    std::cout<<"idx:"<<idx<<" jdx:"<<jdx<<" revents:"<<translateRevents(poll_collect[jdx].revents)<<std::endl;
					if(callback!=NULL)callback(poll_collect[idx].revents, sock[idx], this);
					  ///
					jdx++;
				}
			}
			for(int idx=0;idx<sock.size();idx++){
				if(sock[idx].life()==0){
					pmutex.lock();
					sock[idx].close();
					sock.erase(sock.begin()+idx);
					pmutex.unlock();
					idx--;
				}
			}
		} //end of while loop
	}); //end of lambda for mthread
	timer_thread = std::thread([&](){
		while(!_stop){
			std::this_thread::sleep_for(std::chrono::seconds(1));
			for(int idx=0;idx<sock.size();idx++){
				if(sock[idx].life()>0){
					pmutex.lock();
					sock[idx].live();
					pmutex.unlock();
				}
			}
		}
	});  //end of timer_thread
	mthread.detach();
	timer_thread.detach();
}  //end of Poll constructor
Poll::~Poll(void){
	std::cout<<"Poll exiting"<<std::endl;
	_stop=true;
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
void Poll::add(Socket&& s){
	pmutex.lock();
	sock.push_back(std::move(s));
	pmutex.unlock();
}
void Poll::add(const Socket& s){
	add(std::move(const_cast<Socket&>(s)));
}
void Poll::listcurrent(void){
	std::cout<<"sock size:"<<sock.size()<<std::endl;
	for(int idx=0;idx<sock.size();idx++){
		std::cout<<sock[idx].fd()<<" life:"<<sock[idx].life()<<std::endl;
	}
}
void Poll::setfunc(void (*cb)(int, Socket&, Poll *)){
	callback = cb;
}
void Poll::stop(void){
	_stop = true;
}

//Listner BASICS
Listener::Listener(const std::string addr, unsigned short port, int backlog, int timeout):Poll(timeout), mainsock(AF_INET, SOCK_STREAM, IPPROTO_TCP){
    mainsock.setreuseaddr(true);
    if(!mainsock.bind(addr, port))throw std::runtime_error("Listener():failed to bind");
    pmutex.lock();
    std::cout<<"succeeded binding on "<<addr<<':'<<port<<std::endl;
    if(!mainsock.listen(backlog))throw std::runtime_error("Listener():failed to start listen");
    std::cout<<"started listening"<<std::endl;
    pmutex.unlock();
    lthread = std::thread([&](){
        while(!_stop){
            int nfd = mainsock.accept();
            if(nfd>0)add(Socket(nfd));
            std::cout<<"accepting "<<nfd<<std::endl;
            sock[sock.size()-1].setnonblock(true);
        }
    });//end of thread
    lthread.detach();
}//end of constructor


//individual functions:
std::string translateRevents(short re){
    std::string rv;
    if(re&POLLIN) rv+="POLLIN ";
    if(re&POLLRDNORM) rv+="POLLRDNORM ";
    if(re&POLLRDBAND) rv+="POLLRDBAND ";
    if(re&POLLPRI) rv+="POLLPRI ";
    if(re&POLLOUT) rv+="POLLOUT ";
    if(re&POLLWRNORM) rv+="POLLWRNORM ";
    if(re&POLLWRBAND) rv+="POLLWRBAND ";
    if(re&POLLERR) rv+="POLLERR ";
    if(re&POLLHUP) rv+="POLLHUP ";
    if(re&POLLNVAL) rv+="POLLNVAL ";
    return rv;
}
