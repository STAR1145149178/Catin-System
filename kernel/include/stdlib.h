#ifndef _FAKE_STDLIB_H
#define _FAKE_STDLIB_H

#include "types.h"
int katoi(const char *s);
#define atoi katoi

#endif