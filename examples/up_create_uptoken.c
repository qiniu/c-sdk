#include "../qiniu/rs.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    Qiniu_Mac mac;

    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_RS_PutPolicy putPolicy;
    char *uptoken = NULL;

    //简单上传凭证
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("simple:\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);


    //自定义凭证有效期（示例2小时）
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    putPolicy.expires = 7200; //单位秒
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("deadline:\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);

    //覆盖上传凭证
    Qiniu_Zero(putPolicy);
    char *keyToOverwrite = "qiniu.png";
    putPolicy.scope = Qiniu_String_Concat3(bucket, ":", keyToOverwrite);
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("overwrite:\t%s\n\n", uptoken);
    Qiniu_Free((void *) putPolicy.scope);
    Qiniu_Free(uptoken);

    //自定义上传回复（非callback模式）凭证
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    putPolicy.returnBody = "{\"key\":\"$(key)\",\"hash\":\"$(etag)\",\"fsize\":$(fsize),\"bucket\":\"$(bucket)\",\"name\":\"$(x:name)\"";
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("returnBody:\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);

    //带回调业务服务器的凭证（application/json）
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    putPolicy.callbackUrl = "http://api.example.com/upload/callback";
    putPolicy.callbackBodyType = "application/json";
    putPolicy.callbackBody = "{\"key\":\"$(key)\",\"hash\":\"$(etag)\",\"fsize\":$(fsize),\"bucket\":\"$(bucket)\",\"name\":\"$(x:name)\"";
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("callback(json):\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);

    //带回调业务服务器的凭证（application/x-www-form-urlencoded）
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    putPolicy.callbackUrl = "http://api.example.com/upload/callback";
    putPolicy.callbackBody = "key=$(key)&hash=$(etag)&bucket=$(bucket)&fsize=$(fsize)&name=$(x:name)";
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("callback(url):\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);

    //以scope指定的前缀为文件名的前缀的凭证
    Qiniu_Zero(putPolicy);
    putPolicy.scope = Qiniu_String_Concat2(bucket, ":2017/08/08/images/");
    putPolicy.isPrefixalScope = 1;
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("isPrefixal:\t%s\n\n", uptoken);
    Qiniu_Free((void *) putPolicy.scope);
    Qiniu_Free(uptoken);

    //带数据处理的凭证
    Qiniu_Zero(putPolicy);

    char *saveMp4Entry = Qiniu_String_Concat3(bucket, ":", "avthumb_test_target.mp4");
    char *saveMp4EntryEncoded = Qiniu_String_Encode(saveMp4Entry);

    char *saveJpgEntry = Qiniu_String_Concat3(bucket, ":", "vframe_test_target.jpg");
    char *saveJpgEntryEncoded = Qiniu_String_Encode(saveJpgEntry);

    Qiniu_Free(saveMp4Entry);
    Qiniu_Free(saveJpgEntry);

    char *avthumbMp4Fop = Qiniu_String_Concat2("avthumb/mp4|saveas/", saveMp4EntryEncoded);
    char *vframeJpgFop = Qiniu_String_Concat2("vframe/jpg/offset/1|saveas/", saveJpgEntryEncoded);

    Qiniu_Free(saveMp4EntryEncoded);
    Qiniu_Free(saveJpgEntryEncoded);

    char *fops[] = {avthumbMp4Fop, vframeJpgFop};
    char *persistentFops = Qiniu_String_Join(";", fops, 2);

    putPolicy.scope = bucket;
    putPolicy.persistentPipeline = "sdktest";
    putPolicy.persistentNotifyUrl = "http://api.example.com/pfop/notify";
    putPolicy.persistentOps = persistentFops;

    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("persistent:\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);
    Qiniu_Free(persistentFops);

    //带有效期文件上传
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    putPolicy.deleteAfterDays = 7;
    uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
    printf("deleteAfterDays:\t%s\n\n", uptoken);
    Qiniu_Free(uptoken);

}