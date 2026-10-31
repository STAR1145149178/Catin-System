#include "catin.h"
static int parse_url(const char *url, char *host, int hcap, u16 *port, char *path, int pcap){
    const char *p = url;
    if(!catin_strncmp(p,"http://",7)) p+=7;
    else if(!catin_strncmp(p,"https://",8)) p+=8;
    int i=0;
    while(*p && *p!=':' && *p!='/' && i<hcap-1) host[i++]=*p++;
    host[i]=0;
    *port = 80;
    if(*p==':'){ p++; u16 pp=0; while(*p>='0'&&*p<='9') pp=pp*10+(*p++-'0'); *port=pp; }
    i=0;
    if(*p==0) path[i++]='/';
    while(*p && i<pcap-1) path[i++]=*p++;
    path[i]=0;
    return 0;
}
int catin_http_get(const char *url, char *out, u32 cap, u32 *out_len){
    char host[128], path[512]; u16 port;
    parse_url(url, host, sizeof(host), &port, path, sizeof(path));
    u32 ip = catin_net_dns(host);
    if(!ip) return -1;
    int s = catin_net_socket(CATIN_SOCK_TCP);
    if(s < 0) return -1;
    if(catin_net_connect(s, ip, port) != 0){ catin_net_close(s); return -1; }
    char req[1024];
    int rl = catin_snprintf(req, sizeof(req),
        "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Catin/2.2\r\nConnection: close\r\n\r\n",
        path, host);
    catin_net_send(s, req, rl);
    u32 total = 0;
    for(;;){
        if(total >= cap-1) break;
        int r = catin_net_recv(s, out+total, cap-total-1);
        if(r <= 0) break;
        total += r;
    }
    out[total] = 0;
    catin_net_close(s);
    char *body = catin_strstr(out, "\r\n\r\n");
    if(!body) body = catin_strstr(out, "\n\n");
    if(body){
        body += (body[0]=='\r') ? 4 : 2;
        u32 blen = catin_strlen(body);
        catin_memcpy(out, body, blen+1);
        total = blen;
    }
    if(out_len) *out_len = total;
    return 0;
}
int catin_http_download(const char *url, const char *savepath){
    u32 cap = 4*1024*1024;
    char *buf = catin_malloc(cap);
    if(!buf) return -1;
    u32 len = 0;
    int rc = catin_http_get(url, buf, cap, &len);
    if(rc != 0){ catin_free(buf); return -1; }
    int w = catin_fs_writefile(savepath, buf, len);
    catin_free(buf);
    return w;
}
