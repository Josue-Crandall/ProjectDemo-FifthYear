include ../macros/MakefileMacros

LIBS=../nacl/nacl.c

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} test.c  -lnacl -o test
	./test
	rm test
