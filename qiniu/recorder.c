/*
 ============================================================================
 Name        : multipart_upload.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "recorder.h"
#include <fcntl.h>
#include <errno.h>
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

static const size_t BUFFER_SIZE = 1 << 22;
static const char SPLIT_CHAR = '\0';
static Qiniu_Error _Qiniu_FileSystem_Recorder_Open(Qiniu_Recorder *recorder, const char *id, const char *mode, Qiniu_Record_Medium *medium);
static Qiniu_Error _Qiniu_FileSystem_Recorder_Remove(Qiniu_Recorder *recorder, const char *id);
static void _Qiniu_FileSystem_Recorder_Free(Qiniu_Recorder *recorder);
static const char *_Qiniu_FileSystem_Recorder_Make_Path(Qiniu_Recorder *recorder, const char *id);
static Qiniu_Error _Qiniu_FileSystem_RecordMedium_Read_Entry(const struct Qiniu_Record_Medium *medium, char *dest, size_t toRead, size_t *haveRead);
static Qiniu_Error _Qiniu_FileSystem_RecordMedium_fulfill(const struct Qiniu_Record_Medium *medium);
static Qiniu_Error _Qiniu_FileSystem_RecordMedium_Has_Next_Entry(const struct Qiniu_Record_Medium *medium, Qiniu_Bool *has);
static Qiniu_Error _Qiniu_FileSystem_RecordMedium_Write_Entry(const struct Qiniu_Record_Medium *medium, const char *src, size_t *written);
static Qiniu_Error _Qiniu_FileSystem_RecordMedium_Close(const struct Qiniu_Record_Medium *medium);

typedef struct _FileSystem_Recorder_Data
{
    FILE *file;
    void *buf;
    size_t bufOffset, bufSize;
} FileSystem_Recorder_Data;

Qiniu_Error
Qiniu_FileSystem_Recorder_New(const char *rootPath, Qiniu_Recorder *recorder)
{
    char *newRootPath = malloc(strlen(rootPath) + 1);
    memset((void *)newRootPath, '\0', strlen(rootPath) + 1);
    strcpy(newRootPath, rootPath);

    recorder->open = _Qiniu_FileSystem_Recorder_Open;
    recorder->remove = _Qiniu_FileSystem_Recorder_Remove;
    recorder->free = _Qiniu_FileSystem_Recorder_Free;
    recorder->data = (void *)newRootPath;
    return Qiniu_OK;
}

Qiniu_Error
_Qiniu_FileSystem_Recorder_Open(Qiniu_Recorder *recorder, const char *id, const char *mode, Qiniu_Record_Medium *medium)
{
    Qiniu_Error err;

    const char *path = _Qiniu_FileSystem_Recorder_Make_Path(recorder, id);
    FILE *file = fopen(path, mode);
    if (file == NULL)
    {
        err.code = -errno;
        err.message = "fopen() error";
        Qiniu_Free((void *)path);
        return err;
    }
    Qiniu_Free((void *)path);

    struct _FileSystem_Recorder_Data *data = malloc(sizeof(struct _FileSystem_Recorder_Data));
    data->file = file;
    data->buf = NULL;
    data->bufOffset = 0;
    medium->readEntry = _Qiniu_FileSystem_RecordMedium_Read_Entry;
    medium->writeEntry = _Qiniu_FileSystem_RecordMedium_Write_Entry;
    medium->hasNextEntry = _Qiniu_FileSystem_RecordMedium_Has_Next_Entry;
    medium->close = _Qiniu_FileSystem_RecordMedium_Close;
    medium->data = (void *)data;
    return Qiniu_OK;
}

Qiniu_Error _Qiniu_FileSystem_Recorder_Remove(Qiniu_Recorder *recorder, const char *id)
{
    const char *path = _Qiniu_FileSystem_Recorder_Make_Path(recorder, id);
    remove(path);
    Qiniu_Free((void *)path);

    return Qiniu_OK;
}

void _Qiniu_FileSystem_Recorder_Free(struct Qiniu_Recorder *recorder)
{
    recorder->open = NULL;
    recorder->remove = NULL;
    recorder->free = NULL;
    Qiniu_FreeV2(&recorder->data);
}

const char *_Qiniu_FileSystem_Recorder_Make_Path(Qiniu_Recorder *recorder, const char *id)
{
#if defined(_WIN32)
    return Qiniu_String_Concat3((const char *)recorder->data, "\\", id);
#else
    return Qiniu_String_Concat3((const char *)recorder->data, "/", id);
#endif
}

Qiniu_Error _Qiniu_FileSystem_RecordMedium_Read_Entry(const struct Qiniu_Record_Medium *medium, char *dest, size_t toRead, size_t *haveRead)
{
    struct _FileSystem_Recorder_Data *data = (struct _FileSystem_Recorder_Data *)medium->data;
    size_t destOffset = 0, haveReadTotal = 0, i;
    for (;;)
    {
        if (data->buf == NULL || data->bufOffset >= data->bufSize)
        {
            Qiniu_Error err = _Qiniu_FileSystem_RecordMedium_fulfill(medium);
            if (err.code != 200)
            {
                return err;
            }
        }
        if (data->buf != NULL)
        {
            size_t bufSize = data->bufSize - data->bufOffset;
            size_t couldRead = bufSize;
            if (couldRead == 0)
            {
                break;
            }
            else if (couldRead > toRead)
            {
                couldRead = toRead;
            }
            for (i = 0; i < couldRead; i++)
            {
                if (((const char *)data->buf)[data->bufOffset] == SPLIT_CHAR)
                {
                    data->bufOffset += 1;
                    goto DONE;
                }
                ((char *)dest)[destOffset] = ((const char *)data->buf)[data->bufOffset];
                toRead -= 1;
                haveReadTotal += 1;
                data->bufOffset += 1;
                destOffset += 1;
            }
        }
    };
DONE:

    if (haveRead != NULL)
    {
        *haveRead = haveReadTotal;
    }

    return Qiniu_OK;
}

Qiniu_Error _Qiniu_FileSystem_RecordMedium_Has_Next_Entry(const struct Qiniu_Record_Medium *medium, Qiniu_Bool *has)
{
    struct _FileSystem_Recorder_Data *data = (struct _FileSystem_Recorder_Data *)medium->data;
    Qiniu_Bool hasEntry = Qiniu_True;

    if (data->buf == NULL || data->bufOffset >= data->bufSize)
    {
        Qiniu_Error err = _Qiniu_FileSystem_RecordMedium_fulfill(medium);
        if (err.code != 200)
        {
            return err;
        }
    }
    if (data->buf != NULL)
    {
        size_t bufSize = data->bufSize - data->bufOffset;
        hasEntry = bufSize > 0;
    }
    if (has != NULL)
    {
        *has = hasEntry;
    }

    return Qiniu_OK;
}

Qiniu_Error _Qiniu_FileSystem_RecordMedium_Write_Entry(const struct Qiniu_Record_Medium *medium, const char *src, size_t *written)
{
    Qiniu_Error err;
    struct _FileSystem_Recorder_Data *data = (struct _FileSystem_Recorder_Data *)medium->data;

    Qiniu_Writer writer = Qiniu_FILE_Writer(data->file);
    size_t haveWritten = writer.Write(src, sizeof(char), strlen(src) + 1, writer.self);
    if (ferror(writer.self))
    {
        err.code = Qiniu_Recorder_Write_Error;
        err.message = "fwrite() error";
        return err;
    }
    if (written != NULL)
    {
        if (haveWritten > strlen(src))
        {
            *written = strlen(src);
        }
        else
        {
            *written = haveWritten;
        }
    }

    if (writer.Flush(writer.self))
    {
        err.code = -errno;
        err.message = "fflush() error";
        return err;
    }

    return Qiniu_OK;
}

Qiniu_Error _Qiniu_FileSystem_RecordMedium_Close(const struct Qiniu_Record_Medium *medium)
{
    struct _FileSystem_Recorder_Data *data = (struct _FileSystem_Recorder_Data *)medium->data;
    fclose(data->file);
    data->file = NULL;
    Qiniu_FreeV2(&data->buf);
    data->bufOffset = 0;
    return Qiniu_OK;
}

Qiniu_Error _Qiniu_FileSystem_RecordMedium_fulfill(const struct Qiniu_Record_Medium *medium)
{
    Qiniu_Error err;
    struct _FileSystem_Recorder_Data *data = (struct _FileSystem_Recorder_Data *)medium->data;

    if (data->buf == NULL)
    {
        data->buf = malloc(BUFFER_SIZE);
        if (data->buf == NULL)
        {
            err.code = -errno;
            err.message = "malloc() error";
            return err;
        }
    }

    Qiniu_Reader reader = Qiniu_FILE_Reader(data->file);
    size_t haveRead = reader.Read(data->buf, sizeof(char), BUFFER_SIZE, reader.self);
    if (ferror(reader.self))
    {
        err.code = Qiniu_Recorder_Read_Error;
        err.message = "fread() error";
        return err;
    }
    data->bufOffset = 0;
    data->bufSize = haveRead;
    return Qiniu_OK;
}

//---------------------------------------
