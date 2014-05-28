all:
	cd docs/gist; make

test:
	cd tests; make test

install: all
	cp qiniu/*.h ../c-sdk-for-windows/include/qiniu/

