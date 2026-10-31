#ifndef _CATIN_ZIP_H
#define _CATIN_ZIP_H
#include "catin_types.h"
int catin_zip_extract(const char *zip_path, const char *dir);
int catin_zip_create(const char *zip_path, const char *const *files, int n);
#endif
