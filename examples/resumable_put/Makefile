SOURCE_FILES=\
	../../b64/urlsafe_b64.c\
	../../cJSON/cJSON.c\
	../../qbox/conf.c\
	../../qbox/base.c\
	../../qbox/base_io.c\
	../../qbox/auth_policy.c\
	../../qbox/oauth2.c\
	../../qbox/oauth2_digest.c\
	../../qbox/oauth2_uptoken.c\
	../../qbox/rs.c\
	../../qbox/rs_up.c\
	../../qbox/up.c\
	../../qbox/rscli.c\
	up_demo.c\

all: $(SOURCE_FILES)
	gcc -o up_demo -lcurl -lm -lz $^

install: all
	@echo

clean:
	rm -f up_demo
