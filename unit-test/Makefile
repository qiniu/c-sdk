SOURCE_FILES=\
	     ../b64/urlsafe_b64.c\
	     ../cJSON/cJSON.c\
	     ../qbox/conf.c\
	     ../qbox/base.c\
	     ../qbox/oauth2.c\
	     ../qbox/oauth2_digest.c\
	     ../qbox/rs.c\
	     ../qbox/rscli.c\
	     ../qbox/image.c\
	     ../qbox/up.c\
	     ../qbox/*.c\
	     *.c

all: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses

install: all
	@echo

clean:  
	-rm -rf *.o test
	-rm -rf result *~
output: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses -fprofile-arcs -ftest-coverage
	./test
	lcov --directory .   --capture --output-file rs_app.info
	genhtml -o result rs_app.info
	-rm -rf *.gcno *.gcda *.info
	
run: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses -fprofile-arcs -ftest-coverage
	./test
