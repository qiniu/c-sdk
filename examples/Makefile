SOURCE_FILES=\
	../b64/urlsafe_b64.c\
	../cJSON/cJSON.c\
	../qbox/conf.c\
	../qbox/base.c\
	../qbox/oauth2.c\
	../qbox/rs.c\
	../qbox/rscli.c\
	rs_demo.c\

ACS_SOURCE_FILES=\
	../b64/urlsafe_b64.c\
	../cJSON/cJSON.c\
	../qbox/conf.c\
	../qbox/base.c\
	../qbox/oauth2.c\
	../qbox/rs.c\
	../qbox/rscli.c\
	rs_accesskey_demo.c\

.PHONY: all rs rs_accesskey

all: rs rs_accesskey

rs: $(SOURCE_FILES)
	gcc -o rs_demo -lcurl -lm $^

rs_accesskey: $(ACS_SOURCE_FILES)
	gcc -o rs_accesskey_demo -lcurl -lm -lssl -lcrypto $^

install: all
	@echo

clean:
	rm -f rs_demo rs_accesskey_demo

