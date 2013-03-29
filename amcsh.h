#ifndef _AMCSH_H
#define _AMCSH_H

char magickey[2] = { 0377, 0377 };

typedef struct
{       
    char flag[4];
    int ws_row;
    int ws_col;
}WINCH,*pWINCH;
 
typedef struct           
{
    char term[255];   
    int ws_row;
    int ws_col;
    char pwd[20];
}MSG,*pMSG; 

#endif
