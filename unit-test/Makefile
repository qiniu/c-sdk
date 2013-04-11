SOURCE_FILES=\
	     ../../c-sdk-2.2.0/b64/urlsafe_b64.c\
	     ../../c-sdk-2.2.0/cJSON/cJSON.c\
	     ../../c-sdk-2.2.0/qbox/conf.c\
	     ../../c-sdk-2.2.0/qbox/base.c\
	     ../../c-sdk-2.2.0/qbox/oauth2.c\
	     ../../c-sdk-2.2.0/qbox/oauth2_digest.c\
	     ../../c-sdk-2.2.0/qbox/rs.c\
	     ../../c-sdk-2.2.0/qbox/rscli.c\
	     ../../c-sdk-2.2.0/qbox/image.c\
	     ../../c-sdk-2.2.0/qbox/up.c\
	     ../../c-sdk-2.2.0/qbox/*.c\
	     *.c

all: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses -fprofile-arcs -ftest-coverage

install: all
	@echo

clean:
	-rm -rf *.o test
