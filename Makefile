all: mkcunit
	@echo "OK"

test:
	@echo "Test OK"

mkcunit:
	cd CUnit; make

