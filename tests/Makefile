SOURCE_FILES=\
	../b64/urlsafe_b64.c\
	../cJSON/cJSON.c\
	../qiniu/conf.c\
	../qiniu/base.c\
	../qiniu/base_io.c\
	../qiniu/oauth2.c\
	../qiniu/oauth2_digest.c\
	../qiniu/rs.c\
	../qiniu/rs_token.c\
	../qiniu/io.c\
	../qiniu/resumable_io.c\
	test.c\

CUNIT_LIB=../CUnit/CUnit/Sources/.libs

all: $(SOURCE_FILES)
	gcc $^ -o qiniutest -L$(CUNIT_LIB) -lcurl -lssl -lcrypto -lcunit -lm

install: all
	@echo

clean:
	rm -f test

test: all
	LD_LIBRARY_PATH=$(CUNIT_LIB) ./qiniutest

