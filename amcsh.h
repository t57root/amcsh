#ifndef _AMCSH_H
#define _AMCSH_H

char magickey[2] = { 0377, 0377 };

typedef struct
{       
    char flag[4];
    int32_t ws_row;
    int32_t ws_col;
}WINCH,*pWINCH;
 
typedef struct           
{
    char term[255];   
    int32_t ws_row;
    int32_t ws_col;
    char pwd[20];
}MSG,*pMSG; 

#endif
