SOURCE_FILES=\
	../../b64/urlsafe_b64.c\
	../../cJSON/cJSON.c\
	../../qbox/conf.c\
	../../qbox/base.c\
	../../qbox/oauth2.c\
	../../qbox/oauth2_digest.c\
	../../qbox/oauth2_uptoken.c\
	../../qbox/rs.c\
	../../qbox/rscli.c\
	rs_demo.c\

all: $(SOURCE_FILES)
	gcc -o rs_demo -lcurl -lm -lz $^

install: all
	@echo

clean:
	rm -f rs_demo
