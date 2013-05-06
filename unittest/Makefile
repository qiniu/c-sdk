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
	
badoutput: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses -fprofile-arcs -ftest-coverage
	./test -b
	lcov --directory .   --capture --output-file rs_app.info
	genhtml -o result rs_app.info
	-rm -rf *.gcno *.gcda *.info
	
run: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses -fprofile-arcs -ftest-coverage
	./test
