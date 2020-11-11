include ../macros/MakefileMacros

BODY=test.c ../nacl/nacl.c

test:
	${CC} ${DEBUG_FLAGS} ${BODY} -lnacl -o test
	./test
	rm test
