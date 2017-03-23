#include <openssl/sha.h>

#include "qetag.h"

#define NO 0
#define YES 1

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(1)

#define BLOCK_ELEMENT_MAX_COUNT 16
#define BLOCK_MAX_SIZE (1 << 22)

typedef struct _Qiniu_Qetag_Block {
    struct {
        unsigned int capacity:23;
        unsigned int done:1;
        unsigned int reserved:8;
    };

    SHA_CTX sha1Ctx;
} Qiniu_Qetag_Block;

typedef struct _Qiniu_Qetag_Context
{
    unsigned short int blkElementCount;
    unsigned short int blkUnused;
    unsigned short int blkBegin;
    unsigned short int blkEnd;

    unsigned int blkCount;
    Qiniu_Qetag_Block * blk;

    SHA_CTX sha1Ctx;
    Qiniu_Qetag_Block blkArray[1];
} Qiniu_Qetag_Context;

static Qiniu_Error Qiniu_Qetag_mergeBlocks(struct _Qiniu_Qetag_Context * ctx) {
    Qiniu_Error err;
    unsigned char digest[SHA_DIGEST_LENGTH];
    struct _Qiniu_Qetag_Block * blk = NULL;

    while (ctx->blkUnused < ctx->blkElementCount && ctx->blkArray[ctx->blkBegin].done == YES) {
        blk = &ctx->blkArray[ctx->blkBegin];

        if (SHA1_Final(digest, &blk->sha1Ctx) == 0) {
            err.code = 9999;
            err.message = "openssl internal error";
            return err;
        } // if
        if (SHA1_Update(&ctx->sha1Ctx, digest, sizeof(digest)) == 0) {
            err.code = 9999;
            err.message = "openssl internal error";
            return err;
        } // if

        blk->done = NO;
        
        ctx->blkBegin += 1;
        if (ctx->blkBegin >= ctx->blkElementCount) {
            ctx->blkBegin = 0;
        }
        ctx->blkUnused += 1;
    } // while

    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_mergeBlocks

static Qiniu_Error Qiniu_Qetag_allocateBlock(struct _Qiniu_Qetag_Context * ctx, struct _Qiniu_Qetag_Block ** blk)
{
    Qiniu_Error err;
    struct _Qiniu_Qetag_Block * newBlk = NULL;

    if (ctx->blkUnused == 0) {
        err = Qiniu_Qetag_mergeBlocks(ctx);
        if (err.code != 200) {
            return err;
        }

        if (ctx->blkUnused == 0) {
            *blk = NULL;
            err.code = 9991;
            err.message = "no enough blocks";
            return err;
        } // if
    } // if

    newBlk = &ctx->blkArray[ctx->blkEnd];
    if (SHA1_Init(&newBlk->sha1Ctx) == 0) {
        err.code = 9999;
        err.message = "openssl internal error";
        return err;
    }

    newBlk->done = NO;
    newBlk->capacity = BLOCK_MAX_SIZE;

    ctx->blkUnused -= 1;
    ctx->blkEnd += 1;
    if (ctx->blkEnd >= ctx->blkElementCount) {
        ctx->blkEnd = 0;
    }

    *blk = newBlk;
    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_allocateBlock

Qiniu_Error Qiniu_Qetag_New(struct _Qiniu_Qetag_Context ** ctx, unsigned int concurrency)
{
    Qiniu_Error err;
    Qiniu_Qetag_Context * newCtx = NULL;

    // 分配主结构体
    if (concurrency < 1) {
        concurrency = 1;
    } else if (concurrency > BLOCK_ELEMENT_MAX_COUNT) {
        concurrency = BLOCK_ELEMENT_MAX_COUNT;
    } // if

    newCtx = calloc(1, sizeof(*newCtx) + sizeof(newCtx->blkArray[0]) * (concurrency - 1));
    if (newCtx == NULL) {
        err.code = 9999;
        err.message = "not enough memory";
        return err;
    }
    newCtx->blkElementCount = concurrency;

    err = Qiniu_Qetag_Reset(newCtx);
    if (err.code != 200) {
        Qiniu_Qetag_Destroy(newCtx);
        return err;
    }

    *ctx = newCtx;
    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_New

Qiniu_Error Qiniu_Qetag_Reset(struct _Qiniu_Qetag_Context * ctx)
{
    Qiniu_Error err;

    if (SHA1_Init(&ctx->sha1Ctx) == 0) {
        err.code = 9999;
        err.message = "openssl internal error";
        return err;
    }

    ctx->blkCount   = 0;
    ctx->blkUnused  = ctx->blkElementCount;
    ctx->blkBegin   = 0;
    ctx->blkEnd     = 0;
    ctx->blk        = NULL;

    err = Qiniu_Qetag_allocateBlock(ctx, &ctx->blk);
    if (err.code != 200) {
        return err;
    }

    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_Reset

void Qiniu_Qetag_Destroy(struct _Qiniu_Qetag_Context * ctx)
{
    if (ctx) {
        free(ctx);
    } // if
} // Qiniu_Qetag_Destroy

Qiniu_Error Qiniu_Qetag_Update(struct _Qiniu_Qetag_Context * ctx, const char * buf, size_t bufSize)
{
    Qiniu_Error err;
    const char * pos = buf;
    size_t size = bufSize;
    size_t copySize = 0;

    while (size > 0) {
        if (!ctx->blk) {
            err = Qiniu_Qetag_allocateBlock(ctx, &ctx->blk);
            if (err.code != 200) {
                return err;
            }
        } // if

        copySize = (size >= ctx->blk->capacity) ? ctx->blk->capacity : size;
        if (copySize) {
            err = Qiniu_Qetag_UpdateBlock(ctx->blk, pos, copySize, NULL);
            if (err.code != 200) {
                return err;
            }

            pos += copySize;
            size -= copySize;
        } // if

        if (ctx->blk->capacity == 0) {
            Qiniu_Qetag_CommitBlock(ctx, ctx->blk);
            ctx->blk = NULL;
        } // if
    } // while

    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_Update

Qiniu_Error Qiniu_Qetag_Final(struct _Qiniu_Qetag_Context * ctx, char ** digest)
{
    Qiniu_Error err;
    unsigned char digestSummary[4 + SHA_DIGEST_LENGTH];
    char * newDigest = NULL;

    if (ctx->blk) {
        Qiniu_Qetag_CommitBlock(ctx, ctx->blk);
        ctx->blk = NULL;
    } // if

    if (ctx->blkCount <= 1) {
        digestSummary[0] = 0x16;

        if (SHA1_Final(&digestSummary[1], &ctx->blkArray[0].sha1Ctx) == 0) {
            err.code = 9999;
            err.message = "openssl internal error";
            return err;
        } // if
    } else {
        err = Qiniu_Qetag_mergeBlocks(ctx);
        if (err.code != 200) {
            return err;
        }

        digestSummary[0] = 0x96;
        if (SHA1_Final(&digestSummary[1], &ctx->sha1Ctx) == 0) {
            err.code = 9999;
            err.message = "openssl internal error";
            return err;
        }
    } // if

    newDigest = Qiniu_Memory_Encode((const char *)digestSummary, 1 + SHA_DIGEST_LENGTH);
    if (newDigest == NULL) {
        err.code = 9999;
        err.message = "no enough memory";
        return err;
    }

    *digest = newDigest;
    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_Final

Qiniu_Error Qiniu_Qetag_AllocateBlock(struct _Qiniu_Qetag_Context * ctx, struct _Qiniu_Qetag_Block ** blk, size_t * remainderSize)
{
    Qiniu_Error err;
    
    if (ctx->blk) {
        *blk = ctx->blk;
        ctx->blk = NULL;
    } else {
        err = Qiniu_Qetag_allocateBlock(ctx, blk);
        if (err.code != 200) {
            return err;
        }
    } // if

    if (remainderSize) {
        *remainderSize = (*blk)->capacity;
    }
    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_AllocateBlock

Qiniu_Error Qiniu_Qetag_UpdateBlock(struct _Qiniu_Qetag_Block * blk, const char * buf, size_t bufSize, size_t * remainderSize)
{
    Qiniu_Error err;

    if (bufSize > 0) {
        if (SHA1_Update(&blk->sha1Ctx, buf, bufSize) == 0) {
            err.code = 9999;
            err.message = "openssl internal error";
            return err;
        }
        blk->capacity -= bufSize;
    } // if

    if (remainderSize) {
        *remainderSize = blk->capacity;
    }
    err.code = 200;
    err.message = "ok";
    return err;
} // Qiniu_Qetag_UpdateBlock

void Qiniu_Qetag_CommitBlock(struct _Qiniu_Qetag_Context * ctx, struct _Qiniu_Qetag_Block * blk)
{
    if (blk->capacity < BLOCK_MAX_SIZE) {
        blk->done = YES;
        ctx->blkCount += 1;
    } else {
        blk->done = NO;
    }
} // Qiniu_Qetag_CommitBlock

Qiniu_Error Qiniu_Qetag_DigestFile(const char * localFile, char ** digest)
{
    Qiniu_Error err;
    Qiniu_Off_T offset = 0;
    size_t readingBytes = (BLOCK_MAX_SIZE >> 2);
    ssize_t readBytes = 0;
	Qiniu_File * f = NULL;
    struct _Qiniu_Qetag_Context * ctx = NULL;
    char * buf = NULL;

    // 1MB buffer
    buf = malloc(readingBytes);
    if (!buf) {
        err.code = 9999;
        err.message = "no enough memory";
        return err;
    }

    err = Qiniu_Qetag_New(&ctx, 1);
    if (err.code != 200) {
        goto DIGESTFILE_NEWCTX_ERROR;
    }

	err = Qiniu_File_Open(&f, localFile);
	if (err.code != 200) {
        goto DIGESTFILE_OPEN_ERROR;
	}

    do {
        readBytes = Qiniu_File_ReadAt(f, buf, readingBytes, offset);
        if (readBytes < 0) {
            err.code = 9990;
            err.message = "failed in reading file";
            goto DIGESTFILE_UPDATE_ERROR;
        } else if (readBytes > 0) {
            err = Qiniu_Qetag_Update(ctx, buf, readBytes);
            if (err.code != 200) {
                goto DIGESTFILE_UPDATE_ERROR;
            }
            offset += readBytes;
        } // if
    } while(readBytes > 0);

    err = Qiniu_Qetag_Final(ctx, digest);

DIGESTFILE_UPDATE_ERROR:
    Qiniu_File_Close(f);

DIGESTFILE_OPEN_ERROR:
    Qiniu_Qetag_Destroy(ctx);

DIGESTFILE_NEWCTX_ERROR:
    free(buf);

    return err;
} // Qiniu_Qetag_DigestFile

Qiniu_Error Qiniu_Qetag_DigestBuffer(const char * buf, size_t bufSize, char ** digest)
{
    Qiniu_Error err;
    struct _Qiniu_Qetag_Context * ctx = NULL;

    err = Qiniu_Qetag_New(&ctx, bufSize);
    if (err.code != 200) {
        return err;
    }

    err = Qiniu_Qetag_Update(ctx, buf, bufSize);
    if (err.code != 200) {
        Qiniu_Qetag_Destroy(ctx);
        return err;
    }

    err = Qiniu_Qetag_Final(ctx, digest);
    Qiniu_Qetag_Destroy(ctx);
    return err;
} // Qiniu_Qetag_DigestBuffer

#pragma pack()

#ifdef __cplusplus
}
#endif
