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
	seq.c\
	equal.c\
	test_io_put.c\
	test_resumable_io.c\
	test_base_io.c\
	test_fmt.c\
	test.c\

all: $(SOURCE_FILES)
	gcc $^ -o test -lcurl -lm -lssl -lcrypto -lcunit -lcurses

install: all
	@echo


test: all
	LD_LIBRARY_PATH=$(CUNIT_LIB) ./qiniutest
	
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
