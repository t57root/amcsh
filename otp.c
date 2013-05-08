/*                                 
 *  otp.c - one-time password generator
 *  t57root@gmail.com              
 *  openwill.me / www.hackshell.net
 */                                
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "hmac.h"
#include "sha1.h"
#include "config.h"

int otpLength = 6;
char *lut = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

int strpos(char *str,char find)
{
    int i=0;
    while(str[i]){
        if(str[i]==find)
            break;
        i++;
    }
    i=(i==strlen(str))?-1:i;
    return i;
}

void base32_decode(char *str,char *binary)
{
    int l = strlen(str);
    int n=0,i,j=0;
    int c = 0;
    char tmp;
    for(i=0;i<l;i++){
        n = n << 5;
        n = n + strpos(lut,str[i]);
        j = j +5;
        if(j >= 8){
            j = j-8;
            tmp = (n & (0xFF << j)) >>j;
            memcpy(binary+c,&tmp,1);
            c++;
        }
    }
}

void getotp(char *key,int keylen, int timestamp,char *pwd)
{
    char *t = malloc(2*sizeof(int));
    memset(t,0,2*sizeof(int));
    char *bp = (char *)&timestamp;
    int i = 0;            
    for(i=0;i<4;i++){     
        memcpy(t+2*sizeof(int)-1-i,bp+i,1);
    }                     
    uint8_t hash[SHA1_DIGEST_LENGTH];
    hmac_sha1(key, keylen, t, 8, hash, SHA1_DIGEST_LENGTH);
    char offset = hash[19] & 0xf;

    int ret = (
        ((hash[offset+0] & 0x7f) << 24) | ((hash[offset+1] & 0xff) << 16 ) |((hash[offset+2] & 0xff) << 8) | (( hash[offset+3]) & 0xff)
    ) % (int)pow(10, otpLength);
    free(t);
    sprintf(pwd,"%d",ret);
}

void get_otp(char *pwd)
{
    time_t now;
    time(&now);
    int timestamp = (int)now/30;
    char *initKey = _OneTimePass;
    int keylen = strlen(initKey);
    char *key = malloc(keylen);
    memset(key,0,keylen);
    base32_decode(initKey,key);
    getotp(key, keylen, timestamp,pwd);
    free(key);
    int padding,i;
    if((padding = otpLength - strlen(pwd))>0){
        char *otp = malloc(7);
        memset(otp,0,7);
        for(i=0;i<padding;i++){
            otp[i]='0';
        }
        strcat(otp,pwd);
        memcpy(pwd,otp,7);
        free(otp);
    }
}

