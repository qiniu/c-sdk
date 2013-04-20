SOURCE_FILES=\
	../b64/urlsafe_b64.c\
	../cJSON/cJSON.c\
	../qbox/conf.c\
	../qbox/base.c\
	../qbox/base_io.c\
	../qbox/auth_policy.c\
	../qbox/oauth2.c\
	../qbox/oauth2_digest.c\
	../qbox/rs.c\
	../qbox/rscli.c\
	../qbox/io.c\
	test.c\

all: $(SOURCE_FILES)
	ln -s -f ../CUnit/CUnit/Sources/.libs/libcunit.so.1 libcunit.so.1
	gcc -o qiniutest -L../CUnit/CUnit/Sources/.libs -lcurl -lm -lssl -lcrypto -lcunit $^

install: all
	@echo

clean:
	rm -f test

test: all
	./qiniutest

