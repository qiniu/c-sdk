INCLUDE_PATH=-I. -I../b64 -I../cJSON -I../qiniu -I/usr/local/Cellar/openssl/1.0.2r/include
LINK_PATH=-L/usr/local/Cellar/openssl/1.0.2r/lib

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
	seq.c\
	equal.c\
	test_io_put.c\
	test_resumable_io.c\
	test_base_io.c\
	test_fmt.c\
	test.c\
	test_rs_ops.c\
	test_multipart.c

#To Gen CUnit-Libs:  cd c-sdk/CUnit;  ./configure; make
CUNIT_LIB=../CUnit/CUnit/Sources/.libs

all: $(SOURCE_FILES)
	gcc -g $^ -o qiniutest -L$(CUNIT_LIB) $(INCLUDE_PATH) $(LINK_PATH) -lcurl -lssl -lcrypto -lcunit -lm

install: all
	@echo

clean:
	rm -f test

test: all
	LD_LIBRARY_PATH=$(CUNIT_LIB) ./qiniutest

