INCLUDE_PATH=-I. -I../b64 -I../cJSON -I../qiniu -I/usr/local/Cellar/openssl/1.0.2k/include
LINK_PATH=-L/usr/local/Cellar/openssl/1.0.2k/lib

SOURCE_FILES=\
    ../b64/b64.c\
    ../b64/urlsafe_b64.c\
    ../cJSON/cJSON.c\
    ../qiniu/auth_mac.c\
    ../qiniu/conf.c\
    ../qiniu/base.c\
    ../qiniu/base_io.c\
    ../qiniu/cdn.c\
    ../qiniu/fop.c\
    ../qiniu/http.c\
    ../qiniu/io.c\
    ../qiniu/qetag.c\
    ../qiniu/reader.c\
    ../qiniu/resumable_io.c\
    ../qiniu/rs.c\
    ../qiniu/rsf.c\
    ../qiniu/tm.c

rs_stat: $(SOURCE_FILES) ./rs_stat.c
	gcc -g $^ -o exec_rs_stat $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_delete: $(SOURCE_FILES) ./rs_delete.c
	gcc -g $^ -o exec_rs_delete $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_copy: $(SOURCE_FILES) ./rs_copy.c
	gcc -g $^ -o exec_rs_copy $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_move: $(SOURCE_FILES) ./rs_move.c
	gcc -g $^ -o exec_rs_move $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_change_type: $(SOURCE_FILES) ./rs_change_type.c
	gcc -g $^ -o exec_rs_change_type $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_change_mime: $(SOURCE_FILES) ./rs_change_mime.c
	gcc -g $^ -o exec_rs_change_mime $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_delete_after_days: $(SOURCE_FILES) ./rs_delete_after_days.c
	gcc -g $^ -o exec_rs_delete_after_days $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_prefetch: $(SOURCE_FILES) ./rs_prefetch.c
	gcc -g $^ -o exec_rs_prefetch $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_fetch: $(SOURCE_FILES) ./rs_fetch.c
	gcc -g $^ -o exec_rs_fetch $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_batch_stat: $(SOURCE_FILES) ./rs_batch_stat.c
	gcc -g $^ -o exec_rs_batch_stat $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rs_batch_change_type: $(SOURCE_FILES) ./rs_batch_change_type.c
	gcc -g $^ -o exec_rs_batch_change_type $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

fop_video_avthumb: $(SOURCE_FILES) ./fop_video_avthumb.c
	gcc -g $^ -o exec_fop_video_avthumb $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rsf_list_files: $(SOURCE_FILES) ./rsf_list_files.c
	gcc -g $^ -o exec_rsf_list_files $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

rsf_list_bucket: $(SOURCE_FILES) ./rsf_list_bucket.c
	gcc -g $^ -o exec_rsf_list_bucket $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

up_create_uptoken: $(SOURCE_FILES) ./up_create_uptoken.c
	gcc -g $^ -o exec_up_create_uptoken $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

up_form_upload: $(SOURCE_FILES) ./up_form_upload.c
	gcc -g $^ -o exec_up_form_upload $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

up_chunk_upload: $(SOURCE_FILES) ./up_chunk_upload.c
	gcc -g $^ -o exec_up_chunk_upload $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

cdn_refresh_urls: $(SOURCE_FILES) ./cdn_refresh_urls.c
	gcc -g $^ -o exec_cdn_refresh_urls $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

test1: $(SOURCE_FILES) ./test.c
	gcc -g $^ -o exec_test1 $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lcrypto -lssl -lm

clean:
	rm -rf *.dSYM exec*

all: rs_stat rs_delete rs_copy rs_move rs_change_type rs_change_mime rs_delete_after_days rs_prefetch rs_fetch \
     rsf_list_files rsf_list_bucket \
     fop_video_avthumb \
     up_create_uptoken up_form_upload up_chunk_upload
