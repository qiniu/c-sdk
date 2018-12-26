/*
 ============================================================================
 Name        : resumable_io.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "resumable_io.h"
#include <curl/curl.h>
#include <sys/stat.h>

#define    blockBits            22
#define blockMask            ((1 << blockBits) - 1)

#define defaultTryTimes        3
#define defaultWorkers        4
#define defaultChunkSize    (4 * 1024 * 1024) // 4MB

/*============================================================================*/
/* type Qiniu_Rio_ST - SingleThread */

#if defined(_WIN32)

#include <windows.h>

typedef struct _Qiniu_Rio_MTWG_Data
{
    Qiniu_Count addedCount;
    Qiniu_Count doneCount;
    HANDLE event;
} Qiniu_Rio_MTWG_Data;

static void Qiniu_Rio_MTWG_Add(void* self, int n)
{
    Qiniu_Count_Inc(&((Qiniu_Rio_MTWG_Data*)self)->addedCount);
} // Qiniu_Rio_MTWG_Add

static void Qiniu_Rio_MTWG_Done(void* self)
{
    Qiniu_Count_Inc(&((Qiniu_Rio_MTWG_Data*)self)->doneCount);
    SetEvent(((Qiniu_Rio_MTWG_Data*)self)->event);
} // Qiniu_Rio_MTWG_Done

static void Qiniu_Rio_MTWG_Wait(void* self)
{
    Qiniu_Rio_MTWG_Data * data = (Qiniu_Rio_MTWG_Data*)self;
    Qiniu_Count lastDoneCount = data->doneCount;
    DWORD ret = 0;

    while (lastDoneCount != data->addedCount) {
        ret = WaitForSingleObject(((Qiniu_Rio_MTWG_Data*)self)->event, INFINITE);
        if (ret == WAIT_OBJECT_0) {
            lastDoneCount = data->doneCount;
        }
    } // while
} // Qiniu_Rio_MTWG_Wait

static void Qiniu_Rio_MTWG_Release(void* self)
{
    CloseHandle(((Qiniu_Rio_MTWG_Data*)self)->event);
    free(self);
} // Qiniu_Rio_MTWG_Release

static Qiniu_Rio_WaitGroup_Itbl Qiniu_Rio_MTWG_Itbl = {
    &Qiniu_Rio_MTWG_Add,
    &Qiniu_Rio_MTWG_Done,
    &Qiniu_Rio_MTWG_Wait,
    &Qiniu_Rio_MTWG_Release,
};

Qiniu_Rio_WaitGroup Qiniu_Rio_MTWG_Create(void)
{
    Qiniu_Rio_WaitGroup wg;
    Qiniu_Rio_MTWG_Data * newData = NULL;

    newData = (Qiniu_Rio_MTWG_Data*)malloc(sizeof(*newData));
    newData->addedCount = 0;
    newData->doneCount = 0;
    newData->event = CreateEvent(NULL, FALSE, FALSE, NULL);

    wg.itbl = &Qiniu_Rio_MTWG_Itbl;
    wg.self = newData;
    return wg;
} // Qiniu_Rio_MTWG_Create

#endif

static void Qiniu_Rio_STWG_Add(void *self, int n) { }

static void Qiniu_Rio_STWG_Done(void *self) { }

static void Qiniu_Rio_STWG_Wait(void *self) { }

static void Qiniu_Rio_STWG_Release(void *self) { }

static Qiniu_Rio_WaitGroup_Itbl Qiniu_Rio_STWG_Itbl = {
        Qiniu_Rio_STWG_Add,
        Qiniu_Rio_STWG_Done,
        Qiniu_Rio_STWG_Wait,
        Qiniu_Rio_STWG_Release
};

static Qiniu_Rio_WaitGroup Qiniu_Rio_STWG = {
        NULL, &Qiniu_Rio_STWG_Itbl
};

static Qiniu_Rio_WaitGroup Qiniu_Rio_ST_WaitGroup(void *self) {
    return Qiniu_Rio_STWG;
}

static Qiniu_Client *Qiniu_Rio_ST_ClientTls(void *self, Qiniu_Client *mc) {
    return mc;
}

static int Qiniu_Rio_ST_RunTask(void *self, void (*task)(void *params), void *params) {
    task(params);
    return QINIU_RIO_NOTIFY_OK;
}

static Qiniu_Rio_ThreadModel_Itbl Qiniu_Rio_ST_Itbl = {
        Qiniu_Rio_ST_WaitGroup,
        Qiniu_Rio_ST_ClientTls,
        Qiniu_Rio_ST_RunTask
};

Qiniu_Rio_ThreadModel Qiniu_Rio_ST = {
        NULL, &Qiniu_Rio_ST_Itbl
};

/*============================================================================*/
/* type Qiniu_Rio_Settings */

static Qiniu_Rio_Settings settings = {
        defaultWorkers * 4,
        defaultWorkers,
        defaultChunkSize,
        defaultTryTimes,
        {NULL, &Qiniu_Rio_ST_Itbl}
};

void Qiniu_Rio_SetSettings(Qiniu_Rio_Settings *v) {
    settings = *v;
    if (settings.workers == 0) {
        settings.workers = defaultWorkers;
    }
    if (settings.taskQsize == 0) {
        settings.taskQsize = settings.workers * 4;
    }
    if (settings.chunkSize == 0) {
        settings.chunkSize = defaultChunkSize;
    }
    if (settings.tryTimes == 0) {
        settings.tryTimes = defaultTryTimes;
    }
    if (settings.threadModel.itbl == NULL) {
        settings.threadModel = Qiniu_Rio_ST;
    }
}

/*============================================================================*/
/* func Qiniu_UptokenAuth */

static Qiniu_Error Qiniu_UptokenAuth_Auth(
        void *self, Qiniu_Header **header, const char *url, const char *addition, size_t addlen) {
    Qiniu_Error err;

    *header = curl_slist_append(*header, self);

    err.code = 200;
    err.message = "OK";
    return err;
}

static void Qiniu_UptokenAuth_Release(void *self) {
    free(self);
}

static Qiniu_Auth_Itbl Qiniu_UptokenAuth_Itbl = {
        Qiniu_UptokenAuth_Auth,
        Qiniu_UptokenAuth_Release
};

static Qiniu_Auth Qiniu_UptokenAuth(const char *uptoken) {
    char *self = Qiniu_String_Concat2("Authorization: UpToken ", uptoken);
    Qiniu_Auth auth = {self, &Qiniu_UptokenAuth_Itbl};
    return auth;
}

/*============================================================================*/
/* type Qiniu_Rio_BlkputRet */

static void Qiniu_Rio_BlkputRet_Cleanup(Qiniu_Rio_BlkputRet *self) {
    if (self->ctx != NULL) {
        free((void *) self->ctx);
        memset(self, 0, sizeof(*self));
    }
}

static void Qiniu_Rio_BlkputRet_Assign(Qiniu_Rio_BlkputRet *self, Qiniu_Rio_BlkputRet *ret) {
    char *p;
    size_t n1 = 0, n2 = 0, n3 = 0;

    Qiniu_Rio_BlkputRet_Cleanup(self);

    *self = *ret;
    if (ret->ctx == NULL) {
        return;
    }

    n1 = strlen(ret->ctx) + 1;
    n3 = strlen(ret->host) + 1;
    if (ret->checksum) {
        n2 = strlen(ret->checksum) + 1;
    }

    p = (char *) malloc(n1 + n2 + n3);

    memcpy(p, ret->ctx, n1);
    self->ctx = p;

    memcpy(p + n1, ret->host, n3);
    self->host = p + n1;

    if (n2) {
        memcpy(p + n1 + n3, ret->checksum, n2);
        self->checksum = p + n1 + n3;
    }
}

/*============================================================================*/
/* type Qiniu_Rio_PutExtra */

static int notifyNil(void *self, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet *ret) { return QINIU_RIO_NOTIFY_OK; }

static int notifyErrNil(void *self, int blkIdx, int blkSize, Qiniu_Error err) { return QINIU_RIO_NOTIFY_OK; }

static Qiniu_Error ErrInvalidPutProgress = {
        Qiniu_Rio_InvalidPutProgress, "invalid put progress"
};

static Qiniu_Error Qiniu_Rio_PutExtra_Init(
        Qiniu_Rio_PutExtra *self, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra *extra) {
    size_t cbprog;
    int i, blockCnt = Qiniu_Rio_BlockCount(fsize);
    int fprog = (extra != NULL) && (extra->progresses != NULL);

    if (fprog && extra->blockCnt != (size_t) blockCnt) {
        return ErrInvalidPutProgress;
    }

    if (extra != NULL) {
        *self = *extra;
    } else {
        memset(self, 0, sizeof(Qiniu_Rio_PutExtra));
    }

    cbprog = sizeof(Qiniu_Rio_BlkputRet) * blockCnt;
    self->progresses = (Qiniu_Rio_BlkputRet *) malloc(cbprog);
    self->blockCnt = blockCnt;
    memset(self->progresses, 0, cbprog);
    if (fprog) {
        for (i = 0; i < blockCnt; i++) {
            Qiniu_Rio_BlkputRet_Assign(&self->progresses[i], &extra->progresses[i]);
        }
    }

    if (self->chunkSize == 0) {
        self->chunkSize = settings.chunkSize;
    }
    if (self->tryTimes == 0) {
        self->tryTimes = settings.tryTimes;
    }
    if (self->notify == NULL) {
        self->notify = notifyNil;
    }
    if (self->notifyErr == NULL) {
        self->notifyErr = notifyErrNil;
    }
    if (self->threadModel.itbl == NULL) {
        self->threadModel = settings.threadModel;
    }

    return Qiniu_OK;
}

static void Qiniu_Rio_PutExtra_Cleanup(Qiniu_Rio_PutExtra *self) {
    size_t i;
    for (i = 0; i < self->blockCnt; i++) {
        Qiniu_Rio_BlkputRet_Cleanup(&self->progresses[i]);
    }
    free(self->progresses);
    self->progresses = NULL;
    self->blockCnt = 0;
}

static Qiniu_Int64 Qiniu_Rio_PutExtra_ChunkSize(Qiniu_Rio_PutExtra *self) {
    if (self) {
        return self->chunkSize;
    }
    return settings.chunkSize;
}

static void Qiniu_Io_PutExtra_initFrom(Qiniu_Io_PutExtra *self, Qiniu_Rio_PutExtra *extra) {
    if (extra) {
        self->mimeType = extra->mimeType;
        self->localFileName = extra->localFileName;
    } else {
        memset(self, 0, sizeof(*self));
    }
}

/*============================================================================*/

static Qiniu_Error Qiniu_Rio_bput(
        Qiniu_Client *self, Qiniu_Rio_BlkputRet *ret, Qiniu_Reader body, int bodyLength, const char *url) {
    Qiniu_Rio_BlkputRet retFromResp;
    Qiniu_Json *root;

    Qiniu_Error err = Qiniu_Client_CallWithBinary(self, &root, url, body, bodyLength, NULL);
    if (err.code == 200) {
        retFromResp.ctx = Qiniu_Json_GetString(root, "ctx", NULL);
        retFromResp.checksum = Qiniu_Json_GetString(root, "checksum", NULL);
        retFromResp.host = Qiniu_Json_GetString(root, "host", NULL);
        retFromResp.crc32 = (Qiniu_Uint32) Qiniu_Json_GetInt64(root, "crc32", 0);
        retFromResp.offset = (Qiniu_Uint32) Qiniu_Json_GetInt64(root, "offset", 0);

        if (retFromResp.ctx == NULL || retFromResp.host == NULL || retFromResp.offset == 0) {
            err.code = 9998;
            err.message = "unexcepted response: invalid ctx, host or offset";
            return err;
        }

        Qiniu_Rio_BlkputRet_Assign(ret, &retFromResp);
    }

    return err;
}

static Qiniu_Error Qiniu_Rio_Mkblock(
        Qiniu_Client *self, Qiniu_Rio_BlkputRet *ret, int blkSize, Qiniu_Reader body, int bodyLength,
        Qiniu_Rio_PutExtra *extra) {
    Qiniu_Error err;
    const char *upHost = NULL;
    char *url = NULL;

    if (extra && (upHost = extra->upHost) == NULL) {
        upHost = QINIU_UP_HOST;
    } // if

    url = Qiniu_String_Format(128, "%s/mkblk/%d", upHost, blkSize);
    err = Qiniu_Rio_bput(self, ret, body, bodyLength, url);
    Qiniu_Free(url);

    return err;
}

static Qiniu_Error Qiniu_Rio_Blockput(
        Qiniu_Client *self, Qiniu_Rio_BlkputRet *ret, Qiniu_Reader body, int bodyLength) {
    char *url = Qiniu_String_Format(1024, "%s/bput/%s/%d", ret->host, ret->ctx, (int) ret->offset);
    Qiniu_Error err = Qiniu_Rio_bput(self, ret, body, bodyLength, url);
    Qiniu_Free(url);
    return err;
}

/*============================================================================*/

static Qiniu_Error ErrUnmatchedChecksum = {
        Qiniu_Rio_UnmatchedChecksum, "unmatched checksum"
};

static int Qiniu_TemporaryError(int code) {
    return code != 401;
}

static Qiniu_Error Qiniu_Rio_ResumableBlockput(
        Qiniu_Client *c, Qiniu_Rio_BlkputRet *ret, Qiniu_ReaderAt f, int blkIdx, int blkSize,
        Qiniu_Rio_PutExtra *extra) {
    Qiniu_Error err = {200, NULL};
    Qiniu_Tee tee;
    Qiniu_Section section;
    Qiniu_Reader body, body1;

    Qiniu_Crc32 crc32;
    Qiniu_Writer h = Qiniu_Crc32Writer(&crc32, 0);
    Qiniu_Int64 offbase = (Qiniu_Int64) (blkIdx) << blockBits;

    int chunkSize = extra->chunkSize;
    int bodyLength;
    int tryTimes;
    int notifyRet = 0;

    if (ret->ctx == NULL) {

        if (chunkSize < blkSize) {
            bodyLength = chunkSize;
        } else {
            bodyLength = blkSize;
        }

        body1 = Qiniu_SectionReader(&section, f, (Qiniu_Off_T) offbase, bodyLength);
        body = Qiniu_TeeReader(&tee, body1, h);

        err = Qiniu_Rio_Mkblock(c, ret, blkSize, body, bodyLength, extra);
        if (err.code != 200) {
            return err;
        }
        if (ret->crc32 != crc32.val || (int) (ret->offset) != bodyLength) {
            return ErrUnmatchedChecksum;
        }
        notifyRet = extra->notify(extra->notifyRecvr, blkIdx, blkSize, ret);
        if (notifyRet == QINIU_RIO_NOTIFY_EXIT) {
            // Terminate the upload process if  the caller requests
            err.code = Qiniu_Rio_PutInterrupted;
            err.message = "Interrupted by the caller";
            return err;
        }
    }

    while ((int) (ret->offset) < blkSize) {

        if (chunkSize < blkSize - (int) (ret->offset)) {
            bodyLength = chunkSize;
        } else {
            bodyLength = blkSize - (int) (ret->offset);
        }

        tryTimes = extra->tryTimes;

        lzRetry:
        crc32.val = 0;
        body1 = Qiniu_SectionReader(&section, f, (Qiniu_Off_T) offbase + (ret->offset), bodyLength);
        body = Qiniu_TeeReader(&tee, body1, h);

        err = Qiniu_Rio_Blockput(c, ret, body, bodyLength);
        if (err.code == 200) {
            if (ret->crc32 == crc32.val) {
                notifyRet = extra->notify(extra->notifyRecvr, blkIdx, blkSize, ret);
                if (notifyRet == QINIU_RIO_NOTIFY_EXIT) {
                    // Terminate the upload process if the caller requests
                    err.code = Qiniu_Rio_PutInterrupted;
                    err.message = "Interrupted by the caller";
                    return err;
                }

                continue;
            }
            Qiniu_Log_Warn("ResumableBlockput: invalid checksum, retry");
            err = ErrUnmatchedChecksum;
        } else {
            if (err.code == Qiniu_Rio_InvalidCtx) {
                Qiniu_Rio_BlkputRet_Cleanup(ret); // reset
                Qiniu_Log_Warn("ResumableBlockput: invalid ctx, please retry");
                return err;
            }
            Qiniu_Log_Warn("ResumableBlockput %d off:%d failed - %E", blkIdx, (int) ret->offset, err);
        }
        if (tryTimes > 1 && Qiniu_TemporaryError(err.code)) {
            tryTimes--;
            Qiniu_Log_Info("ResumableBlockput %E, retrying ...", err);
            goto lzRetry;
        }
        break;
    }
    return err;
}

/*============================================================================*/

static Qiniu_Error Qiniu_Rio_Mkfile(
        Qiniu_Client *c, Qiniu_Rio_PutRet *ret, const char *key, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra *extra) {
    size_t i, blkCount = extra->blockCnt;
    Qiniu_Json *root;
    Qiniu_Error err;
    const char *upHost = NULL;
    Qiniu_Rio_BlkputRet *prog;
    Qiniu_Buffer url, body;
    int j = 0;

    Qiniu_Buffer_Init(&url, 2048);

    if ((upHost = extra->upHost) == NULL) {
        upHost = QINIU_UP_HOST;
    } // if

    Qiniu_Buffer_AppendFormat(&url, "%s/mkfile/%D", upHost, fsize);

    if (key != NULL) {
        // Allow using empty key
        Qiniu_Buffer_AppendFormat(&url, "/key/%S", key);
    }
    if (extra->xVarsList != NULL && extra->xVarsCount > 0) {
        for (j = 0; j < extra->xVarsCount; j += 1) {
            Qiniu_Buffer_AppendFormat(&url, "/%s/%S", (extra->xVarsList[j])[0], (extra->xVarsList[j])[1]);
        } // for
    }

    Qiniu_Buffer_Init(&body, 176 * blkCount);
    for (i = 0; i < blkCount; i++) {
        prog = &extra->progresses[i];
        Qiniu_Buffer_Write(&body, prog->ctx, strlen(prog->ctx));
        Qiniu_Buffer_PutChar(&body, ',');
    }
    if (blkCount > 0) {
        body.curr--;
    }

    err = Qiniu_Client_CallWithBuffer(
            c, &root, Qiniu_Buffer_CStr(&url), body.buf, body.curr - body.buf, "text/plain");

    Qiniu_Buffer_Cleanup(&url);
    Qiniu_Buffer_Cleanup(&body);

    if (err.code == 200) {
        ret->hash = Qiniu_Json_GetString(root, "hash", NULL);
        ret->key = Qiniu_Json_GetString(root, "key", NULL);
        ret->persistentId = Qiniu_Json_GetString(root, "persistentId", NULL);
    }
    return err;
}

/*============================================================================*/

int Qiniu_Rio_BlockCount(Qiniu_Int64 fsize) {
    return (int) ((fsize + blockMask) >> blockBits);
}

/*============================================================================*/
/* type Qiniu_Rio_task */

typedef struct _Qiniu_Rio_task {
    Qiniu_ReaderAt f;
    Qiniu_Client *mc;
    Qiniu_Rio_PutExtra *extra;
    Qiniu_Rio_WaitGroup wg;
    int *nfails;
    Qiniu_Count *ninterrupts;
    int blkIdx;
    int blkSize1;
} Qiniu_Rio_task;

static void Qiniu_Rio_doTask(void *params) {
    Qiniu_Error err;
    Qiniu_Rio_BlkputRet ret;
    Qiniu_Rio_task *task = (Qiniu_Rio_task *) params;
    Qiniu_Rio_WaitGroup wg = task->wg;
    Qiniu_Rio_PutExtra *extra = task->extra;
    Qiniu_Rio_ThreadModel tm = extra->threadModel;
    Qiniu_Client *c = tm.itbl->ClientTls(tm.self, task->mc);
    int blkIdx = task->blkIdx;
    int tryTimes = extra->tryTimes;

    if ((*task->ninterrupts) > 0) {
        free(task);
        Qiniu_Count_Inc(task->ninterrupts);
        wg.itbl->Done(wg.self);
        return;
    }

    memset(&ret, 0, sizeof(ret));

    lzRetry:
    Qiniu_Rio_BlkputRet_Assign(&ret, &extra->progresses[blkIdx]);
    err = Qiniu_Rio_ResumableBlockput(c, &ret, task->f, blkIdx, task->blkSize1, extra);
    if (err.code != 200) {
        if (err.code == Qiniu_Rio_PutInterrupted) {
            // Terminate the upload process if the caller requests
            Qiniu_Rio_BlkputRet_Cleanup(&ret);
            Qiniu_Count_Inc(task->ninterrupts);
            free(task);
            wg.itbl->Done(wg.self);
            return;
        }

        if (tryTimes > 1 && Qiniu_TemporaryError(err.code)) {
            tryTimes--;
            Qiniu_Log_Info("resumable.Put %E, retrying ...", err);
            goto lzRetry;
        }
        Qiniu_Log_Warn("resumable.Put %d failed: %E", blkIdx, err);
        extra->notifyErr(extra->notifyRecvr, task->blkIdx, task->blkSize1, err);
        (*task->nfails)++;
    } else {
        Qiniu_Rio_BlkputRet_Assign(&extra->progresses[blkIdx], &ret);
    }
    Qiniu_Rio_BlkputRet_Cleanup(&ret);
    free(task);
    wg.itbl->Done(wg.self);
}

/*============================================================================*/
/* func Qiniu_Rio_PutXXX */

static Qiniu_Error ErrPutFailed = {
        Qiniu_Rio_PutFailed, "resumable put failed"
};
static Qiniu_Error ErrPutInterrupted = {
        Qiniu_Rio_PutInterrupted, "resumable put interrupted"
};

Qiniu_Error Qiniu_Rio_Put(
        Qiniu_Client *self, Qiniu_Rio_PutRet *ret,
        const char *uptoken, const char *key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra *extra1) {
    Qiniu_Int64 offbase;
    Qiniu_Rio_task *task;
    Qiniu_Rio_WaitGroup wg;
    Qiniu_Rio_PutExtra extra;
    Qiniu_Rio_ThreadModel tm;
    Qiniu_Auth auth, auth1 = self->auth;
    int i, last, blkSize;
    int nfails;
    int retCode;
    Qiniu_Count ninterrupts;
    Qiniu_Error err = Qiniu_Rio_PutExtra_Init(&extra, fsize, extra1);
    if (err.code != 200) {
        return err;
    }

    tm = extra.threadModel;
    wg = tm.itbl->WaitGroup(tm.self);

    last = extra.blockCnt - 1;
    blkSize = 1 << blockBits;
    nfails = 0;
    ninterrupts = 0;

    self->auth = auth = Qiniu_UptokenAuth(uptoken);

    for (i = 0; i < (int) extra.blockCnt; i++) {
        task = (Qiniu_Rio_task *) malloc(sizeof(Qiniu_Rio_task));
        task->f = f;
        task->extra = &extra;
        task->mc = self;
        task->wg = wg;
        task->nfails = &nfails;
        task->ninterrupts = &ninterrupts;
        task->blkIdx = i;
        task->blkSize1 = blkSize;
        if (i == last) {
            offbase = (Qiniu_Int64) (i) << blockBits;
            task->blkSize1 = (int) (fsize - offbase);
        }

        wg.itbl->Add(wg.self, 1);
        retCode = tm.itbl->RunTask(tm.self, Qiniu_Rio_doTask, task);
        if (retCode == QINIU_RIO_NOTIFY_EXIT) {
            wg.itbl->Done(wg.self);
            Qiniu_Count_Inc(&ninterrupts);
            free(task);
        }

        if (ninterrupts > 0) {
            break;
        }
    } // for

    wg.itbl->Wait(wg.self);
    if (nfails != 0) {
        err = ErrPutFailed;
    } else if (ninterrupts != 0) {
        err = ErrPutInterrupted;
    } else {
        err = Qiniu_Rio_Mkfile(self, ret, key, fsize, &extra);
    }

    Qiniu_Rio_PutExtra_Cleanup(&extra);

    wg.itbl->Release(wg.self);
    auth.itbl->Release(auth.self);
    self->auth = auth1;
    return err;
}

Qiniu_Error Qiniu_Rio_PutFile(
        Qiniu_Client *self, Qiniu_Rio_PutRet *ret,
        const char *uptoken, const char *key, const char *localFile, Qiniu_Rio_PutExtra *extra) {
    Qiniu_Io_PutExtra extra1;
    Qiniu_Int64 fsize;
    Qiniu_FileInfo fi;
    Qiniu_File *f;
    Qiniu_Error err = Qiniu_File_Open(&f, localFile);
    if (err.code != 200) {
        return err;
    }
    err = Qiniu_File_Stat(f, &fi);
    if (err.code == 200) {
        fsize = Qiniu_FileInfo_Fsize(fi);
        if (fsize <= Qiniu_Rio_PutExtra_ChunkSize(extra)) { // file is too small, don't need resumable-io
            Qiniu_File_Close(f);

            Qiniu_Zero(extra1);
            Qiniu_Io_PutExtra_initFrom(&extra1, extra);

            return Qiniu_Io_PutFile(self, ret, uptoken, key, localFile, &extra1);
        }
        err = Qiniu_Rio_Put(self, ret, uptoken, key, Qiniu_FileReaderAt(f), fsize, extra);
    }
    Qiniu_File_Close(f);
    return err;
}

