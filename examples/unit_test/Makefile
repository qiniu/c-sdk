SOURCE_FILES=\
	     ../../b64/urlsafe_b64.c\
	     ../../cJSON/cJSON.c\
	     ../../qbox/conf.c\
	     ../../qbox/base.c\
	     ../../qbox/oauth2.c\
	     ../../qbox/oauth2_digest.c\
	     ../../qbox/rs.c\
	     ../../qbox/rscli.c\
	     ../../qbox/image.c\
	     *.c

all: $(SOURCE_FILES)
	gcc -o run_test -lcurl -lm -lssl -lcrypto $^

install: all
	@echo

clean:
	rm -f run_test
