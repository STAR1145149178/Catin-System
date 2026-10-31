#ifndef _SECURITY_H
#define _SECURITY_H
int security_check_write(const char *path);
int security_check_unlink(const char *path);
void load_security_policy(void);
#endif