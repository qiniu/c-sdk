/*
 ============================================================================
 Name        : io.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "io.h"
#include "reader.h"
#include <curl/curl.h>

/*============================================================================*/
/* func Qiniu_Io_form */

typedef struct _Qiniu_Io_form {
    struct curl_httppost *formpost;
    struct curl_httppost *lastptr;
} Qiniu_Io_form;

static Qiniu_Io_PutExtra qiniu_defaultExtra = {NULL, NULL, 0, 0, NULL};

static void Qiniu_Io_form_init(
        Qiniu_Io_form *self, const char *uptoken, const char *key, Qiniu_Io_PutExtra **extra) {
    Qiniu_Io_PutExtraParam *param;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "token", CURLFORM_COPYCONTENTS, uptoken, CURLFORM_END);

    if (*extra == NULL) {
        *extra = &qiniu_defaultExtra;
    }
    if (key != NULL) {
        curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "key", CURLFORM_COPYCONTENTS, key, CURLFORM_END);
    }
    for (param = (*extra)->params; param != NULL; param = param->next) {
        curl_formadd(
                &formpost, &lastptr, CURLFORM_COPYNAME, param->key, CURLFORM_COPYCONTENTS, param->value, CURLFORM_END);
    }

    self->formpost = formpost;
    self->lastptr = lastptr;
}

/*============================================================================*/
/* func Qiniu_Io_PutXXX */

CURL *Qiniu_Client_reset(Qiniu_Client *self);

Qiniu_Error Qiniu_callex(CURL *curl, Qiniu_Buffer *resp, Qiniu_Json **ret, Qiniu_Bool simpleError, Qiniu_Buffer *resph);

static Qiniu_Error Qiniu_Io_call(
        Qiniu_Client *self, Qiniu_Io_PutRet *ret, struct curl_httppost *formpost,
        Qiniu_Io_PutExtra *extra) {
    int retCode = 0;
    Qiniu_Error err;
    struct curl_slist *headers = NULL;
    const char *upHost = NULL;

    CURL *curl = Qiniu_Client_reset(self);

    // Bind the NIC for sending packets.
    if (self->boundNic != NULL) {
        retCode = curl_easy_setopt(curl, CURLOPT_INTERFACE, self->boundNic);
        if (retCode == CURLE_INTERFACE_FAILED) {
            err.code = 9994;
            err.message = "Can not bind the given NIC";
            return err;
        }
    }

    // Specify the low speed limit and time
    if (self->lowSpeedLimit > 0 && self->lowSpeedTime > 0) {
        retCode = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, self->lowSpeedLimit);
        if (retCode == CURLE_INTERFACE_FAILED) {
            err.code = 9994;
            err.message = "Can not specify the low speed limit";
            return err;
        }
        retCode = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, self->lowSpeedTime);
        if (retCode == CURLE_INTERFACE_FAILED) {
            err.code = 9994;
            err.message = "Can not specify the low speed time";
            return err;
        }
    }

    headers = curl_slist_append(NULL, "Expect:");

    //// For using multi-region storage.
    if (extra && (upHost = extra->upHost) == NULL) {
        upHost = QINIU_UP_HOST;
    } // if


    curl_easy_setopt(curl, CURLOPT_URL, upHost);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    //// For aborting uploading file.
    if (extra->upAbortCallback) {
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, Qiniu_Rd_Reader_Callback);
    } // if

    err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
    if (err.code == 200 && ret != NULL) {
        if (extra->callbackRetParser != NULL) {
            err = (*extra->callbackRetParser)(extra->callbackRet, self->root);
        } else {
            ret->hash = Qiniu_Json_GetString(self->root, "hash", NULL);
            ret->key = Qiniu_Json_GetString(self->root, "key", NULL);
            ret->persistentId = Qiniu_Json_GetString(self->root, "persistentId", NULL);
        }
    }

    curl_formfree(formpost);
    curl_slist_free_all(headers);
    return err;
}

Qiniu_Error Qiniu_Io_PutFile(
        Qiniu_Client *self, Qiniu_Io_PutRet *ret,
        const char *uptoken, const char *key, const char *localFile, Qiniu_Io_PutExtra *extra) {
    Qiniu_Error err;
    Qiniu_FileInfo fi;
    Qiniu_Rd_Reader rdr;
    Qiniu_Io_form form;
    size_t fileSize;
    const char *localFileName;
    Qiniu_Io_form_init(&form, uptoken, key, &extra);

    // BugFix : If the filename attribute of the file form-data section is not assigned or holds an empty string,
    //          and the real file size is larger than 10MB, then the Go server will return an error like
    //          "multipart: message too large".
    //          Assign an arbitary non-empty string to this attribute will force the Go server to write all the data
    //          into a temporary file and then every thing goes right.
    localFileName = (extra->localFileName) ? extra->localFileName : "QINIU-C-SDK-UP-FILE";

    //// For aborting uploading file.
    if (extra->upAbortCallback) {
        Qiniu_Zero(rdr);

        rdr.abortCallback = extra->upAbortCallback;
        rdr.abortUserData = extra->upAbortUserData;

        err = Qiniu_Rd_Reader_Open(&rdr, localFile);
        if (err.code != 200) {
            return err;
        } // if


        Qiniu_Zero(fi);
        err = Qiniu_File_Stat(rdr.file, &fi);
        if (err.code != 200) {
            return err;
        } // if

        fileSize = fi.st_size;

        curl_formadd(&form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file", CURLFORM_STREAM, &rdr,
                     CURLFORM_CONTENTSLENGTH, (long) fileSize, CURLFORM_FILENAME, localFileName, CURLFORM_END);
    } else {
        curl_formadd(&form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, localFile,
                     CURLFORM_FILENAME, localFileName, CURLFORM_END);
    } // if

    err = Qiniu_Io_call(self, ret, form.formpost, extra);

    //// For aborting uploading file.
    if (extra->upAbortCallback) {
        Qiniu_Rd_Reader_Close(&rdr);
        if (err.code == CURLE_ABORTED_BY_CALLBACK) {
            if (rdr.status == QINIU_RD_ABORT_BY_CALLBACK) {
                err.code = 9987;
                err.message = "Upload progress has been aborted by caller";
            } else if (rdr.status == QINIU_RD_ABORT_BY_READAT) {
                err.code = 9986;
                err.message = "Upload progress has been aborted by Qiniu_File_ReadAt()";
            } // if
        } // if
    } // if

    return err;
}

Qiniu_Error Qiniu_Io_PutBuffer(
        Qiniu_Client *self, Qiniu_Io_PutRet *ret,
        const char *uptoken, const char *key, const char *buf, size_t fsize, Qiniu_Io_PutExtra *extra) {
    Qiniu_Io_form form;
    Qiniu_Io_form_init(&form, uptoken, key, &extra);

    if (key == NULL) {
        // Use an empty string instead of the NULL pointer to prevent the curl lib from crashing
        // when read it.
        // **NOTICE**: The magic variable $(filename) will be set as empty string.
        key = "";
    }

    curl_formadd(
            &form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file",
            CURLFORM_BUFFER, key, CURLFORM_BUFFERPTR, buf, CURLFORM_BUFFERLENGTH, fsize, CURLFORM_END);

    return Qiniu_Io_call(self, ret, form.formpost, extra);
}

// This function  will be called by 'Qiniu_Io_PutStream'
// In this function, readFunc(read-stream-data) will be set
static Qiniu_Error Qiniu_Io_call_with_callback(
        Qiniu_Client *self, Qiniu_Io_PutRet *ret,
        struct curl_httppost *formpost,
        rdFunc rdr,
        Qiniu_Io_PutExtra *extra) {
    int retCode = 0;
    Qiniu_Error err;
    struct curl_slist *headers = NULL;

    CURL *curl = Qiniu_Client_reset(self);

    // Bind the NIC for sending packets.
    if (self->boundNic != NULL) {
        retCode = curl_easy_setopt(curl, CURLOPT_INTERFACE, self->boundNic);
        if (retCode == CURLE_INTERFACE_FAILED) {
            err.code = 9994;
            err.message = "Can not bind the given NIC";
            return err;
        }
    }

    headers = curl_slist_append(NULL, "Expect:");

    curl_easy_setopt(curl, CURLOPT_URL, QINIU_UP_HOST);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, rdr);

    // Specify the low speed limit and time
    if (self->lowSpeedLimit > 0 && self->lowSpeedTime > 0) {
        retCode = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, self->lowSpeedLimit);
        if (retCode == CURLE_INTERFACE_FAILED) {
            err.code = 9994;
            err.message = "Can not specify the low speed limit";
            return err;
        }
        retCode = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, self->lowSpeedTime);
        if (retCode == CURLE_INTERFACE_FAILED) {
            err.code = 9994;
            err.message = "Can not specify the low speed time";
            return err;
        }
    }

    err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
    if (err.code == 200 && ret != NULL) {
        if (extra->callbackRetParser != NULL) {
            err = (*extra->callbackRetParser)(extra->callbackRet, self->root);
        } else {
            ret->hash = Qiniu_Json_GetString(self->root, "hash", NULL);
            ret->key = Qiniu_Json_GetString(self->root, "key", NULL);
        }
    }

    curl_formfree(formpost);
    curl_slist_free_all(headers);
    return err;
}

Qiniu_Error Qiniu_Io_PutStream(
        Qiniu_Client *self, Qiniu_Io_PutRet *ret,
        const char *uptoken, const char *key,
        void *ctx, size_t fsize, rdFunc rdr,
        Qiniu_Io_PutExtra *extra) {
    Qiniu_Io_form form;
    Qiniu_Io_form_init(&form, uptoken, key, &extra);

    if (key == NULL) {
        // Use an empty string instead of the NULL pointer to prevent the curl lib from crashing
        // when read it.
        // **NOTICE**: The magic variable $(filename) will be set as empty string.
        key = "";
    }

    // Add 'filename' property to make it like a file upload one
    // Otherwise it may report: CURL_ERROR(18) or "multipart/message too large"
    // See https://curl.haxx.se/libcurl/c/curl_formadd.html#CURLFORMSTREAM
    // FIXED by fengyh 2017-03-22 10:30
    curl_formadd(
            &form.formpost, &form.lastptr,
            CURLFORM_COPYNAME, "file",
            CURLFORM_FILENAME, "filename",
            CURLFORM_STREAM, ctx,
            CURLFORM_CONTENTSLENGTH, fsize,
            CURLFORM_END);

    return Qiniu_Io_call_with_callback(self, ret, form.formpost, rdr, extra);
}
