/*
 ============================================================================
 Name        : multipart_upload.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "http.h"
#include "recorder_key.h"
#include "recorder_utils.h"
#include "../cJSON/cJSON.h"
#include <limits.h>
#include <errno.h>
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

Qiniu_Bool Qiniu_Utils_Extract_Bucket(const char *uptoken, const char **pAccessKey, const char **pBucket)
{
	// uptoken=<ak>:<signedData>:<policyDataBase64>
	// extract put policy str from uptoken
	// step1 decode base64
	char *accessKey, *bucket;
	int hitCount = 0;
	int hitIndex = -1;
	int len = strlen(uptoken);

	for (int i = 0; i < len; i++)
	{
		if (uptoken[i] == ':')
		{
			if (hitCount == 0)
			{
				accessKey = Qiniu_String_Dup(uptoken);
				*(accessKey + i) = '\0';
			}
			hitCount++;
			hitIndex = i;
		}
	}
	if ((hitCount != 2) || ((hitIndex + 1) == len))
	{
		Qiniu_Log_Error("invalid uptoken,should contain exactly two colon");
		return Qiniu_False;
	}
	const char *policyB64Data = uptoken + hitIndex + 1;
	char *policyData = Qiniu_String_Decode(policyB64Data);

	// step2 parse json
	cJSON *policy = cJSON_Parse(policyData);
	const char *scope = Qiniu_Json_GetString(policy, "scope", NULL);

	// step3 split bucketname from scope=<bucket>:<xx>
	len = strlen(scope);
	hitIndex = len;
	for (int i = 0; i < len; i++)
	{
		if (scope[i] == ':')
		{
			hitIndex = i;
			break;
		}
	}

	bucket = Qiniu_String_Dup(scope);
	*(bucket + hitIndex) = '\0';
	cJSON_Delete(policy);
	Qiniu_Free(policyData);

	if (pAccessKey != NULL)
	{
		*pAccessKey = accessKey;
	}
	if (pBucket != NULL)
	{
		*pBucket = bucket;
	}
	return Qiniu_True;
}

static char *_realpath(const char *path, char *resolved_path)
{
#ifdef _WIN32
	return _fullpath(resolved_path, path, MAX_PATH);
#else
	return realpath(path, resolved_path);
#endif
}

Qiniu_Error Qiniu_Utils_Generate_RecorderKey(const char *uptoken, const char *version, const char *key, const char *localFile, const char **pRecorderKey)
{
	char fullPath[PATH_MAX];
	const char *bucket, *accessKey;
	Qiniu_Error err;
	if (!Qiniu_Utils_Extract_Bucket(uptoken, &accessKey, &bucket))
	{
		err.code = 400;
		err.message = "parse uptoken failed";
		return err;
	}
	if (_realpath(localFile, fullPath) == NULL)
	{
		err.code = -errno;
		err.message = "realpath() error";
		return err;
	}

	Qiniu_Recorder_Key_Generator recorderKeyGenerator = Qiniu_Recorder_Key_Generator_New();
	Qiniu_Recorder_Key_Generator_Append(&recorderKeyGenerator, version);
	Qiniu_Recorder_Key_Generator_Append(&recorderKeyGenerator, accessKey);
	Qiniu_Recorder_Key_Generator_Append(&recorderKeyGenerator, bucket);
	Qiniu_Recorder_Key_Generator_Append(&recorderKeyGenerator, fullPath);
	if (key != NULL)
	{
		Qiniu_Recorder_Key_Generator_Append(&recorderKeyGenerator, key);
	}

	Qiniu_FreeV2((void **)&bucket);
	Qiniu_FreeV2((void **)&accessKey);

	if (pRecorderKey != NULL)
	{
		*pRecorderKey = Qiniu_Recorder_Key_Generator_Generate(&recorderKeyGenerator);
	}
	Qiniu_Recorder_Key_Generator_Free(recorderKeyGenerator);
	return Qiniu_OK;
}

Qiniu_Error Qiniu_Utils_Find_Medium(Qiniu_Recorder *recorder, const char *recorderKey, Qiniu_Int64 expectedVersion, struct Qiniu_Record_Medium *medium, Qiniu_FileInfo *fileInfo, Qiniu_Bool *ok)
{
#define BUFFER_SIZE (1 << 12)
	size_t haveRead;
	char buf[BUFFER_SIZE];
	Qiniu_Error err = recorder->open(recorder, recorderKey, "rb+", medium);
	if (err.code != 200)
	{
		if (err.code == -ENOENT)
		{
			if (ok != NULL)
			{
				*ok = Qiniu_False;
			}
			return Qiniu_OK;
		}
		return err;
	}
	err = medium->readEntry(medium, buf, BUFFER_SIZE, &haveRead);
	if (err.code != 200)
	{
		medium->close(medium);
		return err;
	}
	if (haveRead >= BUFFER_SIZE)
	{
		medium->close(medium);
		err.code = 500;
		err.message = "recorder metadata is too large";
		return err;
	}
	cJSON *metadata = cJSON_Parse(buf);
	Qiniu_Int64 createTimestamp = Qiniu_Json_GetInt64(metadata, "createTime", -1);
	Qiniu_Int64 fileSize = Qiniu_Json_GetInt64(metadata, "fileSize", -1);
	Qiniu_Int64 modTimestamp = Qiniu_Json_GetInt64(metadata, "modTime", -1);
	Qiniu_Int64 version = Qiniu_Json_GetInt64(metadata, "version", -1);
	cJSON_Delete(metadata);
	Qiniu_Int64 nowTimestamp = (Qiniu_Int64)time(NULL);
	if (version != expectedVersion || nowTimestamp - createTimestamp >= 24 * 3600 * 7 || fileInfo->st_size != fileSize || fileInfo->st_mtime != modTimestamp)
	{
		medium->close(medium);
		if (ok != NULL)
		{
			*ok = Qiniu_False;
		}
		return Qiniu_OK;
	}
	if (ok != NULL)
	{
		*ok = Qiniu_True;
	}

	return Qiniu_OK;
#undef BUFFER_SIZE
}

Qiniu_Error Qiniu_Utils_New_Medium(Qiniu_Recorder *recorder, const char *recorderKey, Qiniu_Int64 version, struct Qiniu_Record_Medium *medium, Qiniu_FileInfo *fileInfo)
{
	Qiniu_Error err = recorder->open(recorder, recorderKey, "wb", medium);
	if (err.code != 200)
	{
		return err;
	}
	cJSON *metadata = cJSON_CreateObject();
	cJSON_AddItemToObject(metadata, "createTime", cJSON_CreateNumber(time(NULL)));
	cJSON_AddItemToObject(metadata, "fileSize", cJSON_CreateNumber(fileInfo->st_size));
	cJSON_AddItemToObject(metadata, "modTime", cJSON_CreateNumber(fileInfo->st_mtime));
	cJSON_AddItemToObject(metadata, "version", cJSON_CreateNumber(version));
	char *metadataJson = cJSON_PrintUnformatted(metadata);
	cJSON_Delete(metadata);
	if (metadataJson == NULL)
	{
		err.code = 400;
		err.message = "cJSON_PrintUnformatted() error";
	}
	err = medium->writeEntry(medium, metadataJson, NULL);
	free(metadataJson);
	if (err.code != 200)
	{
		return err;
	}
	return Qiniu_OK;
}
