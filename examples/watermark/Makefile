SOURCE_FILES=\
	../../b64/urlsafe_b64.c\
	../../cJSON/cJSON.c\
	../../qbox/conf.c\
	../../qbox/base.c\
	../../qbox/base_io.c\
	../../qbox/oauth2.c\
	../../qbox/oauth2_digest.c\
	../../qbox/rs.c\
	../../qbox/rscli.c\
	../../qbox/auth_policy.c\
	../../qbox/up.c\
	../../qbox/wm.c\
	wmdemo.c\

all: $(SOURCE_FILES)
	gcc -g -o wmdemo -lcurl -lm -lssl -lcrypto $^

install: all
	@echo

clean:
	rm -f wmdemo

