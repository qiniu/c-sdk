/*
 ============================================================================
 Name        : region.c
 Author      : Qiniu.com
 Copyright   : 2016(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include <ctype.h>
#include <curl/curl.h>

#include "conf.h"
#include "tm.h"
#include "region.h"

#ifdef __cplusplus
extern "C"
{
#endif

static Qiniu_Uint32 Qiniu_Rgn_enabling = 1;

QINIU_DLLAPI extern void Qiniu_Rgn_Enable(void)
{
	Qiniu_Rgn_enabling = 1;
} // Qiniu_Rgn_Enable

QINIU_DLLAPI extern void Qiniu_Rgn_Disable(void)
{
	Qiniu_Rgn_enabling = 0;
} // Qiniu_Rgn_Disable

QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_IsEnabled(void)
{
	return (Qiniu_Rgn_enabling != 0);
} // Qiniu_Rgn_IsEnabled

static inline int Qiniu_Rgn_Info_isValidHost(const char * str)
{
	return (strstr(str, "http") == str) || (strstr(str, "HTTP") == str);
} // Qiniu_Rgn_Info_isValidHost

static void Qiniu_Rgn_Info_measureHosts(Qiniu_Json * root, Qiniu_Uint32 * bufLen, Qiniu_Uint32 * upHostCount, Qiniu_Uint32 * ioHostCount)
{
	int i = 0;
	Qiniu_Json * arr = NULL;
	const char * str = NULL;

	arr = Qiniu_Json_GetObjectItem(root, "up", NULL);
	if (arr) {
		while ((str = Qiniu_Json_GetStringAt(arr, i++, NULL))) {
			if (!Qiniu_Rgn_Info_isValidHost(str)) {
				// Skip URLs which contains incorrect schema.
				continue;
			} // if
			*bufLen += strlen(str) + 1;
			*upHostCount += 1;
		} // while
	} // if

	i = 0;
	arr = Qiniu_Json_GetObjectItem(root, "io", NULL);
	if (arr) {
		while ((str = Qiniu_Json_GetStringAt(arr, i++, NULL))) {
			if (!Qiniu_Rgn_Info_isValidHost(str)) {
				// Skip URLs which contains incorrect schema.
				continue;
			} // if
			*bufLen += strlen(str) + 1;
			*ioHostCount += 1;
		} // while
	} // if
} // Qiniu_Rgn_Info_measureHosts

static void Qiniu_Rgn_Info_duplicateHosts(Qiniu_Json * root, Qiniu_Rgn_HostInfo *** upHosts, Qiniu_Rgn_HostInfo *** ioHosts, char ** strPos, Qiniu_Uint32 hostFlags)
{
	int i = 0;
	Qiniu_Uint32 len = 0;
	Qiniu_Json * arr = NULL;
	const char * str = NULL;

	arr = Qiniu_Json_GetObjectItem(root, "up", NULL);
	if (arr) {
		while ((str = Qiniu_Json_GetStringAt(arr, i++, NULL))) {
			if (!Qiniu_Rgn_Info_isValidHost(str)) {
				// Skip URLs which contains incorrect schema.
				continue;
			} // if
			(**upHosts)->flags = hostFlags;
			(**upHosts)->host = *strPos;
			len = strlen(str);
			memcpy((void*)(**upHosts)->host, str, len);
			(*strPos) += len + 1;
			*upHosts += 1;
		} // while
	} // if

	i = 0;
	arr = Qiniu_Json_GetObjectItem(root, "io", NULL);
	if (arr) {
		while ((str = Qiniu_Json_GetStringAt(arr, i++, NULL))) {
			if (!Qiniu_Rgn_Info_isValidHost(str)) {
				// Skip URLs which contains incorrect schema.
				continue;
			} // if
			(**ioHosts)->flags = hostFlags | QINIU_RGN_DOWNLOAD_HOST;
			(**ioHosts)->host = *strPos;
			len = strlen(str);
			memcpy((void*)(**ioHosts)->host, str, len);
			(*strPos) += len + 1;
			*ioHosts += 1;
		} // while
	} // if
} // Qiniu_Rgn_Info_duplicateHosts

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Info_Fetch(Qiniu_Client * cli, Qiniu_Rgn_RegionInfo ** rgnInfo, const char * bucket, const char * accessKey)
{
	Qiniu_Error err;
	Qiniu_Json * root = NULL;
	Qiniu_Json * http = NULL;
	Qiniu_Json * https = NULL;
	Qiniu_Uint32 i = 0;
	Qiniu_Uint32 upHostCount = 0;
	Qiniu_Uint32 ioHostCount = 0;
	Qiniu_Rgn_HostInfo ** upHosts = NULL;
	Qiniu_Rgn_HostInfo ** ioHosts = NULL;
	Qiniu_Rgn_RegionInfo * newRgnInfo = NULL;
	char * buf = NULL;
	char * pos = NULL;
	char * url = NULL;
	Qiniu_Uint32 bufLen = 0;

	url = Qiniu_String_Format(256, "%s/v1/query?ak=%s&bucket=%s", QINIU_UC_HOST, accessKey, bucket);
	err = Qiniu_Client_Call(cli, &root, url);
	free(url);
	if (err.code != 200) {
		return err;
	} // if

	bufLen += sizeof(Qiniu_Rgn_RegionInfo) + strlen(bucket) + 1;

	http = Qiniu_Json_GetObjectItem(root, "http", NULL);
	if (http) {
		Qiniu_Rgn_Info_measureHosts(http, &bufLen, &upHostCount, &ioHostCount);
	} // if
	https = Qiniu_Json_GetObjectItem(root, "https", NULL);
	if (https) {
		Qiniu_Rgn_Info_measureHosts(https, &bufLen, &upHostCount, &ioHostCount);
	} // if
	bufLen += (sizeof(Qiniu_Rgn_HostInfo*) + sizeof(Qiniu_Rgn_HostInfo)) * (upHostCount + ioHostCount);

	buf = calloc(1, bufLen);
	if (!buf) {
		err.code = 499;
		err.message = "No enough memory";
		return err;
	} // buf

	pos = buf;

	newRgnInfo = (Qiniu_Rgn_RegionInfo*)pos;
	pos += sizeof(Qiniu_Rgn_RegionInfo);

	newRgnInfo->upHosts = upHosts = (Qiniu_Rgn_HostInfo**)pos;
	pos += sizeof(Qiniu_Rgn_HostInfo*) * upHostCount;

	for (i = 0; i < upHostCount; i += 1) {
		newRgnInfo->upHosts[i] = (Qiniu_Rgn_HostInfo*)(pos);
		pos += sizeof(Qiniu_Rgn_HostInfo);
	} // for

	newRgnInfo->ioHosts = ioHosts = (Qiniu_Rgn_HostInfo**)pos;
	pos += sizeof(Qiniu_Rgn_HostInfo*) * ioHostCount;

	for (i = 0; i < ioHostCount; i += 1) {
		newRgnInfo->ioHosts[i] = (Qiniu_Rgn_HostInfo*)(pos);
		pos += sizeof(Qiniu_Rgn_HostInfo);
	} // for

	if (http) {
		Qiniu_Rgn_Info_duplicateHosts(http, &upHosts, &ioHosts, &pos, QINIU_RGN_HTTP_HOST);
	} // if
	if (https) {
		Qiniu_Rgn_Info_duplicateHosts(https, &upHosts, &ioHosts, &pos, QINIU_RGN_HTTPS_HOST);
	} // if

	newRgnInfo->upHostCount = upHostCount;
	newRgnInfo->ioHostCount = ioHostCount;
	newRgnInfo->global = Qiniu_Json_GetBoolean(root, "global", 0);
	newRgnInfo->ttl = Qiniu_Json_GetInt64(root, "ttl", 86400) ;
	newRgnInfo->nextTimestampToUpdate = newRgnInfo->ttl + Qiniu_Tm_LocalTime();

	newRgnInfo->bucket = pos;
	memcpy((void*)newRgnInfo->bucket, bucket, strlen(bucket));

	*rgnInfo = newRgnInfo;
	return Qiniu_OK;
} // Qiniu_Rgn_Info_Fetch

static Qiniu_Error Qiniu_Rgn_parseQueryArguments(const char * uptoken, char ** bucket, char ** accessKey)
{
	Qiniu_Error err;
	const char * begin = uptoken;
	const char * end = uptoken;
	char * putPolicy = NULL;

	end = strchr(begin, ':');
	if (!end) {
		err.code = 9989;
		err.message = "Invalid uptoken";
		return err;
	} // if

	*accessKey = calloc(1, end - begin + 1);
	if (!*accessKey) {
		err.code = 499;
		err.message = "No enough memory";
		return err;
	} // if
	memcpy(*accessKey, begin, end - begin);

	begin = strchr(end + 1, ':');
	if (!begin) {
		free(*accessKey);
		*accessKey = NULL;
		err.code = 9989;
		err.message = "Invalid uptoken";
		return err;
	} // if

	putPolicy = Qiniu_String_Decode(begin);
	begin = strstr(putPolicy, "\"scope\"");
	if (!begin) {
		free(*accessKey);
		*accessKey = NULL;
		err.code = 9989;
		err.message = "Invalid uptoken";
		return err;
	} // if

	begin += strlen("\"scope\"");
	while (isspace(*begin) || *begin == ':') {
		begin += 1;
	} // while
	if (*begin != '"') {
		free(putPolicy);
		free(*accessKey);
		*accessKey = NULL;

		err.code = 9989;
		err.message = "Invalid uptoken";
		return err;
	} // if

	begin += 1;
	end = begin;
	while (1) {
		if (*end == ':' || (*end == '"' && *(end - 1) != '\\')) {
			break;
		} // if
		end += 1;
	} // while

	*bucket = calloc(1, end - begin + 1);
	if (!*bucket) {
		free(putPolicy);
		free(*accessKey);
		*accessKey = NULL;

		err.code = 499;
		err.message = "No enough memory";
		return err;
	} // if
	memcpy(*bucket, begin, end - begin);
	return Qiniu_OK;
} // Qiniu_Rgn_parseQueryArguments

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Info_FetchByUptoken(Qiniu_Client * cli, Qiniu_Rgn_RegionInfo ** rgnInfo, const char * uptoken)
{
	Qiniu_Error err;
	char * bucket = NULL;
	char * accessKey = NULL;

	err = Qiniu_Rgn_parseQueryArguments(uptoken, &bucket, &accessKey);
	if (err.code != 200) {
		return err;
	} // if

	err = Qiniu_Rgn_Info_Fetch(cli, rgnInfo, bucket, accessKey);
	free(bucket);
	free(accessKey);
	return err;
} // Qiniu_Rgn_Info_FetchByUptoken

QINIU_DLLAPI extern void Qiniu_Rgn_Info_Destroy(Qiniu_Rgn_RegionInfo * rgnInfo)
{
	free(rgnInfo);
} // Qiniu_Rgn_Info_Destroy

QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_Info_HasExpirated(Qiniu_Rgn_RegionInfo * rgnInfo)
{
	return (rgnInfo->nextTimestampToUpdate <= Qiniu_Tm_LocalTime());
} // Qiniu_Rgn_Info_HasExpirated

QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_Info_CountUpHost(Qiniu_Rgn_RegionInfo * rgnInfo)
{
	return rgnInfo->upHostCount;
} // Qiniu_Rgn_Info_CountUpHost

QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_Info_CountIoHost(Qiniu_Rgn_RegionInfo * rgnInfo)
{
	return rgnInfo->ioHostCount;
} // Qiniu_Rgn_Info_CountIoHost

QINIU_DLLAPI extern const char * Qiniu_Rgn_Info_GetHost(Qiniu_Rgn_RegionInfo * rgnInfo, Qiniu_Uint32 n, Qiniu_Uint32 hostFlags)
{
	if ((hostFlags & QINIU_RGN_HTTPS_HOST) == 0) {
		hostFlags |= QINIU_RGN_HTTP_HOST;
	} // if
	if (hostFlags & QINIU_RGN_DOWNLOAD_HOST) {
		if (n < rgnInfo->ioHostCount && (rgnInfo->ioHosts[n]->flags & hostFlags) == hostFlags) {
			return rgnInfo->ioHosts[n]->host;
		} // if
	} else {
		if (n < rgnInfo->upHostCount && (rgnInfo->upHosts[n]->flags & hostFlags) == hostFlags) {
			return rgnInfo->upHosts[n]->host;
		} // if
	} // if
	return NULL;
} // Qiniu_Rgn_Info_GetHost

QINIU_DLLAPI extern Qiniu_Rgn_RegionTable * Qiniu_Rgn_Table_Create(void)
{
	Qiniu_Rgn_RegionTable * new_tbl = NULL;

	new_tbl = calloc(1, sizeof(Qiniu_Rgn_RegionTable));
	if (!new_tbl) {
		return NULL;
	} // if

	return new_tbl;
} // Qiniu_Rgn_Create

QINIU_DLLAPI extern void Qiniu_Rgn_Table_Destroy(Qiniu_Rgn_RegionTable * rgnTable)
{
	Qiniu_Uint32 i = 0;
	if (rgnTable) {
		for (i = 0; i < rgnTable->rgnCount; i += 1) {
			Qiniu_Rgn_Info_Destroy(rgnTable->regions[i]);
		} // for
		free(rgnTable);
	} // if
} // Qiniu_Rgn_Destroy

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_FetchAndUpdate(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Client * cli, const char * bucket, const char * accessKey)
{
	Qiniu_Error err;
	Qiniu_Rgn_RegionInfo * newRgnInfo = NULL;
	
	err = Qiniu_Rgn_Info_Fetch(cli, &newRgnInfo, bucket, accessKey);
	if (err.code != 200) {
		return err;
	} // if

	return Qiniu_Rgn_Table_SetRegionInfo(rgnTable, newRgnInfo);
} // Qiniu_Rgn_Table_FetchAndUpdate

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_FetchAndUpdateByUptoken(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Client * cli, const char * uptoken)
{
	Qiniu_Error err;
	Qiniu_Rgn_RegionInfo * newRgnInfo = NULL;
	
	err = Qiniu_Rgn_Info_FetchByUptoken(cli, &newRgnInfo, uptoken);
	if (err.code != 200) {
		return err;
	} // if

	return Qiniu_Rgn_Table_SetRegionInfo(rgnTable, newRgnInfo);
} // Qiniu_Rgn_Table_FetchAndUpdateByUptoken

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_SetRegionInfo(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Rgn_RegionInfo * rgnInfo)
{
	Qiniu_Error err;
	Qiniu_Uint32 i = 0;
	Qiniu_Rgn_RegionInfo ** newRegions = NULL;

	for (i = 0; i < rgnTable->rgnCount; i += 1) {
		if (strcmp(rgnTable->regions[i]->bucket, rgnInfo->bucket) == 0) {
			Qiniu_Rgn_Info_Destroy(rgnTable->regions[i]);
			rgnTable->regions[i] = rgnInfo;
			return Qiniu_OK;
		} // if
	} // for

	newRegions = calloc(1, sizeof(Qiniu_Rgn_RegionInfo *) * rgnTable->rgnCount + 1);
	if (!newRegions) {
		err.code = 499;
		err.message = "No enough memory";
		return err;
	} // if

	if (rgnTable->rgnCount > 0) {
		memcpy(newRegions, rgnTable->regions, sizeof(Qiniu_Rgn_RegionInfo *) * rgnTable->rgnCount);
	} // if
	newRegions[rgnTable->rgnCount] = rgnInfo;

	free(rgnTable->regions);
	rgnTable->regions = newRegions;
	rgnTable->rgnCount += 1;
	
	return Qiniu_OK;
} // Qiniu_Rgn_Table_SetRegionInfo

QINIU_DLLAPI extern Qiniu_Rgn_RegionInfo * Qiniu_Rgn_Table_GetRegionInfo(Qiniu_Rgn_RegionTable * rgnTable, const char * bucket)
{
	Qiniu_Uint32 i = 0;
	for (i = 0; i < rgnTable->rgnCount; i += 1) {
		if (strcmp(rgnTable->regions[i]->bucket, bucket) == 0) {
			return rgnTable->regions[i];
		} // if
	} // for
	return NULL;
} // Qiniu_Rgn_Table_GetRegionInfo

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_GetHost(
	Qiniu_Rgn_RegionTable * rgnTable,
	Qiniu_Client * cli,
	const char * bucket,
	const char * accessKey,
	Qiniu_Uint32 hostFlags,
	const char ** upHost,
	Qiniu_Rgn_HostVote * vote)
{
	Qiniu_Error err;
	Qiniu_Rgn_RegionInfo * rgnInfo = NULL;
	Qiniu_Rgn_RegionInfo * newRgnInfo = NULL;

	memset(vote, 0, sizeof(Qiniu_Rgn_HostVote));
	rgnInfo = Qiniu_Rgn_Table_GetRegionInfo(rgnTable, bucket);
	if (!rgnInfo || Qiniu_Rgn_Info_HasExpirated(rgnInfo)) {
		err = Qiniu_Rgn_Info_Fetch(cli, &newRgnInfo, bucket, accessKey);
		if (err.code != 200) {
			return err;
		} // if

		err = Qiniu_Rgn_Table_SetRegionInfo(rgnTable, newRgnInfo);
		if (err.code != 200) {
			return err;
		} // if

		rgnInfo = newRgnInfo;
	} // if

	if ((hostFlags & QINIU_RGN_HTTPS_HOST) == 0) {
		hostFlags |= QINIU_RGN_HTTP_HOST;
	} // if
	*upHost = Qiniu_Rgn_Info_GetHost(rgnInfo, 0, hostFlags);

	if (vote) {
		vote->rgnInfo = rgnInfo;
		vote->host = &rgnInfo->upHosts[0];
		vote->hostFlags = hostFlags;
		vote->hosts = rgnInfo->upHosts;
		vote->hostCount = rgnInfo->upHostCount;
	} // if
	return Qiniu_OK;
} // Qiniu_Rgn_Table_GetHost

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_GetHostByUptoken(
	Qiniu_Rgn_RegionTable * rgnTable,
	Qiniu_Client * cli,
	const char * uptoken,
	Qiniu_Uint32 hostFlags,
	const char ** upHost,
	Qiniu_Rgn_HostVote * vote)
{
	Qiniu_Error err;
	char * bucket = NULL;
	char * accessKey = NULL;

	err = Qiniu_Rgn_parseQueryArguments(uptoken, &bucket, &accessKey);
	if (err.code != 200) {
		return err;
	} // if

	err = Qiniu_Rgn_Table_GetHost(rgnTable, cli, bucket, accessKey, hostFlags, upHost, vote);
	free(bucket);
	free(accessKey);
	return Qiniu_OK;
} // Qiniu_Rgn_Table_GetHostByUptoken

static void Qiniu_Rgn_Vote_downgradeHost(Qiniu_Rgn_HostVote * vote)
{
	Qiniu_Rgn_HostInfo ** next = vote->host + 1;
	Qiniu_Rgn_HostInfo * t = NULL;

	while (next < (vote->hosts + vote->hostCount) && ((*next)->flags & vote->hostFlags) == vote->hostFlags) {
		if ((*next)->voteCount >= (*vote->host)->voteCount) {
			t = *next;
			*next = *(vote->host);
			*(vote->host) = t;
		} // if
		vote->host = next;
		next += 1;
	} // while
} // Qiniu_Rgn_Vote_downgradeHost

QINIU_DLLAPI extern void Qiniu_Rgn_Table_VoteHost(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Rgn_HostVote * vote, Qiniu_Error err)
{
	if (!vote->rgnInfo) {
		return;
	} // if
	if (err.code >= 100) {
		(*vote->host)->voteCount += 1;
		return;
	} // if
	switch (err.code) {
		case CURLE_UNSUPPORTED_PROTOCOL:
		case CURLE_COULDNT_RESOLVE_PROXY:
		case CURLE_COULDNT_RESOLVE_HOST:
		case CURLE_COULDNT_CONNECT:
			(*vote->host)->voteCount /= 4;
			Qiniu_Rgn_Vote_downgradeHost(vote);
			break;

		default:
			break;
	} // switch
} // Qiniu_Rgn_VoteHost
