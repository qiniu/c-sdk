/******************************************************************************* 
 *  @file      demo.c 2013\10\11 10:55:14 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 ******************************************************************************/

#include "io.h"
#include "resumable_io.h"
#include "rs.h"

#include "base.h"

#include <stdio.h>
/******************************************************************************/


/*debug 函数*/
void debuginfo(Qiniu_Client* client, Qiniu_Error err)
{
    printf("\nerror code: %d, message: %s\n", err.code, err.message);
    printf("response header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
    printf("response body:\n%s", Qiniu_Buffer_CStr(&client->b));
    printf("\n\n\n");
}
/*得到上传文件的token*/
char* upLoadToken(const char* bucket, Qiniu_Mac* mac)
{
    Qiniu_RS_PutPolicy putPolicy;
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    return Qiniu_RS_PutPolicy_Token(&putPolicy, mac);
}
/*得到下载文件的url的token*/
char* downloadUrl(const char* domain, const char* key, Qiniu_Mac* mac)
{

    char* url = 0;
    char* baseUrl = 0;

    Qiniu_RS_GetPolicy getPolicy;
    Qiniu_Zero(getPolicy);

    baseUrl = Qiniu_RS_MakeBaseUrl(domain, key);
    url = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, baseUrl, mac);
    Qiniu_Free(baseUrl);
    return url;
}



void demoGetFileStat(Qiniu_Client* pClient, const char* bucketName, const char* keyName)
{

    /* 假设Qiniu帐号存储下有 bucket名称为bucketName所指字符串， 此bucket下有keyName所指字符串的名称文件，
     * 则此如下方法，查询keyName的文件信息
     */
    Qiniu_RS_StatRet statRet;
    Qiniu_Error error = Qiniu_RS_Stat(pClient, &statRet, bucketName, keyName);
    /* 判断http返回值*/
    if (error.code != 200)
    {   /*非200，不正确返回*/
        printf("get file %s:%s stat error.\n", bucketName, keyName);
        debuginfo(pClient, error);
    }else
    {   /*200, 正确返回了, 你可以通过statRet变量查询一些关于这个文件的信息*/
         printf("get file %s:%s stat success.\n", bucketName, keyName);
    }
}

void demoMoveFile(Qiniu_Client* pClient, const char* bucketName, const char* src, const char* dest)
{
    /* 假设Qiniu帐号存储下有 bucket名称为bucketName， 此bucket下有名称为src文件，
     * 则此如下方法，改名子src 为 dest
     */
    Qiniu_Error error = Qiniu_RS_Move(pClient, bucketName, src , bucketName, dest);
    if (error.code != 200)
    {
        printf("rename file from %s:%s to %s:%s error.\n", bucketName, src, bucketName, dest);
        debuginfo(pClient, error);
    }
    else
    {
       printf("rename file from %s:%s to %s:%s success.\n", bucketName, src, bucketName, dest);
    }


    /* 以上改名的逆操作
     */
    error = Qiniu_RS_Move(pClient, bucketName, dest , bucketName, src);
    if (error.code != 200)
    {
        printf("rename file from %s:%s to %s:%s error.\n", bucketName,dest, bucketName, src);
        debuginfo(pClient, error);
    }
    else
    {
       printf("rename file from %s:%s to %s:%s success.\n", bucketName, dest, bucketName, src);
    }
}

void demoCopyFile(Qiniu_Client* pClient, const char* bucketName, const char* src, const char* dest)
{
    /* 假设Qiniu帐号存储下有 bucket名称为bucketName， 此bucket下有src文件，
     * 则此如下方法，拷贝src 为 dest
     */
    Qiniu_Error error = Qiniu_RS_Copy(pClient, bucketName, src, bucketName, dest);
    if (error.code != 200)
    {
        printf("copy file from %s:%s to %s:%s error.\n", bucketName, src, bucketName, dest);
        debuginfo(pClient, error);
    }
    else
    {
        printf("copy file from %s:%s to %s:%s success.\n", bucketName, src, bucketName, dest);
    }
}

void demoDeleteFile(Qiniu_Client* pClient, const char* bucketName, const char* keyName)
{
    /* 假设Qiniu帐号存储下有 bucket名称为bucketName， 此bucket下有 keyName 文件，
     * 则此如下方法, 删除 keyName
     */

    Qiniu_Error error = Qiniu_RS_Delete(pClient, bucketName, keyName);
    if (error.code != 200)
    {
        printf("delete file %s:%s error.\n", bucketName, keyName);
        debuginfo(pClient, error);
    }
    else
    {
        printf("delete file %s:%s success.\n", bucketName, keyName);
    }
}

void demoBatchStatFiles(Qiniu_Client* pClient, const char* bucketName)
{
    /*  假设Qiniu帐号存储下有 bucket名称为bucketName，此bucket 下有批量文件
     *  此demo function演示如何批量得到七牛云存储文件信息
     */
    Qiniu_RS_EntryPath entryPath[] = {
                                        {bucketName, "1.txt"},
                                        {bucketName, "2.txt"},
                                        {bucketName, "3.txt"},
                                        {bucketName, "4.txt"},
                                        {bucketName, "5.txt"},
                                       };
    int len = sizeof(entryPath)/sizeof(Qiniu_RS_EntryPath);
    Qiniu_RS_BatchStatRet* rets = (Qiniu_RS_BatchStatRet*)calloc(len, sizeof(Qiniu_RS_BatchStatRet));
    Qiniu_Error error = Qiniu_RS_BatchStat(pClient, rets, entryPath, len);
    if (200 != error.code)
    {
        printf("get files stat error.\n");
        debuginfo(pClient, error);
    }
    else
    {
        printf("get files stat success.\n");
    }
    free(rets);
}

void demoBatchCopyFiles(Qiniu_Client* pClient, const char* bucketName)
{
    Qiniu_RS_EntryPathPair entryPathpair1[] ={{{bucketName, "1.txt"}, {bucketName, "1_copy.txt"}},
                                              {{bucketName, "2.txt"}, {bucketName, "2_copy.txt"}},
                                              {{bucketName, "3.txt"}, {bucketName, "3_copy.txt"}},
                                              {{bucketName, "4.txt"}, {bucketName, "4_copy.txt"}},
                                              {{bucketName, "5.txt"}, {bucketName, "5_copy.txt"}},
                                             };
    int len = sizeof(entryPathpair1)/sizeof(Qiniu_RS_EntryPathPair);
    Qiniu_RS_BatchItemRet* itemRets = (Qiniu_RS_BatchItemRet*)calloc(len, sizeof(Qiniu_RS_BatchItemRet));
    Qiniu_Error error = Qiniu_RS_BatchCopy(pClient, itemRets, entryPathpair1, len);
    if (200 != error.code)
    {
        printf("copy files error.\n");
        debuginfo(pClient, error);
    }
    else
    {
        printf("copy files success.\n");
    }
    Qiniu_Free(itemRets);
}

void demoBatchDeleteFiles(Qiniu_Client* pClient, const char* bucketName)
{
    Qiniu_RS_EntryPath entryPath1[] = {
                                        {bucketName, "1_copy.txt"},
                                        {bucketName, "2_copy.txt"},
                                        {bucketName, "3_copy.txt"},
                                        {bucketName, "4_copy.txt"},
                                        {bucketName, "5_copy.txt"},
                                      };
    int len = sizeof(entryPath1)/sizeof(Qiniu_RS_EntryPath);
     Qiniu_RS_BatchItemRet* itemRets = (Qiniu_RS_BatchItemRet*)calloc(len, sizeof(Qiniu_RS_BatchItemRet));
    Qiniu_Error error = Qiniu_RS_BatchDelete(pClient, itemRets, entryPath1, len);
    if (200 != error.code)
    {
        printf("delete files error.\n");
        debuginfo(pClient, error);
    }
    else
    {
        printf("delete files success.\n");
    }
    Qiniu_Free(itemRets);
}


void demoUploadFile(Qiniu_Client* pClient, const char* bucketName, Qiniu_Mac* mac)
{
    const char* uploadName = "testUpload1.hpp";
    /*得到uploadKey*/
    const char* uploadtoken = upLoadToken(bucketName, mac);

    const char* pLocalFilePath = "C:\\3rdLib\\operators.hpp";
    
    Qiniu_Io_PutRet putRet;
    Qiniu_Error error = Qiniu_Io_PutFile(pClient, &putRet, uploadtoken, uploadName, pLocalFilePath, NULL);
    if (error.code != 200) 
    {
        printf("Upload File %s To %s:%s error.\n", pLocalFilePath, bucketName,  uploadName);
        debuginfo(pClient, error);
    }
    else
    {
        printf("Upload File %s To %s:%s success.\n", pLocalFilePath, bucketName,  uploadName);
    }

    Qiniu_Free(uploadtoken);
}


void demoGetDownloadURL(const char* bucketName, Qiniu_Mac* mac)
{
    char* domain = Qiniu_String_Concat2(bucketName, ".u.qiniudn.com");
    const char* downloadName = "testUpload1.hpp";
    char* pUrl = downloadUrl(domain, downloadName, mac);

    if (0 == pUrl)
    {
        printf("get URL %s:%s error.\n", bucketName, downloadName);
    }
    else 
    {
        printf("get URL %s:%s is %s.\n", bucketName, downloadName, pUrl);
    }

    Qiniu_Free(pUrl);
    Qiniu_Free(domain);
}

int main()
{

    Qiniu_Client client;
    Qiniu_Mac    mac;
    char* bucketName = "qiniu-demo-test";

    mac.accessKey ="og9UTrj8I83ndelDQrzlpjXS8HwtiQVMV2S_v7D1";
    mac.secretKey = "oGUsG0nvsCcltrlkci08qY48DaM-uC7MQjsWRPe0";
    // 初始化
    Qiniu_Servend_Init(-1);
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    /* 此方法展示如何得到七牛云存储的一个文件的信息*/
    demoGetFileStat(&client, bucketName, "a.txt");
    /* 此方法展示如何更改七牛云存储的一个文件的名称*/
    demoMoveFile(&client, bucketName, "a.txt", "b.txt");
    /* 此方法展示如何复制七牛云存储的一个文件*/
    demoCopyFile(&client, bucketName, "a.txt", "a_back.txt");
    /* 此方法展示如何删除七牛云存储的一个文件*/
    demoDeleteFile(&client, bucketName, "a_back.txt");
    /* 此方法展示如何批量的得到七牛云存储文件的信息*/
    demoBatchStatFiles(&client, bucketName);
    /* 此方法展示如何批量复制七牛云存储文件*/
    demoBatchCopyFiles(&client, bucketName);
    /* 此方法展示如何批量删除七牛云存储文件*/
    demoBatchDeleteFiles(&client, bucketName);

    /* 此方法展示如何上传一个本地文件到服务器*/
    demoUploadFile(&client, bucketName, &mac);
    /*此方法展示如何得到一个服务器上的文件的，下载url*/
    demoGetDownloadURL(bucketName, &mac);

    // 反初始化
    Qiniu_Client_Cleanup(&client);
    Qiniu_Servend_Cleanup();
    return 0;
}

// 
// -----------------------------------------------------------------------------
