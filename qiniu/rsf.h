//
// Created by jemy on 07/08/2017.
//

#ifndef PROJECT_RSF_H
#define PROJECT_RSF_H

#include "http.h"

typedef struct __Qiniu_RSF_CommonPrefix {
    const char *commonPrefix;
    struct Qiniu_RSF_CommonPrefix *next;
} Qiniu_RSF_CommonPrefix;

typedef struct __Qiniu_RSF_ListItem {
    const char *key;
    const char *hash;
    const char *mimeType;
    const char *endUser;
    Qiniu_Int64 fsize;
    Qiniu_Int64 putTime;
    Qiniu_Int64 type;
    struct Qiniu_RSF_ListItem *next;
} Qiniu_RSF_ListItem;

typedef struct __Qiniu_RSF_ListRet {
    const char *marker;
    Qiniu_RSF_CommonPrefix *commonPrefix;
    Qiniu_RSF_ListItem *item;
} Qiniu_RSF_ListRet;

QINIU_DLLAPI extern Qiniu_Error Qiniu_RSF_ListFiles(Qiniu_Client *self, Qiniu_RSF_ListRet *ret, const char *bucket,
                                                    const char *prefix, const char *delimiter, const char *marker,
                                                    int limit);

#endif //PROJECT_RSF_H

