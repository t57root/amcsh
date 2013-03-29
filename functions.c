#include <sys/socket.h>    
#include <netinet/in.h>

int wsend(int fd,void *buf,int size){
    int ret;
    while(size){
        ret=send(fd, buf, size,0);
        if(ret<0) return -1;
        size=size-ret;
        buf+=ret;
    } 
    return ret;
}     

int wrecv(int fd,void *buf,int size){
    int ret;
    while(size){
        ret=recv(fd, buf, size,0);
        if(ret<0) return -1;
        size=size-ret;
        buf+=ret;
    }
    return ret;
}                                            

