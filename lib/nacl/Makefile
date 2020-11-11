include ../macros/MakefileMacros

test:
	${CC} ${DEBUG_FLAGS} test.c nacl.c -lnacl -o test
	./test
	rm test
