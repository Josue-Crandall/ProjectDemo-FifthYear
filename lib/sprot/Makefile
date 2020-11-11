include ../macros/MakefileMacros

LIBS=../nacl/nacl.c
LINKS=-lnacl

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} test.c ${LINKS} -o test
	./test
	rm test
