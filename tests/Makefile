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
	seq.c\
	equal.c\
	test_io_put.c\
	test_resumable_io.c\
	test_base_io.c\
	test_fmt.c\
	test.c\
	test_rs_ops.c\

CUNIT_LIB=../CUnit/CUnit/Sources/.libs

all: $(SOURCE_FILES)
	gcc $^ -o qiniutest -L$(CUNIT_LIB) -lcurl -lssl -lcrypto -lcunit -lm

install: all
	@echo

clean:
	rm -f test

test: all
	LD_LIBRARY_PATH=$(CUNIT_LIB) ./qiniutest

