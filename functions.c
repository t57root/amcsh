#include <sys/socket.h>    
#include <netinet/in.h>
#include <stdint.h>

int full_send(int fd,void *buf,int size)
{
    int ret,total=0;
    while(size){
        ret=send(fd, buf, size,0);
        total+=ret;
        if(ret<0) return ret;
        size=size-ret;
        buf+=ret;
    } 
    return total;
}     

int full_recv(int fd,void *buf,int size)
{
    int ret,total=0;
    while(size){
        ret=recv(fd, buf, size,0);
        total+=ret;
        if(ret<=0) return ret;
        size=size-ret;
        buf+=ret;
    }
    return total;
}                                            

int wsend(int fd,void *buf,int size)
{
    int ret;
    ret = full_send(fd, &size, sizeof(int32_t));
    if(ret<0) return ret;
    ret = full_send(fd,buf,size);
    return ret;
}     

int wrecv(int fd,void *buf)
{
    int ret,size;
    ret=full_recv(fd, &size, sizeof(int32_t));
    if(ret<=0) return ret;
    ret = full_recv(fd,buf,size);
    return ret;
}
