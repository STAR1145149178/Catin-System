#include "catin.h"
int catin_net_socket(int t){ return catin_socket(t); }
int catin_net_connect(int s,u32 ip,u16 p){ return catin_connect(s,ip,p); }
int catin_net_send(int s,const void*b,int n){ return catin_send(s,b,n); }
int catin_net_recv(int s,void*b,int n){ return catin_recv(s,b,n); }
int catin_net_close(int s){ return catin_close_sock(s); }
u32 catin_net_dns(const char*n){ return catin_dns_lookup(n); }
u32 catin_net_parse_ip(const char*str){
    u32 a=0,b=0,c=0,d=0;
    const char*p=str;
    while(*p>='0'&&*p<='9') a=a*10+(*p++-'0'); if(*p=='.')p++;
    while(*p>='0'&&*p<='9') b=b*10+(*p++-'0'); if(*p=='.')p++;
    while(*p>='0'&&*p<='9') c=c*10+(*p++-'0'); if(*p=='.')p++;
    while(*p>='0'&&*p<='9') d=d*10+(*p++-'0');
    return (a<<24)|(b<<16)|(c<<8)|d;
}
