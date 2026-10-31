#ifndef _CATIN_HTTP_H
#define _CATIN_HTTP_H
#include "catin_types.h"
int catin_http_get(const char *url, char *out, u32 cap, u32 *out_len);
int catin_http_download(const char *url, const char *savepath);
#endif
