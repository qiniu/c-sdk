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
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include "recorder_key.h"
#include "recorder_utils.h"
#include "../cJSON/cJSON.h"
#include "../hashmap/hashmap.h"
#include "private/region.h"
#include "private/code.h"

#define blockBits 22
#define blockMask ((1 << blockBits) - 1)

#define defaultTryTimes 3
#define defaultWorkers 4
#define defaultChunkSize (4 * 1024 * 1024) // 4MB

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

static void Qiniu_Rio_MTWG_Add(void *self, int n)
{
    Qiniu_Count_Inc(&((Qiniu_Rio_MTWG_Data *)self)->addedCount);
} // Qiniu_Rio_MTWG_Add

static void Qiniu_Rio_MTWG_Done(void *self)
{
    Qiniu_Count_Inc(&((Qiniu_Rio_MTWG_Data *)self)->doneCount);
    SetEvent(((Qiniu_Rio_MTWG_Data *)self)->event);
} // Qiniu_Rio_MTWG_Done

static void Qiniu_Rio_MTWG_Wait(void *self)
{
    Qiniu_Rio_MTWG_Data *data = (Qiniu_Rio_MTWG_Data *)self;
    Qiniu_Count lastDoneCount = data->doneCount;
    DWORD ret = 0;

    while (lastDoneCount != data->addedCount)
    {
        ret = WaitForSingleObject(((Qiniu_Rio_MTWG_Data *)self)->event, INFINITE);
        if (ret == WAIT_OBJECT_0)
        {
            lastDoneCount = data->doneCount;
        }
    } // while
} // Qiniu_Rio_MTWG_Wait

static void Qiniu_Rio_MTWG_Release(void *self)
{
    CloseHandle(((Qiniu_Rio_MTWG_Data *)self)->event);
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
    Qiniu_Rio_MTWG_Data *newData = NULL;

    newData = (Qiniu_Rio_MTWG_Data *)malloc(sizeof(*newData));
    newData->addedCount = 0;
    newData->doneCount = 0;
    newData->event = CreateEvent(NULL, FALSE, FALSE, NULL);

    wg.itbl = &Qiniu_Rio_MTWG_Itbl;
    wg.self = newData;
    return wg;
} // Qiniu_Rio_MTWG_Create

#endif

static void Qiniu_Rio_STWG_Add(void *self, int n)
{
}

static void Qiniu_Rio_STWG_Done(void *self) {}

static void Qiniu_Rio_STWG_Wait(void *self) {}

static void Qiniu_Rio_STWG_Release(void *self) {}

static Qiniu_Rio_WaitGroup_Itbl Qiniu_Rio_STWG_Itbl = {
    Qiniu_Rio_STWG_Add,
    Qiniu_Rio_STWG_Done,
    Qiniu_Rio_STWG_Wait,
    Qiniu_Rio_STWG_Release};

static Qiniu_Rio_WaitGroup Qiniu_Rio_STWG = {
    NULL, &Qiniu_Rio_STWG_Itbl};

static Qiniu_Rio_WaitGroup Qiniu_Rio_ST_WaitGroup(void *self)
{
    return Qiniu_Rio_STWG;
}

static Qiniu_Client *Qiniu_Rio_ST_ClientTls(void *self, Qiniu_Client *mc)
{
    return mc;
}

static int Qiniu_Rio_ST_RunTask(void *self, void (*task)(void *params), void *params)
{
    task(params);
    return QINIU_RIO_NOTIFY_OK;
}

static Qiniu_Rio_ThreadModel_Itbl Qiniu_Rio_ST_Itbl = {
    Qiniu_Rio_ST_WaitGroup,
    Qiniu_Rio_ST_ClientTls,
    Qiniu_Rio_ST_RunTask};

Qiniu_Rio_ThreadModel Qiniu_Rio_ST = {
    NULL, &Qiniu_Rio_ST_Itbl};

/*============================================================================*/
/* type Qiniu_Rio_Settings */

static Qiniu_Rio_Settings settings = {
    defaultWorkers * 4,
    defaultWorkers,
    defaultChunkSize,
    defaultTryTimes,
    {NULL, &Qiniu_Rio_ST_Itbl}};

void Qiniu_Rio_SetSettings(Qiniu_Rio_Settings *v)
{
    settings = *v;
    if (settings.workers == 0)
    {
        settings.workers = defaultWorkers;
    }
    if (settings.taskQsize == 0)
    {
        settings.taskQsize = settings.workers * 4;
    }
    if (settings.chunkSize == 0)
    {
        settings.chunkSize = defaultChunkSize;
    }
    if (settings.tryTimes == 0)
    {
        settings.tryTimes = defaultTryTimes;
    }
    if (settings.threadModel.itbl == NULL)
    {
        settings.threadModel = Qiniu_Rio_ST;
    }
}

/*============================================================================*/
/* type Qiniu_Rio_Recorder */
typedef struct _Qiniu_Rio_Recorder
{
    Qiniu_Record_Medium *recorderMedium;
    const char *recorderKey;
    Qiniu_Bool toLoadProgresses;
} Qiniu_Rio_Recorder;

static void Qiniu_Rio_Recorder_Cleanup(Qiniu_Rio_Recorder *recorder)
{
    if (recorder != NULL)
    {
        if (recorder->recorderMedium != NULL)
        {
            recorder->recorderMedium->close(recorder->recorderMedium);
            recorder->recorderMedium = NULL;
        }
        if (recorder->recorderKey != NULL)
        {
            Qiniu_FreeV2((void **)&recorder->recorderKey);
        }
    }
}

/*============================================================================*/
/* type Qiniu_Rio_BlkputRet */

static void Qiniu_Rio_BlkputRet_Cleanup(Qiniu_Rio_BlkputRet *self)
{
    Qiniu_Free((void *)self->ctx);
    if (self->economical == Qiniu_False)
    {
        Qiniu_Free((void *)self->checksum);
        Qiniu_Free((void *)self->host);
    }
    Qiniu_Zero_Ptr(self);
}

static void Qiniu_Rio_BlkputRet_Assign(Qiniu_Rio_BlkputRet *self, Qiniu_Rio_BlkputRet *ret)
{
    char *p;
    size_t n1 = 0, n2 = 0, n3 = 0;

    Qiniu_Rio_BlkputRet_Cleanup(self);

    *self = *ret;
    if (ret->ctx == NULL)
    {
        return;
    }

    n1 = strlen(ret->ctx) + 1;
    n3 = strlen(ret->host) + 1;
    if (ret->checksum)
    {
        n2 = strlen(ret->checksum) + 1;
    }

    p = (char *)malloc(n1 + n2 + n3);

    memcpy(p, ret->ctx, n1);
    self->ctx = p;

    memcpy(p + n1, ret->host, n3);
    self->host = p + n1;

    if (n2)
    {
        memcpy(p + n1 + n3, ret->checksum, n2);
        self->checksum = p + n1 + n3;
    }
    self->economical = Qiniu_True;
}

/*============================================================================*/
/* type Qiniu_Rio_PutExtra */

static int notifyNil(void *self, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet *ret) { return QINIU_RIO_NOTIFY_OK; }

static int notifyErrNil(void *self, int blkIdx, int blkSize, Qiniu_Error err) { return QINIU_RIO_NOTIFY_OK; }

static Qiniu_Error ErrInvalidPutProgress = {
    Qiniu_Rio_InvalidPutProgress, "invalid put progress"};

static Qiniu_Error Qiniu_Rio_PutExtra_Init(
    Qiniu_Rio_PutExtra *self, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra *extra)
{
    size_t cbprog;
    int i, blockCnt = Qiniu_Rio_BlockCount(fsize);
    int fprog = (extra != NULL) && (extra->progresses != NULL);

    if (fprog && extra->blockCnt != (size_t)blockCnt)
    {
        return ErrInvalidPutProgress;
    }

    if (extra != NULL)
    {
        *self = *extra;
    }
    else
    {
        Qiniu_Zero_Ptr(self);
    }

    cbprog = sizeof(Qiniu_Rio_BlkputRet) * blockCnt;
    self->progresses = (Qiniu_Rio_BlkputRet *)malloc(cbprog);
    self->blockCnt = blockCnt;
    memset(self->progresses, 0, cbprog);
    if (fprog)
    {
        for (i = 0; i < blockCnt; i++)
        {
            Qiniu_Rio_BlkputRet_Assign(&self->progresses[i], &extra->progresses[i]);
        }
    }

    if (self->chunkSize == 0)
    {
        self->chunkSize = settings.chunkSize;
    }
    if (self->tryTimes == 0)
    {
        self->tryTimes = settings.tryTimes;
    }
    if (self->notify == NULL)
    {
        self->notify = notifyNil;
    }
    if (self->notifyErr == NULL)
    {
        self->notifyErr = notifyErrNil;
    }
    if (self->threadModel.itbl == NULL)
    {
        self->threadModel = settings.threadModel;
    }

    return Qiniu_OK;
}

static void Qiniu_Rio_PutExtra_Clear(Qiniu_Rio_PutExtra *self)
{
    for (size_t i = 0; i < self->blockCnt; i++)
    {
        Qiniu_Rio_BlkputRet_Cleanup(&self->progresses[i]);
    }
}

static void Qiniu_Rio_PutExtra_Cleanup(Qiniu_Rio_PutExtra *self)
{
    Qiniu_Rio_PutExtra_Clear(self);
    Qiniu_FreeV2((void **)&self->progresses);
    self->blockCnt = 0;
}

static Qiniu_Int64 Qiniu_Rio_PutExtra_ChunkSize(Qiniu_Rio_PutExtra *self)
{
    if (self)
    {
        return self->chunkSize;
    }
    return settings.chunkSize;
}

static void Qiniu_Io_PutExtra_initFrom(Qiniu_Io_PutExtra *self, Qiniu_Rio_PutExtra *extra)
{
    if (extra)
    {
        self->mimeType = extra->mimeType;
        self->localFileName = extra->localFileName;
    }
    else
    {
        Qiniu_Zero_Ptr(self);
    }
}

/*============================================================================*/

struct _Qiniu_Uploading_Parts_Progress
{
    size_t uploaded;
    struct hashmap *uploading;
    Qiniu_Mutex mutex;
};

struct _Qiniu_Uploading_Parts_Progress_Pair
{
    int blkIdx;
    size_t uploaded;
};

static uint64_t _Qiniu_Uploading_Parts_Progress_Hash(const void *item, uint64_t seed0, uint64_t seed1)
{
    const struct _Qiniu_Uploading_Parts_Progress_Pair *pair = (const struct _Qiniu_Uploading_Parts_Progress_Pair *)item;
    return hashmap_sip(&pair->blkIdx, sizeof(pair->blkIdx), seed0, seed1);
}

static int _Qiniu_Uploading_Parts_Progress_Compare(const void *a, const void *b, void *udata)
{
    const struct _Qiniu_Uploading_Parts_Progress_Pair *left = (const struct _Qiniu_Uploading_Parts_Progress_Pair *)a;
    const struct _Qiniu_Uploading_Parts_Progress_Pair *right = (const struct _Qiniu_Uploading_Parts_Progress_Pair *)b;
    return left->blkIdx - right->blkIdx;
}

static struct _Qiniu_Uploading_Parts_Progress *_Qiniu_Uploading_Parts_Progress_New()
{
    struct _Qiniu_Uploading_Parts_Progress *data = (struct _Qiniu_Uploading_Parts_Progress *)malloc(sizeof(struct _Qiniu_Uploading_Parts_Progress));
    Qiniu_Zero_Ptr(data);
    Qiniu_Mutex_Init(&data->mutex);
    data->uploading = hashmap_new(
        sizeof(struct _Qiniu_Uploading_Parts_Progress_Pair), 0, rand(), rand(),
        _Qiniu_Uploading_Parts_Progress_Hash, _Qiniu_Uploading_Parts_Progress_Compare,
        free, NULL);
    return data;
}

static void _Qiniu_Uploading_Parts_Progress_Free(struct _Qiniu_Uploading_Parts_Progress *data)
{
    hashmap_free(data->uploading);
    Qiniu_Mutex_Cleanup(&data->mutex);
    free((void *)data);
}

static void _Qiniu_Uploading_Parts_Progress_Set_Progress(struct _Qiniu_Uploading_Parts_Progress *progress, int blkIdx, size_t uploaded)
{
    Qiniu_Mutex_Lock(&progress->mutex);
    const struct _Qiniu_Uploading_Parts_Progress_Pair pair = {.blkIdx = blkIdx};
    struct _Qiniu_Uploading_Parts_Progress_Pair *pPair = (struct _Qiniu_Uploading_Parts_Progress_Pair *)hashmap_get(progress->uploading, &pair);
    if (pPair == NULL)
    {
        pPair = (struct _Qiniu_Uploading_Parts_Progress_Pair *)malloc(sizeof(struct _Qiniu_Uploading_Parts_Progress_Pair));
        pPair->blkIdx = blkIdx;
        pPair->uploaded = uploaded;
        hashmap_set(progress->uploading, pPair);
    }
    else
    {
        pPair->uploaded = uploaded;
    }

    Qiniu_Mutex_Unlock(&progress->mutex);
}

static void _Qiniu_Uploading_Parts_Progress_Part_Uploaded(struct _Qiniu_Uploading_Parts_Progress *progress, int blkIdx, size_t partSize)
{
    Qiniu_Mutex_Lock(&progress->mutex);
    const struct _Qiniu_Uploading_Parts_Progress_Pair pair = {.blkIdx = blkIdx};
    hashmap_delete(progress->uploading, (const void *)&pair);
    progress->uploaded += partSize;
    Qiniu_Mutex_Unlock(&progress->mutex);
}

static size_t _Qiniu_Uploading_Parts_Progress_Get_Total_Size(struct _Qiniu_Uploading_Parts_Progress *progress)
{
    Qiniu_Mutex_Lock(&progress->mutex);
    size_t total = progress->uploaded, iter = 0;
    void *iter_val;
    while (hashmap_iter(progress->uploading, &iter, &iter_val))
    {
        struct _Qiniu_Uploading_Parts_Progress_Pair *pair = (struct _Qiniu_Uploading_Parts_Progress_Pair *)iter_val;
        total += pair->uploaded;
    }
    Qiniu_Mutex_Unlock(&progress->mutex);
    return total;
}

struct _Qiniu_Progress_Callback_Data
{
    size_t totalSize, previousUlNow;
    int blkIdx;
    struct _Qiniu_Uploading_Parts_Progress *progress;
    void (*callback)(size_t, size_t);
};

static int _Qiniu_Progress_Callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    struct _Qiniu_Progress_Callback_Data *data = (struct _Qiniu_Progress_Callback_Data *)clientp;
    if (data->previousUlNow != (size_t)ulnow)
    {
        _Qiniu_Uploading_Parts_Progress_Set_Progress(data->progress, data->blkIdx, (size_t)ulnow);
        data->callback((size_t)data->totalSize, _Qiniu_Uploading_Parts_Progress_Get_Total_Size(data->progress));
        data->previousUlNow = (size_t)ulnow;
    }
    return 0;
}

static Qiniu_Error Qiniu_Rio_bput(Qiniu_Client *self, Qiniu_Rio_BlkputRet *ret, Qiniu_Reader body, int bodyLength, const char *url, struct _Qiniu_Progress_Callback_Data *progressCallback)
{
    Qiniu_Rio_BlkputRet retFromResp;
    Qiniu_Json *root;
    int (*callback)(void *, double, double, double, double) = NULL;
    void *callbackData = NULL;
    if (progressCallback != NULL && progressCallback->callback != NULL)
    {
        callback = _Qiniu_Progress_Callback;
        callbackData = (void *)progressCallback;
    }

    Qiniu_Error err = Qiniu_Client_CallWithBinaryAndProgressCallback(self, &root, url, body, bodyLength, NULL, callback, callbackData);
    if (err.code == 200)
    {
        retFromResp.ctx = Qiniu_Json_GetString(root, "ctx", NULL);
        retFromResp.checksum = Qiniu_Json_GetString(root, "checksum", NULL);
        retFromResp.host = Qiniu_Json_GetString(root, "host", NULL);
        retFromResp.crc32 = Qiniu_Json_GetUInt32(root, "crc32", 0);
        retFromResp.offset = Qiniu_Json_GetUInt32(root, "offset", 0);
        retFromResp.expiredAt = Qiniu_Json_GetUInt64(root, "expired_at", 0);

        if (retFromResp.ctx == NULL || retFromResp.host == NULL || retFromResp.offset == 0)
        {
            err.code = 9998;
            err.message = "unexpected response: invalid ctx, host or offset";
            return err;
        }

        Qiniu_Rio_BlkputRet_Assign(ret, &retFromResp);
    }

    return err;
}

static Qiniu_Error Qiniu_Rio_Mkblock(
    Qiniu_Client *self, Qiniu_Rio_BlkputRet *ret, int blkSize, const char *upHost, Qiniu_Reader body, int bodyLength, struct _Qiniu_Progress_Callback_Data *progressCallback)
{
    const char *url = Qiniu_String_Format(128, "%s/mkblk/%d", upHost, blkSize);
    Qiniu_Error err = Qiniu_Rio_bput(self, ret, body, bodyLength, url, progressCallback);
    Qiniu_Free((void *)url);

    return err;
}

static Qiniu_Error Qiniu_Rio_Blockput(
    Qiniu_Client *self, Qiniu_Rio_BlkputRet *ret, const char *upHost, Qiniu_Reader body, int bodyLength, struct _Qiniu_Progress_Callback_Data *progressCallback)
{
    const char *url = Qiniu_String_Format(1024, "%s/bput/%s/%d", upHost, ret->ctx, (int)ret->offset);
    Qiniu_Error err = Qiniu_Rio_bput(self, ret, body, bodyLength, url, progressCallback);
    Qiniu_Free((void *)url);
    return err;
}

/*============================================================================*/

static Qiniu_Error ErrUnmatchedChecksum = {
    Qiniu_Rio_UnmatchedChecksum, "unmatched checksum"};

static Qiniu_Error
Qiniu_Rio_ResumableBlockput(
    Qiniu_Client *c, Qiniu_Rio_BlkputRet *ret, Qiniu_ReaderAt f, int blkIdx, int blkSize, int fsize,
    struct _Qiniu_Uploading_Parts_Progress *uploadingPartsProgress, Qiniu_Rio_PutExtra *extra, size_t *pChunksUploaded)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Tee tee;
    Qiniu_Section section;
    Qiniu_Reader body, body1;
    const char *const *upHosts;
    const char *upHost;
    size_t upHostsCount;

    Qiniu_Crc32 crc32;
    Qiniu_Writer h = Qiniu_Crc32Writer(&crc32, 0);
    Qiniu_Int64 offbase = (Qiniu_Int64)(blkIdx) << blockBits;

    int chunkSize = extra->chunkSize;
    int bodyLength;
    int tryTimes;
    int notifyRet = 0;
    size_t chunksUploaded = 0;

    struct _Qiniu_Progress_Callback_Data progressCallbackData;
    Qiniu_Zero(progressCallbackData);

    if (extra->uploadingProgress != NULL)
    {
        progressCallbackData.callback = extra->uploadingProgress;
        progressCallbackData.totalSize = (size_t)fsize;
        progressCallbackData.progress = uploadingPartsProgress;
        progressCallbackData.blkIdx = blkIdx;
    }

    if (extra->upHost != NULL)
    {
        upHosts = &extra->upHost;
        upHostsCount = 1;
    }
    else
    {
        upHosts = extra->upHosts;
        upHostsCount = extra->upHostsCount;
    }

    if (ret->ctx == NULL)
    {
        if (chunkSize < blkSize)
        {
            bodyLength = chunkSize;
        }
        else
        {
            bodyLength = blkSize;
        }

        for (int tries = 0; tries < extra->tryTimes && tries <= c->hostsRetriesMax; tries++)
        {
            body1 = Qiniu_SectionReader(&section, f, (Qiniu_Off_T)offbase, bodyLength);
            body = Qiniu_TeeReader(&tee, body1, h);

            upHost = upHosts[tries % upHostsCount];
            err = Qiniu_Rio_Mkblock(c, ret, blkSize, upHost, body, bodyLength, &progressCallbackData);
            if (err.code == 200)
            {
                if (extra->uploadingProgress != NULL)
                {
                    _Qiniu_Uploading_Parts_Progress_Part_Uploaded(uploadingPartsProgress, blkIdx, bodyLength);
                }
                break;
            }
            else
            {
                if (extra->uploadingProgress != NULL)
                {
                    _Qiniu_Uploading_Parts_Progress_Set_Progress(uploadingPartsProgress, blkIdx, 0);
                }
                if (_Qiniu_Should_Retry(err.code) == QINIU_DONT_RETRY)
                {
                    goto handleErr;
                }
            }
        }
        if (err.code != 200)
        {
            goto handleErr;
        }
        chunksUploaded++;
        if (ret->crc32 != crc32.val || (int)(ret->offset) != bodyLength)
        {
            err = ErrUnmatchedChecksum;
            goto handleErr;
        }
        notifyRet = extra->notify(extra->notifyRecvr, blkIdx, blkSize, ret);
        if (notifyRet == QINIU_RIO_NOTIFY_EXIT)
        {
            // Terminate the upload process if  the caller requests
            err.code = Qiniu_Rio_PutInterrupted;
            err.message = "Interrupted by the caller";
            goto handleErr;
        }
    }
    else if (extra->uploadingProgress != NULL)
    {
        _Qiniu_Uploading_Parts_Progress_Part_Uploaded(uploadingPartsProgress, blkIdx, ret->offset);
    }

    while ((int)(ret->offset) < blkSize)
    {

        if (chunkSize < blkSize - (int)(ret->offset))
        {
            bodyLength = chunkSize;
        }
        else
        {
            bodyLength = blkSize - (int)(ret->offset);
        }

        tryTimes = extra->tryTimes;
        if (tryTimes > c->hostsRetriesMax + 1)
        {
            tryTimes = c->hostsRetriesMax + 1;
        }

    lzRetry:
        crc32.val = 0;
        body1 = Qiniu_SectionReader(&section, f, (Qiniu_Off_T)offbase + (ret->offset), bodyLength);
        body = Qiniu_TeeReader(&tee, body1, h);
        upHost = upHosts[tryTimes % upHostsCount];

        err = Qiniu_Rio_Blockput(c, ret, upHost, body, bodyLength, &progressCallbackData);
        if (err.code == 200)
        {
            chunksUploaded++;
            if (ret->crc32 == crc32.val)
            {
                notifyRet = extra->notify(extra->notifyRecvr, blkIdx, blkSize, ret);
                if (notifyRet == QINIU_RIO_NOTIFY_EXIT)
                {
                    // Terminate the upload process if the caller requests
                    err.code = Qiniu_Rio_PutInterrupted;
                    err.message = "Interrupted by the caller";
                    goto handleErr;
                }
                if (extra->uploadingProgress != NULL)
                {
                    _Qiniu_Uploading_Parts_Progress_Part_Uploaded(uploadingPartsProgress, blkIdx, bodyLength);
                }
                continue;
            }
            else
            {
                Qiniu_Log_Warn("ResumableBlockput: invalid checksum, retry");
                err = ErrUnmatchedChecksum;
                if (extra->uploadingProgress != NULL)
                {
                    _Qiniu_Uploading_Parts_Progress_Set_Progress(uploadingPartsProgress, blkIdx, 0);
                }
            }
        }
        else if (err.code == Qiniu_Rio_InvalidCtx)
        {
            Qiniu_Rio_BlkputRet_Cleanup(ret); // reset
            Qiniu_Log_Warn("ResumableBlockput: invalid ctx, please retry");
            goto handleErr;
        }
        else
        {
            Qiniu_Log_Warn("ResumableBlockput %d off:%d failed - %E", blkIdx, (int)ret->offset, err);
            if (extra->uploadingProgress != NULL)
            {
                _Qiniu_Uploading_Parts_Progress_Set_Progress(uploadingPartsProgress, blkIdx, 0);
            }
        }
        if (tryTimes > 1 && _Qiniu_Should_Retry(err.code) != QINIU_DONT_RETRY)
        {
            tryTimes--;
            Qiniu_Log_Info("ResumableBlockput %E, retrying ...", err);
            goto lzRetry;
        }
        break;
    }

handleErr:
    if (pChunksUploaded != NULL)
    {
        *pChunksUploaded = chunksUploaded;
    }

    return err;
}

/*============================================================================*/

static Qiniu_Error Qiniu_Rio_Mkfile(
    Qiniu_Client *c, Qiniu_Rio_PutRet *ret, const char *upHost, const char *key, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra *extra)
{
    size_t i, blkCount = extra->blockCnt;
    Qiniu_Json *root;
    Qiniu_Rio_BlkputRet *prog;
    Qiniu_Buffer url, body;
    int j = 0;

    Qiniu_Buffer_Init(&url, 2048);
    Qiniu_Buffer_AppendFormat(&url, "%s/mkfile/%D", upHost, fsize);

    if (key != NULL)
    {
        // Allow using empty key
        Qiniu_Buffer_AppendFormat(&url, "/key/%S", key);
    }
    if (extra->xVarsList != NULL && extra->xVarsCount > 0)
    {
        for (j = 0; j < extra->xVarsCount; j += 1)
        {
            Qiniu_Buffer_AppendFormat(&url, "/%s/%S", (extra->xVarsList[j])[0], (extra->xVarsList[j])[1]);
        } // for
    }

    Qiniu_Buffer_Init(&body, 176 * blkCount);
    for (i = 0; i < blkCount; i++)
    {
        prog = &extra->progresses[i];
        Qiniu_Buffer_Write(&body, prog->ctx, strlen(prog->ctx));
        Qiniu_Buffer_PutChar(&body, ',');
    }
    if (blkCount > 0)
    {
        body.curr--;
    }

    Qiniu_Error err = Qiniu_Client_CallWithBuffer(
        c, &root, Qiniu_Buffer_CStr(&url), body.buf, body.curr - body.buf, "text/plain");

    Qiniu_Buffer_Cleanup(&url);
    Qiniu_Buffer_Cleanup(&body);

    if (err.code == 200)
    {
        ret->hash = Qiniu_Json_GetString(root, "hash", NULL);
        ret->key = Qiniu_Json_GetString(root, "key", NULL);
        ret->persistentId = Qiniu_Json_GetString(root, "persistentId", NULL);
    }
    return err;
}

/*============================================================================*/

int Qiniu_Rio_BlockCount(Qiniu_Int64 fsize)
{
    return (int)((fsize + blockMask) >> blockBits);
}

/*============================================================================*/
/* type Qiniu_Rio_Recorder_XXX */

static Qiniu_Error Qiniu_Rio_Recorder_Read_Medium(struct Qiniu_Record_Medium *medium, int *blkIdx, Qiniu_Rio_BlkputRet *ret)
{
#define BUFFER_SIZE (1 << 12)
    size_t haveRead;
    char buf[BUFFER_SIZE];
    Qiniu_Error err = medium->readEntry(medium, buf, BUFFER_SIZE, &haveRead);
    if (err.code != 200)
    {
        medium->close(medium);
        return err;
    }
    if (haveRead >= BUFFER_SIZE)
    {
        medium->close(medium);
        err.code = 500;
        err.message = "recorder entry is too large";
        return err;
    }
    cJSON *blockInfo = cJSON_Parse(buf);
    *blkIdx = Qiniu_Json_GetInt(blockInfo, "blkIdx", -1);
    ret->ctx = Qiniu_String_Dup(Qiniu_Json_GetString(blockInfo, "ctx", NULL));
    ret->checksum = Qiniu_String_Dup(Qiniu_Json_GetString(blockInfo, "checksum", NULL));
    ret->crc32 = Qiniu_Json_GetUInt32(blockInfo, "crc32", 0);
    ret->offset = Qiniu_Json_GetUInt32(blockInfo, "offset", 0);
    ret->expiredAt = Qiniu_Json_GetUInt64(blockInfo, "expiredAt", 0);
    ret->host = Qiniu_String_Dup(Qiniu_Json_GetString(blockInfo, "host", NULL));
    ret->economical = Qiniu_False;
    cJSON_Delete(blockInfo);
    return Qiniu_OK;
#undef BUFFER_SIZE
}

static Qiniu_Error Qiniu_Rio_Recorder_Write_Medium(struct Qiniu_Record_Medium *medium, int blkIdx, Qiniu_Rio_BlkputRet *ret)
{
    Qiniu_Error err;
    cJSON *blockInfo = cJSON_CreateObject();
    cJSON_AddItemToObject(blockInfo, "blkIdx", cJSON_CreateNumber(blkIdx));
    cJSON_AddItemToObject(blockInfo, "ctx", cJSON_CreateString(ret->ctx));
    cJSON_AddItemToObject(blockInfo, "checksum", cJSON_CreateString(ret->checksum));
    cJSON_AddItemToObject(blockInfo, "crc32", cJSON_CreateNumber(ret->crc32));
    cJSON_AddItemToObject(blockInfo, "offset", cJSON_CreateNumber(ret->offset));
    cJSON_AddItemToObject(blockInfo, "expiredAt", cJSON_CreateNumber(ret->expiredAt));
    cJSON_AddItemToObject(blockInfo, "host", cJSON_CreateString(ret->host));
    char *blockInfoJson = cJSON_PrintUnformatted(blockInfo);
    cJSON_Delete(blockInfo);
    if (blockInfoJson == NULL)
    {
        err.code = 400;
        err.message = "cJSON_PrintUnformatted() error";
        return err;
    }
    err = medium->writeEntry(medium, blockInfoJson, NULL);
    free(blockInfoJson);
    if (err.code != 200)
    {
        return err;
    }

    return Qiniu_OK;
}

/*============================================================================*/
/* type Qiniu_Rio_task */

typedef struct _Qiniu_Rio_task
{
    Qiniu_ReaderAt f;
    Qiniu_Client *mc;
    Qiniu_Rio_PutExtra *extra;
    Qiniu_Rio_WaitGroup wg;
    Qiniu_Rio_Recorder *recorder;
    int *nfails;
    Qiniu_Count *ninterrupts;
    int blkIdx;
    int blkSize1;
    int fsize;
    struct _Qiniu_Uploading_Parts_Progress *progress;
} Qiniu_Rio_task;

static void Qiniu_Rio_doTask(void *params)
{
    Qiniu_Rio_BlkputRet ret;
    Qiniu_Rio_task *task = (Qiniu_Rio_task *)params;
    Qiniu_Rio_WaitGroup wg = task->wg;
    Qiniu_Rio_PutExtra *extra = task->extra;
    Qiniu_Rio_Recorder *recorder = task->recorder;
    Qiniu_Rio_ThreadModel tm = extra->threadModel;
    Qiniu_Client *c = tm.itbl->ClientTls(tm.self, task->mc);
    int blkIdx = task->blkIdx;
    int tryTimes = extra->tryTimes;
    size_t chunksUploaded = 0;

    if ((*task->ninterrupts) > 0)
    {
        free(task);
        Qiniu_Count_Inc(task->ninterrupts);
        wg.itbl->Done(wg.self);
        return;
    }

    memset(&ret, 0, sizeof(ret));

lzRetry:
    Qiniu_Rio_BlkputRet_Assign(&ret, &extra->progresses[blkIdx]);
    Qiniu_Error err = Qiniu_Rio_ResumableBlockput(c, &ret, task->f, blkIdx, task->blkSize1, task->fsize, task->progress, extra, &chunksUploaded);
    if (err.code != 200)
    {
        if (err.code == Qiniu_Rio_PutInterrupted)
        {
            // Terminate the upload process if the caller requests
            Qiniu_Rio_BlkputRet_Cleanup(&ret);
            Qiniu_Count_Inc(task->ninterrupts);
            free(task);
            wg.itbl->Done(wg.self);
            return;
        }

        if (tryTimes > 1 && _Qiniu_Should_Retry(err.code) != QINIU_DONT_RETRY)
        {
            tryTimes--;
            Qiniu_Log_Info("resumable.Put %E, retrying ...", err);
            goto lzRetry;
        }
        Qiniu_Log_Warn("resumable.Put %d failed: %E", blkIdx, err);
        extra->notifyErr(extra->notifyRecvr, task->blkIdx, task->blkSize1, err);
        (*task->nfails)++;
    }
    else
    {
        Qiniu_Rio_BlkputRet_Assign(&extra->progresses[blkIdx], &ret);
        if (chunksUploaded > 0 && recorder != NULL && recorder->recorderMedium != NULL)
        {
            Qiniu_Rio_Recorder_Write_Medium(recorder->recorderMedium, blkIdx, &ret);
        }
    }
    Qiniu_Rio_BlkputRet_Cleanup(&ret);
    free(task);
    wg.itbl->Done(wg.self);
}

/*============================================================================*/
/* func Qiniu_Rio_PutXXX */

static Qiniu_Error ErrPutFailed;
static Qiniu_Error ErrPutInterrupted;

static Qiniu_Error Qiniu_Rio_loadProgresses(Qiniu_Rio_PutRet *ret, Qiniu_Rio_PutExtra *extra, Qiniu_Rio_Recorder *recorder)
{
    Qiniu_Bool hasNext;
    int blkIdx;
    Qiniu_Rio_BlkputRet blkPutRet;
    if (recorder != NULL && recorder->recorderMedium != NULL && recorder->toLoadProgresses)
    {
        for (;;)
        {
            Qiniu_Error err = recorder->recorderMedium->hasNextEntry(recorder->recorderMedium, &hasNext);
            if (err.code != 200)
            {
                return err;
            }
            if (hasNext)
            {
                err = Qiniu_Rio_Recorder_Read_Medium(recorder->recorderMedium, &blkIdx, &blkPutRet);
                if (err.code != 200)
                {
                    return err;
                }
                if (blkPutRet.expiredAt != 0 && blkPutRet.expiredAt < (Qiniu_Uint64)time(NULL))
                {
                    continue;
                }
                extra->progresses[blkIdx] = blkPutRet;
            }
            else
            {
                break;
            }
        }
    }
    return Qiniu_OK;
}

static Qiniu_Error Qiniu_Rio_clearProgresses(Qiniu_Rio_PutExtra *extra, Qiniu_Rio_Recorder *recorder)
{
    if (extra->recorder != NULL && recorder->recorderKey != NULL)
    {
        return extra->recorder->remove(extra->recorder, recorder->recorderKey);
    }
    return Qiniu_OK;
}

static Qiniu_Error reinitializeRecorder(Qiniu_Rio_PutExtra *extra, Qiniu_FileInfo *fi, Qiniu_Rio_Recorder *recorder);

static Qiniu_Error _Qiniu_Rio_Put(
    Qiniu_Client *self, Qiniu_Rio_PutRet *ret,
    const char *uptoken, const char *key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_FileInfo *fi, Qiniu_Rio_PutExtra *extra1, Qiniu_Rio_Recorder *recorder)
{
    Qiniu_Int64 offbase;
    Qiniu_Rio_task *task;
    Qiniu_Rio_WaitGroup wg;
    Qiniu_Rio_PutExtra extra;
    Qiniu_Rio_ThreadModel tm;
    Qiniu_Auth auth, auth1 = self->auth;
    const char *accessKey = NULL, *bucketName = NULL;
    int i, last, blkSize;
    int nfails;
    int retCode;
    Qiniu_Count ninterrupts;
    struct _Qiniu_Uploading_Parts_Progress *uploadingPartProgress = NULL;
    Qiniu_Error err = Qiniu_Rio_PutExtra_Init(&extra, fsize, extra1);
    if (err.code != 200)
    {
        return err;
    }
    if (extra.upHost == NULL && (extra.upHosts == NULL || extra.upHostsCount == 0))
    {
        if (!Qiniu_Utils_Extract_Bucket(uptoken, &accessKey, &bucketName))
        {
            err.code = 400;
            err.message = "parse uptoken failed";
            return err;
        }
        err = _Qiniu_Region_Get_Up_Hosts(self, accessKey, bucketName, &extra.upHosts, &extra.upHostsCount);
        if (err.code != 200)
        {
            Qiniu_Free((void *)accessKey);
            Qiniu_Free((void *)bucketName);
            return err;
        }
    }
    if (extra.uploadingProgress != NULL)
    {
        uploadingPartProgress = _Qiniu_Uploading_Parts_Progress_New();
    }

    tm = extra.threadModel;
    wg = tm.itbl->WaitGroup(tm.self);

    last = extra.blockCnt - 1;
    blkSize = 1 << blockBits;
    nfails = 0;
    ninterrupts = 0;

    self->auth = auth = Qiniu_UptokenAuth(uptoken);

    Qiniu_Rio_loadProgresses(ret, &extra, recorder);
reinit:
    for (i = 0; i < (int)extra.blockCnt; i++)
    {
        task = (Qiniu_Rio_task *)malloc(sizeof(Qiniu_Rio_task));
        task->f = f;
        task->extra = &extra;
        task->recorder = recorder;
        task->mc = self;
        task->wg = wg;
        task->nfails = &nfails;
        task->ninterrupts = &ninterrupts;
        task->blkIdx = i;
        task->blkSize1 = blkSize;
        task->fsize = fsize;
        task->progress = uploadingPartProgress;
        if (i == last)
        {
            offbase = (Qiniu_Int64)(i) << blockBits;
            task->blkSize1 = (int)(fsize - offbase);
        }

        wg.itbl->Add(wg.self, 1);
        retCode = tm.itbl->RunTask(tm.self, Qiniu_Rio_doTask, task);
        if (retCode == QINIU_RIO_NOTIFY_EXIT)
        {
            wg.itbl->Done(wg.self);
            Qiniu_Count_Inc(&ninterrupts);
            Qiniu_Free((void *)task);
        }

        if (ninterrupts > 0)
        {
            break;
        }
    } // for

    wg.itbl->Wait(wg.self);
    if (nfails != 0)
    {
        err = ErrPutFailed;
    }
    else if (ninterrupts != 0)
    {
        err = ErrPutInterrupted;
    }
    else
    {
        const char *const *upHosts;
        size_t upHostsCount;

        if (extra.upHost != NULL)
        {
            upHosts = &extra.upHost;
            upHostsCount = 1;
        }
        else
        {
            upHosts = extra.upHosts;
            upHostsCount = extra.upHostsCount;
        }

        for (int tries = 0; tries < extra.tryTimes && tries <= self->hostsRetriesMax; tries++)
        {
            const char *upHost = upHosts[tries % upHostsCount];
            err = Qiniu_Rio_Mkfile(self, ret, upHost, key, fsize, &extra);
            if (err.code == Qiniu_Rio_InvalidCtx && recorder != NULL && recorder->toLoadProgresses == Qiniu_True && fi != NULL)
            {
                Qiniu_Rio_PutExtra_Clear(&extra);
                reinitializeRecorder(&extra, fi, recorder);
                goto reinit;
            }
            else if (err.code == 200 || _Qiniu_Should_Retry(err.code) == QINIU_DONT_RETRY)
            {
                if (err.code / 100 != 4 || err.code == Qiniu_Rio_InvalidCtx)
                {
                    Qiniu_Rio_clearProgresses(&extra, recorder);
                }
                break;
            }
        }
    }

    if (uploadingPartProgress != NULL)
    {
        _Qiniu_Uploading_Parts_Progress_Free(uploadingPartProgress);
    }
    Qiniu_Rio_PutExtra_Cleanup(&extra);
    Qiniu_Free((void *)accessKey);
    Qiniu_Free((void *)bucketName);
    wg.itbl->Release(wg.self);
    auth.itbl->Release(auth.self);
    self->auth = auth1;
    return err;
}

Qiniu_Error Qiniu_Rio_Put(
    Qiniu_Client *self, Qiniu_Rio_PutRet *ret,
    const char *uptoken, const char *key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra *extra1)
{
    return _Qiniu_Rio_Put(self, ret, uptoken, key, f, fsize, NULL, extra1, NULL);
}

static Qiniu_Error initializeRecorder(const char *uptoken, const char *key, const char *localFile, Qiniu_FileInfo *fi,
                                      Qiniu_Rio_PutExtra *extra, Qiniu_Record_Medium *medium, Qiniu_Rio_Recorder *recorder);

Qiniu_Error Qiniu_Rio_PutFile(
    Qiniu_Client *self, Qiniu_Rio_PutRet *ret,
    const char *uptoken, const char *key, const char *localFile, Qiniu_Rio_PutExtra *extra)
{
    Qiniu_Io_PutExtra extra1;
    Qiniu_Int64 fsize;
    Qiniu_FileInfo fi;
    Qiniu_File *f;
    Qiniu_Bool ok;
    Qiniu_Rio_Recorder recorder = {NULL, NULL, Qiniu_False}, *pRecorder = NULL;
    Qiniu_Record_Medium medium;
    Qiniu_Error err = Qiniu_File_Open(&f, localFile);
    if (err.code != 200)
    {
        return err;
    }
    err = Qiniu_File_Stat(f, &fi);
    if (err.code == 200)
    {
        fsize = Qiniu_FileInfo_Fsize(fi);
        if (fsize <= Qiniu_Rio_PutExtra_ChunkSize(extra))
        { // file is too small, don't need resumable-io
            Qiniu_File_Close(f);

            Qiniu_Zero(extra1);
            Qiniu_Io_PutExtra_initFrom(&extra1, extra);

            return Qiniu_Io_PutFile(self, ret, uptoken, key, localFile, &extra1);
        }
        err = initializeRecorder(uptoken, key, localFile, &fi, extra, &medium, &recorder);
        if (err.code != 200)
        {
            return err;
        }
        if (recorder.recorderMedium != NULL)
        {
            pRecorder = &recorder;
        }
        err = _Qiniu_Rio_Put(self, ret, uptoken, key, Qiniu_FileReaderAt(f), fsize, &fi, extra, pRecorder);
        Qiniu_Rio_Recorder_Cleanup(pRecorder);
    }
    Qiniu_File_Close(f);
    return err;
}

Qiniu_Error initializeRecorder(const char *uptoken, const char *key, const char *localFile, Qiniu_FileInfo *fi,
                               Qiniu_Rio_PutExtra *extra, Qiniu_Record_Medium *medium, Qiniu_Rio_Recorder *recorder)
{

    Qiniu_Bool ok;
    Qiniu_Error err = Qiniu_OK;
    if (extra->recorder != NULL)
    {
        err = Qiniu_Utils_Generate_RecorderKey(uptoken, "v1", key, localFile, &recorder->recorderKey);
        if (err.code != 200)
        {
            return err;
        }
        err = Qiniu_Utils_Find_Medium(extra->recorder, recorder->recorderKey, 1, medium, fi, &ok);
        if (err.code != 200)
        {
            return err;
        }
        if (ok)
        {
            recorder->recorderMedium = medium;
            recorder->toLoadProgresses = Qiniu_True;
        }
        else
        {
            err = Qiniu_Utils_New_Medium(extra->recorder, recorder->recorderKey, 1, medium, fi);
            if (err.code != 200)
            {
                return err;
            }
            recorder->recorderMedium = medium;
            recorder->toLoadProgresses = Qiniu_False;
        }
    }
    return err;
}

Qiniu_Error reinitializeRecorder(Qiniu_Rio_PutExtra *extra, Qiniu_FileInfo *fi, Qiniu_Rio_Recorder *recorder)
{
    Qiniu_Error err;
    recorder->recorderMedium->close(recorder->recorderMedium);
    err = Qiniu_Utils_New_Medium(extra->recorder, recorder->recorderKey, 1, recorder->recorderMedium, fi);
    if (err.code != 200)
    {
        return err;
    }
    recorder->toLoadProgresses = Qiniu_False;
    return Qiniu_OK;
}
