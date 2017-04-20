//
//  ip.hpp
//  Generic Server Class
//
//  Created by Xiwen Li on 2/1/17.
//  Copyright © 2017 Xiwen Li. All rights reserved.
//

#ifndef ip_hpp
#define ip_hpp

#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <stdexcept>
#include <cstring>

struct IpOpt{
    unsigned int o_copied:1;
    unsigned int o_class:2;
    unsigned int o_num:3;
    u_int8_t o_len;
    u_int8_t o_data;
};

class IpHeader{
    unsigned int _ihl:4;
    unsigned int _ver:4;
    u_int8_t _tos;
    u_int16_t _len;
    u_int16_t _id;
    unsigned int _flags:3;
    unsigned int _off:13;
    u_int8_t _ttl;
    u_int8_t _protocol;
    u_int16_t _checksum;
    u_int32_t _src;
    u_int32_t _des;
    IpOpt *option_ptr;
    char *_con_ptr;
    
    //private function
    bool checksum(const char *ptr){
        u_int32_t sum = 0;
        u_int16_t sum2;
        int idx;
        for(idx = 0; idx<_ihl*2; idx++){
            sum+=*(u_int16_t *)(ptr+(idx*2));
        }
        sum2 = sum%0x10000+(sum-sum%0x10000)/0x10000;
        return ~sum2==0;
    }
public:
    IpHeader(const char *ptr, bool ifchecksum = false){
        memcpy(this, ptr, 20);
        if(ifchecksum){
            if(!checksum(ptr))throw std::runtime_error("IpHeader constructor checksum test failed");
        }
        if(checksum(ptr))
        if(_ihl>5){
            if(_ihl>15)throw std::runtime_error("IpHeader constructor can't accept IP header length more than 60bytes/15 32-bit-blocks");
            option_ptr = new IpOpt[_ihl-5];
            memcpy(option_ptr, ptr+20, (_ihl-5)*4);
        } else {option_ptr = NULL;}
        _con_ptr = (char *)ptr+(_ihl*4);
    }
    IpHeader(const std::string& srcstr, bool ifchecksum = false){
        memcpy(this, srcstr.data(), 20);
        if(ifchecksum){
            if(!checksum(srcstr.data()))throw std::runtime_error("IpHeader constructor checksum test failed");
        }
        if(_ihl>5){
            if(_ihl>15)throw std::runtime_error("IpHeader constructor can't accept IP header length more than 60bytes/15 32-bit-blocks");
            option_ptr = new IpOpt[_ihl-5];
            memcpy(option_ptr, srcstr.data()+20, (_ihl-5)*4);
        } else {option_ptr = NULL;}
        _con_ptr = (char *)srcstr.data()+(_ihl*4);
    }
    IpHeader(const IpHeader& cpysrc){
        if(cpysrc.option_ptr!=NULL){
            option_ptr = new IpOpt[cpysrc._ihl-5];
            memcpy(option_ptr, cpysrc.option_ptr, (cpysrc._ihl-5)*4);
        } else {option_ptr = NULL;}
    }
    IpHeader& operator = (const IpHeader& cpysrc){
        if(this != &cpysrc){
            if(cpysrc.option_ptr!=NULL){
                IpOpt *_option_ptr = new IpOpt[cpysrc._ihl-5];
                delete[] option_ptr;
                memcpy(option_ptr, cpysrc.option_ptr, (cpysrc._ihl-5)*4);
            } else { option_ptr = NULL;}
        }
        return *this;
    }
    ~IpHeader(void){
        delete[] option_ptr;
    }
    
    char *conptr(void){return _con_ptr;}
    std::string desipstr(void){
        std::string rv(::inet_ntoa(*((in_addr *)&_des)));
        return rv;
    }
    bool flagdf(void){return ((_flags&2)==2);}
    bool flagmf(void){return ((_off&1)==1);}
    u_int16_t hdl(void){return _ihl*4;}
    u_int16_t id(void){return ::ntohs(_id);}
    u_int16_t len(void){return ::ntohs(_len);}
    u_int16_t off(void){return _off*8;}
    u_int16_t opcount(void){return _ihl-5>0?(_ihl-5):0;}
    u_int16_t protocol(void){return _protocol;}
    std::string srcipstr(void){
        std::string rv(::inet_ntoa(*((in_addr *)&_src)));
        return rv;
    }
    u_int8_t ttl(void){return _ttl;}
};

class UdpHeader{
    u_int16_t _srcport;
    u_int16_t _desport;
    u_int16_t _len;
    u_int16_t _checksum;
    char *_con_ptr;
    
public:
    UdpHeader(const char *ptr){
        memcpy(this, ptr, 8);
        _con_ptr = (char *)ptr+8;
    }
    char *conptr(void){ return _con_ptr;}
    u_int16_t desport(void){ return ::ntohs(_desport);}
    u_int16_t len(void){ return ::ntohs(_len);}
    std::string content(void){
        std::string rv(_con_ptr, _len-8);
        return rv;
    }
    u_int16_t srcport(void){ return ::ntohs(_srcport);}
    
};

#endif /* ip_hpp */
