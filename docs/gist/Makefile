CLIENT_SOURCE_FILES=\
	../../b64/urlsafe_b64.c\
	../../cJSON/cJSON.c\
	../../qiniu/conf.c\
	../../qiniu/base.c\
	../../qiniu/base_io.c\
	../../qiniu/http.c\
	../../qiniu/io.c\
	../../qiniu/resumable_io.c\
	client.c\

SERVER_SOURCE_FILES=\
	../../b64/urlsafe_b64.c\
	../../cJSON/cJSON.c\
	../../qiniu/conf.c\
	../../qiniu/base.c\
	../../qiniu/base_io.c\
	../../qiniu/http.c\
	../../qiniu/auth_mac.c\
	../../qiniu/rs.c\
	../../qiniu/io.c\
	server.c\

all: client server
	@echo "OK"

client: $(CLIENT_SOURCE_FILES)
	gcc $^ -o qiniuclient.exe -lcurl -lm

server: $(SERVER_SOURCE_FILES)
	gcc $^ -o qiniuserver.exe -lcurl -lssl -lcrypto -lm

install: all
	@echo

clean:
	rm -f *.exe

test:
	@echo

