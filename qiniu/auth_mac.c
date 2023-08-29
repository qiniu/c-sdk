/*
 ============================================================================
 Name        : mac_auth.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "http.h"
#include <time.h>
#include <ctype.h>
#include <curl/curl.h>
#include "private/crypto.h"

#if defined(_WIN32)
#include "emu_posix.h" // for function Qiniu_Posix_strndup
#pragma comment(lib, "libcrypto.lib")
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define _setenv _putenv_s
static errno_t gmtime_r(const time_t *sourceTime, struct tm *tmDest)
{
	return gmtime_s(tmDest, sourceTime);
}
static errno_t _unsetenv(const char *varname)
{
	return _putenv_s(varname, "");
}
#else
#define Qiniu_Posix_strndup strndup
#define _unsetenv unsetenv
static int _setenv(const char *name, const char *value)
{
	return setenv(name, value, 1);
}
#endif

/*============================================================================*/
/* Global */

void Qiniu_MacAuth_Init()
{
	Qiniu_Crypto_Init();
}

void Qiniu_MacAuth_Cleanup()
{
}

void Qiniu_Servend_Init(long flags)
{
	Qiniu_Global_Init(flags);
	Qiniu_MacAuth_Init();
}

void Qiniu_Servend_Cleanup()
{
	Qiniu_Global_Cleanup();
}

static const char *DISABLE_QINIU_TIMESTAMP_SIGNATURE = "DISABLE_QINIU_TIMESTAMP_SIGNATURE";

void Qiniu_MacAuth_Enable_Qiniu_Timestamp_Signature()
{
	_unsetenv(DISABLE_QINIU_TIMESTAMP_SIGNATURE);
}

void Qiniu_MacAuth_Disable_Qiniu_Timestamp_Signature()
{
	_setenv(DISABLE_QINIU_TIMESTAMP_SIGNATURE, "1");
}

/*============================================================================*/
/* type Qiniu_Mac */

static void
Qiniu_Mac_Hmac_inner(Qiniu_Mac *mac, const char *items[], size_t items_len, const char *addition, size_t addlen, char *digest, unsigned int *digest_len)
{
	Qiniu_HMAC *hmac = Qiniu_HMAC_New(QINIU_DIGEST_TYPE_SHA1, (const unsigned char *)mac->secretKey, (int)strlen(mac->secretKey));
	for (size_t i = 0; i < items_len; i++)
	{
		Qiniu_HMAC_Update(hmac, (const unsigned char *)items[i], (int)strlen(items[i]));
	}
	Qiniu_HMAC_Update(hmac, (const unsigned char *)"\n", 1);
	if (addlen > 0)
	{
		Qiniu_HMAC_Update(hmac, (const unsigned char *)addition, (int)addlen);
	}
	size_t digest_len_tmp;
	Qiniu_HMAC_Final(hmac, (unsigned char *)digest, &digest_len_tmp);
	Qiniu_HMAC_Free(hmac);
	*digest_len = (unsigned int)digest_len_tmp;
}

static void Qiniu_Mac_Hmac(Qiniu_Mac *mac, const char *path, const char *addition, size_t addlen, char *digest, unsigned int *digest_len)
{
	const char *items[] = {path};
	Qiniu_Mac_Hmac_inner(mac, items, sizeof(items) / sizeof(const char *), addition, addlen, digest, digest_len);
}

typedef struct Qiniu_Found_X_Qiniu_Header
{
	char *header_name;
	char *header_value;
	char *free_name_ptr;
	char *free_value_ptr;
} Qiniu_Found_X_Qiniu_Header;

static void Qiniu_Mac_HmacV2(
	Qiniu_Mac *mac, const char *method, const char *host, const char *path, const char *contentType,
	Qiniu_Found_X_Qiniu_Header *x_header, int x_header_count,
	const char *addition, size_t addlen, char *digest, unsigned int *digest_len)
{
	const char *items[] = {method, " ", path, "\nHost: ", host, "\nContent-Type: ", contentType, "\n"};
	if (x_header == NULL)
	{
		size_t count = sizeof(items) / sizeof(const char *);
		Qiniu_Mac_Hmac_inner(mac, items, count, addition, addlen, digest, digest_len);
	}
	else
	{
		size_t new_items_offset = sizeof(items) / sizeof(const char *);
		size_t new_items_count = new_items_offset + x_header_count * 4;
		const char **new_items = (const char **)malloc(new_items_count * sizeof(char *));
		memcpy(new_items, items, sizeof(items));
		for (size_t i = 0; i < x_header_count; i++)
		{
			new_items[new_items_offset++] = x_header[i].header_name;
			new_items[new_items_offset++] = ": ";
			new_items[new_items_offset++] = x_header[i].header_value;
			new_items[new_items_offset++] = "\n";
		}
		Qiniu_Mac_Hmac_inner(mac, new_items, new_items_count, addition, addlen, digest, digest_len);
		Qiniu_Free(new_items);
	}
}

static Qiniu_Error Qiniu_Mac_Parse_Url(const char *url, char const **pHost, size_t *pHostLen, char const **pPath, size_t *pPathLen)
{
	Qiniu_Error err;
	char const *path = strstr(url, "://");
	char const *host = NULL;
	size_t hostLen = 0;

	if (path != NULL)
	{
		host = path + 3;
		path = strchr(path + 3, '/');
	}
	if (path == NULL)
	{
		err.code = 400;
		err.message = "invalid url";
		return err;
	}
	hostLen = path - host;

	if (pHost != NULL)
	{
		*pHost = host;
	}
	if (pHostLen != NULL)
	{
		*pHostLen = hostLen;
	}
	if (pPath != NULL)
	{
		*pPath = path;
	}
	if (pPathLen != NULL)
	{
		*pPathLen = strlen(path);
	}

	return Qiniu_OK;
}

static Qiniu_Error Qiniu_Mac_Auth(
	void *self, Qiniu_Header **header, const char *url, const char *addition, size_t addlen)
{
	Qiniu_Error err;
	char *auth;
	char *enc_digest;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int digest_len = sizeof(digest);
	Qiniu_Mac mac;

	char const *path;
	err = Qiniu_Mac_Parse_Url(url, NULL, NULL, &path, NULL);
	if (err.code != 200)
	{
		return err;
	}

	if (self)
	{
		mac = *(Qiniu_Mac *)self;
	}
	else
	{
		mac.accessKey = QINIU_ACCESS_KEY;
		mac.secretKey = QINIU_SECRET_KEY;
	}
	Qiniu_Mac_Hmac(&mac, path, addition, addlen, digest, &digest_len);
	enc_digest = Qiniu_Memory_Encode(digest, digest_len);

	auth = Qiniu_String_Concat("Authorization: QBox ", mac.accessKey, ":", enc_digest, NULL);
	Qiniu_Free(enc_digest);

	*header = curl_slist_append(*header, auth);
	Qiniu_Free(auth);

	return Qiniu_OK;
}

#define APPLICATION_OCTET_STREAM "application/octet-stream"
#define APPLICATION_WWW_FORM_URLENCODED "application/x-www-form-urlencoded"

static Qiniu_Error Qiniu_For_Each_Header(Qiniu_Header *header, Qiniu_Error (*for_each)(Qiniu_Header *header, void *data), void *data)
{
	for (Qiniu_Header *cur_header = header; cur_header != NULL; cur_header = cur_header->next)
	{
		Qiniu_Error err = for_each(cur_header, data);
		if (err.code != 200)
		{
			return err;
		}
	}
	return Qiniu_OK;
}

static char *trim_string(char *str)
{
	char *end;

	while (isspace((unsigned char)*str))
		str++;

	if (*str == '\0')
	{
		return str;
	}

	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		end--;
	end[1] = '\0';

	return str;
}

typedef struct Qiniu_Find_Content_Type_Callback_Data
{
	char *found_content_type;
	char *free_ptr;
} Qiniu_Find_Content_Type_Callback_Data;

static Qiniu_Error Qiniu_Find_Content_Type_Callback(Qiniu_Header *header, void *data)
{
	Qiniu_Find_Content_Type_Callback_Data *callback_data = (Qiniu_Find_Content_Type_Callback_Data *)data;
	const char *colonPos = strchr(header->data, ':');
	if (colonPos != NULL)
	{
		if (strncasecmp(header->data, "Content-Type", colonPos - header->data) == 0)
		{
			callback_data->free_ptr = strdup(colonPos + 1);
			callback_data->found_content_type = trim_string(callback_data->free_ptr);
			Qiniu_Error found = {201, "Found"};
			return found;
		}
	}

	return Qiniu_OK;
}

static Qiniu_Bool is_x_qiniu_header(Qiniu_Header *header)
{
	const char *x_qiniu_header_prefix = "X-Qiniu-";
	const char *colonPos = strchr(header->data, ':');
	if (colonPos != NULL)
	{
		if (strncasecmp(header->data, x_qiniu_header_prefix, strlen(x_qiniu_header_prefix)) == 0)
		{
			char *first_space_in_header = strchr(header->data, ' ');
			int header_key_length = colonPos - header->data;
			if (first_space_in_header != NULL && first_space_in_header < colonPos)
			{
				header_key_length = first_space_in_header - header->data;
			}
			if (header_key_length > strlen(x_qiniu_header_prefix))
			{
				return Qiniu_True;
			}
		}
	}
	return Qiniu_False;
}

typedef struct Qiniu_Count_X_Qiniu_Callback_Data
{
	size_t count;
} Qiniu_Count_X_Qiniu_Callback_Data;

static Qiniu_Error Qiniu_Count_X_Qiniu_Callback(Qiniu_Header *header, void *data)
{
	if (is_x_qiniu_header(header))
	{
		Qiniu_Count_X_Qiniu_Callback_Data *callback_data = (Qiniu_Count_X_Qiniu_Callback_Data *)data;
		callback_data->count++;
	}
	return Qiniu_OK;
}

#define VALID_TOKEN_TABLE_SIZE (127)
static Qiniu_Bool valid_token_table[VALID_TOKEN_TABLE_SIZE] = {Qiniu_False};

static Qiniu_Bool *initialize_token_table()
{
	static Qiniu_Bool is_initialized = Qiniu_False;
	static char tokens[] = {
		'!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
		'U', 'W', 'V', 'X', 'Y', 'Z', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '|', '~'};
	if (is_initialized == Qiniu_False)
	{
		is_initialized = Qiniu_True;
		for (int i = 0; i < sizeof(tokens) / sizeof(char); i++)
		{
			valid_token_table[i] = Qiniu_True;
		}
	}
	return valid_token_table;
}

static Qiniu_Bool is_valid_header_field_byte(char c, Qiniu_Bool *valid_token_table)
{
	return 0 < c && c < VALID_TOKEN_TABLE_SIZE && valid_token_table[(int)c];
}

static void canonical_mime_header_key(char *name)
{
	static const char TO_LOWER = 'a' - 'A';
	Qiniu_Bool *valid_token_table = initialize_token_table();
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (is_valid_header_field_byte(name[i], valid_token_table) == Qiniu_False)
		{
			return;
		}
	}
	Qiniu_Bool upper = Qiniu_True;
	for (size_t i = 0; i < strlen(name); i++)
	{
		char c = name[i];
		if (upper && 'a' <= c && c <= 'z')
		{
			c -= TO_LOWER;
		}
		else if (!upper && 'A' <= c && c <= 'Z')
		{
			c += TO_LOWER;
		}
		name[i] = c;
		upper = c == '-';
	}
	return;
}

static int compare_x_qiniu_header(const void *left, const void *right)
{
	Qiniu_Found_X_Qiniu_Header *left_header = (Qiniu_Found_X_Qiniu_Header *)left;
	Qiniu_Found_X_Qiniu_Header *right_header = (Qiniu_Found_X_Qiniu_Header *)right;
	return strcmp(left_header->header_name, right_header->header_name);
}

typedef struct Qiniu_Find_X_Qiniu_Callback_Data
{
	Qiniu_Found_X_Qiniu_Header *found_x_qiniu_headers;
	size_t index;
} Qiniu_Find_X_Qiniu_Callback_Data;

static Qiniu_Error Qiniu_Find_X_Qiniu_Callback(Qiniu_Header *header, void *data)
{
	if (is_x_qiniu_header(header))
	{
		const char *colonPos = strchr(header->data, ':');
		if (colonPos != NULL)
		{
			Qiniu_Find_X_Qiniu_Callback_Data *callback_data = (Qiniu_Find_X_Qiniu_Callback_Data *)data;
			Qiniu_Found_X_Qiniu_Header pair = {
				.free_name_ptr = Qiniu_Posix_strndup(header->data, colonPos - header->data),
				.free_value_ptr = strdup(colonPos + 1)};
			pair.header_name = trim_string(pair.free_name_ptr);
			pair.header_value = trim_string(pair.free_value_ptr);
			canonical_mime_header_key(pair.header_name);
			*(callback_data->found_x_qiniu_headers + callback_data->index) = pair;
			callback_data->index += 1;
		}
	}
	return Qiniu_OK;
}

static const char *X_QINIU_DATE = "X-Qiniu-Date: ";

static Qiniu_Error
Qiniu_Mac_AuthV2(
	void *self, const char *method, Qiniu_Header **header, const char *url, const char *addition, size_t addlen)
{
	Qiniu_Error err;
	char *auth;
	char *enc_digest;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int digest_len = sizeof(digest);
	Qiniu_Mac mac;

	char const *host = NULL;
	size_t hostLen = 0;
	char const *path = NULL;
	err = Qiniu_Mac_Parse_Url(url, &host, &hostLen, &path, NULL);
	if (err.code != 200)
	{
		return err;
	}

	char *normalizedHost = (char *)malloc(hostLen + 1);
	memcpy((void *)normalizedHost, host, hostLen);
	*(normalizedHost + hostLen) = '\0';

	if (self)
	{
		mac = *(Qiniu_Mac *)self;
	}
	else
	{
		mac.accessKey = QINIU_ACCESS_KEY;
		mac.secretKey = QINIU_SECRET_KEY;
	}

	Qiniu_Find_Content_Type_Callback_Data find_content_type_callback_data = {NULL, NULL};
	if (Qiniu_For_Each_Header(*header, Qiniu_Find_Content_Type_Callback, &find_content_type_callback_data).code == 201 &&
		strcasecmp(find_content_type_callback_data.found_content_type, APPLICATION_OCTET_STREAM) == 0)
	{
		addlen = 0;
	}
	if (find_content_type_callback_data.found_content_type == NULL)
	{
		*header = curl_slist_append(*header, "Content-Type: " APPLICATION_WWW_FORM_URLENCODED);
		find_content_type_callback_data.found_content_type = APPLICATION_WWW_FORM_URLENCODED;
	}
	if (getenv(DISABLE_QINIU_TIMESTAMP_SIGNATURE) == NULL)
	{
		time_t rawtime;
		struct tm gmtime;
		char time_buf[32] = {0};
		strncpy(time_buf, X_QINIU_DATE, sizeof(time_buf));
		time(&rawtime);
		gmtime_r(&rawtime, &gmtime);
		strftime(time_buf + strlen(X_QINIU_DATE), sizeof(time_buf) - strlen(X_QINIU_DATE), "%Y%m%dT%H%M%SZ", &gmtime);
		*header = curl_slist_append(*header, time_buf);
	}

	Qiniu_Count_X_Qiniu_Callback_Data count_x_qiniu_callback_data = {0};
	Qiniu_For_Each_Header(*header, Qiniu_Count_X_Qiniu_Callback, &count_x_qiniu_callback_data);

	Qiniu_Find_X_Qiniu_Callback_Data find_x_qiniu_callback_data = {NULL, 0};

	if (count_x_qiniu_callback_data.count > 0)
	{
		find_x_qiniu_callback_data.found_x_qiniu_headers = (Qiniu_Found_X_Qiniu_Header *)malloc(count_x_qiniu_callback_data.count * sizeof(Qiniu_Found_X_Qiniu_Header));
		Qiniu_For_Each_Header(*header, Qiniu_Find_X_Qiniu_Callback, &find_x_qiniu_callback_data);
		qsort(find_x_qiniu_callback_data.found_x_qiniu_headers, count_x_qiniu_callback_data.count, sizeof(Qiniu_Found_X_Qiniu_Header), compare_x_qiniu_header);
	}

	Qiniu_Mac_HmacV2(
		&mac, method, normalizedHost, path,
		find_content_type_callback_data.found_content_type,
		find_x_qiniu_callback_data.found_x_qiniu_headers, count_x_qiniu_callback_data.count,
		addition, addlen, digest, &digest_len);

	if (find_content_type_callback_data.free_ptr != NULL)
	{
		Qiniu_Free((void *)find_content_type_callback_data.free_ptr);
	}
	if (find_x_qiniu_callback_data.found_x_qiniu_headers != NULL)
	{
		for (size_t i = 0; i < count_x_qiniu_callback_data.count; i++)
		{
			Qiniu_Free((void *)find_x_qiniu_callback_data.found_x_qiniu_headers[i].free_name_ptr);
			Qiniu_Free((void *)find_x_qiniu_callback_data.found_x_qiniu_headers[i].free_value_ptr);
		}
		Qiniu_Free(find_x_qiniu_callback_data.found_x_qiniu_headers);
	}
	Qiniu_Free(normalizedHost);
	enc_digest = Qiniu_Memory_Encode(digest, digest_len);

	auth = Qiniu_String_Concat("Authorization: Qiniu ", mac.accessKey, ":", enc_digest, NULL);
	Qiniu_Free(enc_digest);

	*header = curl_slist_append(*header, auth);
	Qiniu_Free(auth);

	return Qiniu_OK;
}

static void Qiniu_Mac_Release(void *self)
{
	Qiniu_Free(self);
}

static const char *Qiniu_Mac_Get_AccessKey(void *self)
{
	if (self)
	{
		return ((Qiniu_Mac *)self)->accessKey;
	}
	return QINIU_ACCESS_KEY;
}

static Qiniu_Mac *Qiniu_Mac_Clone(Qiniu_Mac *mac)
{
	Qiniu_Mac *p;
	char *accessKey;
	size_t n1, n2;
	if (mac)
	{
		n1 = strlen(mac->accessKey) + 1;
		n2 = strlen(mac->secretKey) + 1;
		p = (Qiniu_Mac *)malloc(sizeof(Qiniu_Mac) + n1 + n2);
		accessKey = (char *)(p + 1);
		memcpy(accessKey, mac->accessKey, n1);
		memcpy(accessKey + n1, mac->secretKey, n2);
		p->accessKey = accessKey;
		p->secretKey = accessKey + n1;
		return p;
	}
	return NULL;
}

static Qiniu_Auth_Itbl Qiniu_MacAuth_Itbl = {
	Qiniu_Mac_Auth,
	Qiniu_Mac_Release,
	Qiniu_Mac_AuthV2,
	Qiniu_Mac_Get_AccessKey,
};

Qiniu_Auth Qiniu_MacAuth(Qiniu_Mac *mac)
{
	Qiniu_Auth auth = {Qiniu_Mac_Clone(mac), &Qiniu_MacAuth_Itbl};
	return auth;
};

void Qiniu_Client_InitMacAuth(Qiniu_Client *self, size_t bufSize, Qiniu_Mac *mac)
{
	Qiniu_Auth auth = Qiniu_MacAuth(mac);
	Qiniu_Client_InitEx(self, auth, bufSize);
}

/*============================================================================*/
/* func Qiniu_Mac_Sign*/

char *Qiniu_Mac_Sign(Qiniu_Mac *self, char *data)
{
	char *sign;
	char *encoded_digest;
	char digest[EVP_MAX_MD_SIZE + 1];
	size_t digest_len = sizeof(digest);

	Qiniu_Mac mac;

	if (self)
	{
		mac = *self;
	}
	else
	{
		mac.accessKey = QINIU_ACCESS_KEY;
		mac.secretKey = QINIU_SECRET_KEY;
	}

	Qiniu_HMAC *hmac = Qiniu_HMAC_New(QINIU_DIGEST_TYPE_SHA1, (const unsigned char *)mac.secretKey, (int)strlen(mac.secretKey));
	Qiniu_HMAC_Update(hmac, (const unsigned char *)data, (int)strlen(data));
	Qiniu_HMAC_Final(hmac, (unsigned char *)digest, &digest_len);
	Qiniu_HMAC_Free(hmac);

	encoded_digest = Qiniu_Memory_Encode(digest, digest_len);
	sign = Qiniu_String_Concat3(mac.accessKey, ":", encoded_digest);
	Qiniu_Free(encoded_digest);

	return sign;
}

/*============================================================================*/
/* func Qiniu_Mac_SignToken */

char *Qiniu_Mac_SignToken(Qiniu_Mac *self, char *policy_str)
{
	char *data;
	char *sign;
	char *token;

	data = Qiniu_String_Encode(policy_str);
	sign = Qiniu_Mac_Sign(self, data);
	token = Qiniu_String_Concat3(sign, ":", data);

	Qiniu_Free(sign);
	Qiniu_Free(data);

	return token;
}

/*============================================================================*/
