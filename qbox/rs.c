/*
 ============================================================================
 Name        : rs.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include <openssl/hmac.h>
#include <curl/curl.h>
#include "rs.h"

struct QBox_Virtual_FuncTable
{
    QBox_Error (*QBox_RS_PutAuth)(QBox_Client*, QBox_RS_PutAuthRet*, char const*, char const*);
    QBox_Error (*QBox_RS_PutAuthEx)(QBox_Client*, QBox_RS_PutAuthRet*, const char*, int, char const*, char const*);
    QBox_Error (*QBox_RS_Put)(QBox_Client*, QBox_RS_PutRet*, const char*, const char*, const char*, FILE*, QBox_Int64, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_Get)(QBox_Client*, QBox_RS_GetRet*, const char*, const char*, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_GetIfNotModified)(QBox_Client*, QBox_RS_GetRet*, const char*, const char*, const char*, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_Stat)(QBox_Client*, QBox_RS_StatRet*, const char*, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_Publish)(QBox_Client*, const char*, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_Unpublish)(QBox_Client*, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_Delete)(QBox_Client*, const char*, const char*, char const*, char const*);
    QBox_Error (*QBox_RS_Drop)(QBox_Client*, const char*, char const*, char const*);
};

/*============================================================================*/
/* func QBox_RS_PutAuth, QBox_RS_PutAuthEx */

QBox_Error QBox_RS_PutAuth(
	QBox_Client* self, QBox_RS_PutAuthRet* ret)
{
	QBox_Error err;
	cJSON* root;
	char* url = QBox_String_Concat2(QBOX_IO_HOST, "/put-auth/");

    if (self->vptr->QBox_RS_PutAuth) {
        err = self->vptr->QBox_RS_PutAuth(self, ret, QBOX_IO_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", NULL);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

QBox_Error QBox_RS_PutAuthEx(
	QBox_Client* self, QBox_RS_PutAuthRet* ret, const char* callbackUrl, int expiresIn)
{
	QBox_Error err;
	cJSON* root;
	char* url;
	char* url2;
	char* callbackEncoded;

	char expires[32];
	QBox_snprintf(expires, 32, "%d", expiresIn);

	url = QBox_String_Concat3(QBOX_IO_HOST, "/put-auth/", expires);

	if (callbackUrl != NULL && *callbackUrl != '\0') {
		callbackEncoded = QBox_String_Encode(callbackUrl);
		url2 = QBox_String_Concat3(url, "/callback/", callbackEncoded);
		free(url);
		free(callbackEncoded);
		url = url2;
	}

    if (self->vptr->QBox_RS_PutAuthEx) {
        err = self->vptr->QBox_RS_PutAuthEx(self, ret, callbackUrl, expiresIn, QBOX_IO_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", NULL);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_RS_Put, QBox_RS_PutFile */

QBox_Error QBox_RS_Put(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, FILE* source, QBox_Int64 fsize, const char* customMeta)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* customMetaEncoded;
	char* mimeEncoded;
	char* url;
	char* url2;

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}

	mimeEncoded = QBox_String_Encode(mimeType);
	url = QBox_String_Concat(QBOX_IO_HOST, "/rs-put/", entryURIEncoded, "/mime/", mimeEncoded, NULL);
	free(mimeEncoded);
	free(entryURIEncoded);

	if (customMeta != NULL && *customMeta != '\0') {
		customMetaEncoded = QBox_String_Encode(customMeta);
		url2 = QBox_String_Concat3(url, "/meta/", customMetaEncoded);
		free(url);
		free(customMetaEncoded);
		url = url2;
	}

    if (self->vptr->QBox_RS_Put) {
        err = self->vptr->QBox_RS_Put(self, ret, tableName, key, mimeType, source, fsize, customMeta, QBOX_IO_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_CallWithBinary(self, &root, url, source, fsize);
	free(url);

	if (err.code == 200) {
		ret->hash = QBox_Json_GetString(root, "hash", NULL);
	}
	return err;
}

QBox_Error QBox_RS_PutFile(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, const char* srcFile, const char* customMeta)
{
	QBox_Error err;
	QBox_Int64 fsize;
	FILE* fp = fopen(srcFile, "rb");
	if (fp == NULL) {
		err.code = -1;
		err.message = "open source file failed";
		return err;
	}
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	err = QBox_RS_Put(self, ret, tableName, key, mimeType, fp, fsize, customMeta);
	fclose(fp);
	return err;
}

/*============================================================================*/
/* func QBox_RS_Get, QBox_RS_GetIfNotModified */

QBox_Error QBox_RS_Get(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/get/", entryURIEncoded);
	char* urlOld;
	char* attNameEncoded;

	free(entryURI);
	free(entryURIEncoded);

	if (attName != NULL) {
		attNameEncoded = QBox_String_Encode(attName);
		urlOld = url;
		url = QBox_String_Concat3(url, "/attName/", attNameEncoded);
		free(attNameEncoded);
		free(urlOld);
	}

    if (self->vptr->QBox_RS_Get) {
        err = self->vptr->QBox_RS_Get(self, ret, tableName, key, attName, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", "unknown");
		ret->hash = QBox_Json_GetString(root, "hash", NULL);
		ret->mimeType = QBox_Json_GetString(root, "mimeType", NULL);
		ret->fsize = QBox_Json_GetInt64(root, "fsize", 0);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

QBox_Error QBox_RS_GetIfNotModified(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName, const char* base)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat(QBOX_RS_HOST, "/get/", entryURIEncoded, "/base/", base, NULL);
	char* urlOld;
	char* attNameEncoded;

	free(entryURI);
	free(entryURIEncoded);

	if (attName != 0) {
		attNameEncoded = QBox_String_Encode(attName);
		urlOld = url;
		url = QBox_String_Concat3(url, "/attName/", attNameEncoded);
		free(attNameEncoded);
		free(urlOld);
	}

    if (self->vptr->QBox_RS_GetIfNotModified) {
        err = self->vptr->QBox_RS_GetIfNotModified(self, ret, tableName, key, attName, base, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", "unknown");
		ret->hash = QBox_Json_GetString(root, "hash", 0);
		ret->mimeType = QBox_Json_GetString(root, "mimeType", 0);
		ret->fsize = QBox_Json_GetInt64(root, "fsize", 0);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_RS_Stat */

QBox_Error QBox_RS_Stat(
	QBox_Client* self, QBox_RS_StatRet* ret, const char* tableName, const char* key)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/stat/", entryURIEncoded);

	free(entryURI);
	free(entryURIEncoded);

    if (self->vptr->QBox_RS_Stat) {
        err = self->vptr->QBox_RS_Stat(self, ret, tableName, key, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->hash = QBox_Json_GetString(root, "hash", 0);
		ret->mimeType = QBox_Json_GetString(root, "mimeType", 0);
		ret->fsize = QBox_Json_GetInt64(root, "fsize", 0);
		ret->putTime = QBox_Json_GetInt64(root, "putTime", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_RS_Publish, QBox_RS_Unpublish */

QBox_Error QBox_RS_Publish(QBox_Client* self, const char* tableName, const char* domain)
{
	QBox_Error err;

	char* domainEncoded = QBox_String_Encode(domain);
	char* url = QBox_String_Concat(QBOX_RS_HOST, "/publish/", domainEncoded, "/from/", tableName, NULL);
	free(domainEncoded);

    if (self->vptr->QBox_RS_Publish) {
        err = self->vptr->QBox_RS_Publish(self, tableName, domain, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

QBox_Error QBox_RS_Unpublish(QBox_Client* self, const char* domain)
{
	QBox_Error err;

	char* domainEncoded = QBox_String_Encode(domain);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/unpublish/", domainEncoded);
	free(domainEncoded);

    if (self->vptr->QBox_RS_Unpublish) {
        err = self->vptr->QBox_RS_Unpublish(self, domain, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/
/* func QBox_RS_Delete */

QBox_Error QBox_RS_Delete(QBox_Client* self, const char* tableName, const char* key)
{
	QBox_Error err;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/delete/", entryURIEncoded);

	free(entryURI);
	free(entryURIEncoded);

    if (self->vptr->QBox_RS_Delete) {
        err = self->vptr->QBox_RS_Delete(self, tableName, key, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/
/* func QBox_RS_Drop */

QBox_Error QBox_RS_Drop(QBox_Client* self, const char* tableName)
{
	QBox_Error err;

	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/drop/", tableName);

    if (self->vptr->QBox_RS_Drop) {
        err = self->vptr->QBox_RS_Drop(self, tableName, QBOX_RS_HOST, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/
static QBox_Error QBox_RS_Generate_AccessKey_Header(
    QBox_Client* self,
    char const* domain, char const* url,
    char const* addition, size_t addlen)
{
    QBox_Error err;
	struct curl_slist* headers = NULL;
    char const* path = NULL;
    char* auth = NULL;
    char digest[EVP_MAX_MD_SIZE + 1];
    unsigned int dgtlen = sizeof(digest);
    char* enc_digest = NULL;
    int ret = 0;
    HMAC_CTX ctx;

    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

	err.code    = 200;
	err.message = "OK";

    path = strstr(url, domain) + strlen(domain);

    /* Do digest calculation */
    HMAC_CTX_init(&ctx);

    ret = HMAC_Init_ex(&ctx, self->scrKey, strlen(self->scrKey), EVP_sha1(), NULL);

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    ret = HMAC_Update(&ctx, path, strlen(path));

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    ret = HMAC_Update(&ctx, "\n", 1);

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    if (addlen > 0) {
        ret = HMAC_Update(&ctx, addition, addlen);

        if (ret == 0) {
            HMAC_CTX_cleanup(&ctx);
            err.code    = 503;
            err.message = "Service Unavailable";
            return err;
        }
    }

    ret = HMAC_Final(&ctx, digest, &dgtlen);

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    HMAC_CTX_cleanup(&ctx);

    digest[dgtlen] = '\0';
    enc_digest = QBox_String_Encode(digest);

    /* Set appopriate HTTP header */
    auth = QBox_String_Concat("Authorization: QBox ", self->acsKey, ":", enc_digest, NULL);
    free(enc_digest);

    if (self->authHeader != NULL) {
	    curl_slist_free_all(self->authHeader);
        self->authHeader = NULL;
    }

	self->authHeader = curl_slist_append(NULL, auth);
    free(auth);

    return err;
}

static QBox_Error QBox_RS_PutAuth_ByAccessKey(
	QBox_Client* self, QBox_RS_PutAuthRet* ret,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_PutAuthEx_ByAccessKey(
	QBox_Client* self, QBox_RS_PutAuthRet* ret, const char* callbackUrl, int expiresIn,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Put_ByAccessKey(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, FILE* source, QBox_Int64 fsize, const char* customMeta,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Get_ByAccessKey(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_GetIfNotModified_ByAccessKey(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName, const char* base,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Stat_ByAccessKey(
	QBox_Client* self, QBox_RS_StatRet* ret, const char* tableName, const char* key,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Publish_ByAccessKey(QBox_Client* self, const char* tableName, const char* domain,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Unpublish_ByAccessKey(QBox_Client* self, const char* domain,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Delete_ByAccessKey(QBox_Client* self, const char* tableName, const char* key,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

static QBox_Error QBox_RS_Drop_ByAccessKey(QBox_Client* self, const char* tableName,
    char const* qiniu_domain, char const* url)
{
    return QBox_RS_Generate_AccessKey_Header(self, qiniu_domain, url, NULL, 0);
}

/*============================================================================*/

/* virtual tables */
static struct QBox_Virtual_FuncTable QBox_ByPassword_Func = {
    NULL, /* QBox_RS_PutAuth */
    NULL, /* QBox_RS_PutAuthEx */
    NULL, /* QBox_RS_Put */
    NULL, /* QBox_RS_Get */
    NULL, /* QBox_RS_GetIfNotModified */
    NULL, /* QBox_RS_Stat */
    NULL, /* QBox_RS_Publish */
    NULL, /* QBox_RS_Unpublish */
    NULL, /* QBox_RS_Delete */
    NULL  /* QBox_RS_Drop */
};

static struct QBox_Virtual_FuncTable QBox_ByAccessKey_Func = {
    &QBox_RS_PutAuth_ByAccessKey,
    &QBox_RS_PutAuthEx_ByAccessKey,
    &QBox_RS_Put_ByAccessKey,
    &QBox_RS_Get_ByAccessKey,
    &QBox_RS_GetIfNotModified_ByAccessKey,
    &QBox_RS_Stat_ByAccessKey,
    &QBox_RS_Publish_ByAccessKey,
    &QBox_RS_Unpublish_ByAccessKey,
    &QBox_RS_Delete_ByAccessKey,
    &QBox_RS_Drop_ByAccessKey
};

void QBox_RS_Init_ByPassword(struct QBox_Virtual_FuncTable** vptr)
{
    *vptr = &QBox_ByPassword_Func;
}

void QBox_RS_Init_ByAccessKey(struct QBox_Virtual_FuncTable** vptr)
{
    *vptr = &QBox_ByAccessKey_Func;
}
