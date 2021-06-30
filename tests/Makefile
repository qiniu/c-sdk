UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
INCLUDE_PATH=-I. -I../b64 -I../cJSON -I../qiniu -I/usr/local/opt/openssl/include
LINK_PATH=-L/usr/local/opt/openssl/lib
else ifeq ($(UNAME),Linux)
INCLUDE_PATH=-I. -I../b64 -I../cJSON -I../qiniu -I/usr/local/ssl/include
LINK_PATH=-L/usr/local/ssl/lib
else
INCLUDE_PATH=-I. -I../b64 -I../cJSON -I../qiniu
endif


SOURCE_FILES=\
	../b64/urlsafe_b64.c\
	../cJSON/cJSON.c\
	../qiniu/conf.c\
	../qiniu/base.c\
	../qiniu/base_io.c\
	../qiniu/http.c\
	../qiniu/auth_mac.c\
	../qiniu/rs.c\
	../qiniu/io.c\
	../qiniu/resumable_io.c\
	../qiniu/multipart_upload.c\
	../qiniu/tm.c\
	../qiniu/fop.c\
	../qiniu/reader.c\
	../qiniu/recorder.c\
	../qiniu/recorder_key.c\
	../qiniu/recorder_utils.c\
	seq.c\
	equal.c\
	test_io_put.c\
	test_resumable_io.c\
	test_base_io.c\
	test_fmt.c\
	test.c\
	test_rs_ops.c\
	test_multipart.c\
	test_recorder.c

#To Gen CUnit-Libs:  cd c-sdk/CUnit;  ./configure; make
CUNIT_LIB=../CUnit/CUnit/Sources/.libs

all: $(SOURCE_FILES)
	gcc -g $^ -o qiniutest -L$(CUNIT_LIB) $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lssl -lcrypto -lcunit -lm -ldl

install: all
	@echo

clean:
	rm -f test

test: all
	LD_LIBRARY_PATH=$(CUNIT_LIB) ./qiniutest

