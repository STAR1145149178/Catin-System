#ifndef _FAKE_STRING_H
#define _FAKE_STRING_H

#include "types.h"

// 强行声明你自己的底层字符串函数
usize kstrlen(const char *s);
void kstrncpy(char *d, const char *s, usize n);
int kstrcmp(const char *a, const char *b);
int kstrncmp(const char *a, const char *b, usize n);
void *kmemcpy(void *d, const void *s, usize n);
void *kmemset(void *d, int c, usize n);
int kmemcmp(const void *a, const void *b, usize n);

// 核心魔法：把 lwIP 里的标准库函数名，强行替换成你的 k 系列
#define strlen kstrlen
#define strncpy kstrncpy
#define strcmp kstrcmp
#define strncmp kstrncmp
#define memcpy kmemcpy
#define memset kmemset
#define memcmp kmemcmp

#endif