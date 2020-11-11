include ../macros/MakefileMacros

LIBS=

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} test.c -o test
	#echo now
	./test #> log.txt
	rm test
