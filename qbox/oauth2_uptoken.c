/*
 ============================================================================
 Name        : oauth2_uptoken.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "oauth2.h"
#include <curl/curl.h>
#include <openssl/hmac.h>

/*============================================================================*/
static QBox_Error QBox_UpTokenAuth_Auth(void* self, QBox_Header** header,
    const char* url, const char* addition, size_t addlen)
{
	QBox_Error err;

	*header = curl_slist_append(*header, self);

	err.code    = 200;
	err.message = "OK";
	return err;
}

static void QBox_UpTokenAuth_Release(void* self)
{
	free(self);
}

/*============================================================================*/

static QBox_Auth_Vtable QBox_UpTokenAuth_Vtable = {
	QBox_UpTokenAuth_Auth,
	QBox_UpTokenAuth_Release
};

void QBox_Client_InitByUpToken(QBox_Client* self, const char* uptoken, size_t bufSize)
{
	QBox_Error err;
    char* auth = NULL;
    
	/* Set appopriate HTTP header */
	auth = QBox_String_Concat("Authorization: UpToken ", uptoken, NULL);

	QBox_Client_InitEx(self, auth, &QBox_UpTokenAuth_Vtable, bufSize);
}

/*============================================================================*/

char* QBox_Client_MakeUpToken(const char* scope, long expires, const char* callbackURL, const char* returnURL)
{
    time_t sec = 0;
    char expires_str[24];

	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);

    char* policy_str = NULL;
    char* callbackURL_item = "";
    char* returnURL_item = "";
    char* encoded_policy_str = NULL;
    char* encoded_digest = NULL;
    char* uptoken = NULL;

	HMAC_CTX ctx;

	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();

    time(&sec);
    sec += expires;

    bzero(expires_str, sizeof(expires_str));
    QBox_snprintf(expires_str, sizeof(expires_str), "%lu", sec);

    if (callbackURL != NULL) {
        callbackURL_item = QBox_String_Concat("\"callbackUrl\": \"", callbackURL, "\", ", NULL);
    }
    if (returnURL != NULL) {
        returnURL_item = QBox_String_Concat("\"returnUrl\": \"", returnURL, "\", ", NULL);
    }

    policy_str = QBox_String_Concat("{",
        callbackURL_item,
        returnURL_item,
        "\"deadline\": ", expires_str, ", ",
        "\"scope\": \"", scope, "\"",
        "}", NULL);

    if (callbackURL != NULL) {
        free(callbackURL_item);
    }
    if (returnURL != NULL) {
        free(returnURL_item);
    }

    encoded_policy_str = QBox_String_Encode(policy_str);
    free(policy_str);

    bzero(digest, sizeof(digest));
	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, QBOX_SECRET_KEY, strlen(QBOX_SECRET_KEY), EVP_sha1(), NULL);
	HMAC_Update(&ctx, encoded_policy_str, strlen(encoded_policy_str));
	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

    encoded_digest = QBox_Memory_Encode(digest, dgtlen);

    uptoken = QBox_String_Concat(QBOX_ACCESS_KEY, ":", encoded_digest, ":", encoded_policy_str, NULL);
    free(encoded_digest);
    free(encoded_policy_str);

    return uptoken;
}
