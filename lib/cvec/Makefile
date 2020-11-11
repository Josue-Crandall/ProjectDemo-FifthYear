include ../macros/MakefileMacros

LIBS=

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} test.c -lpthread -otest
	./test
	rm test
thread:
	${CC} ${THREAD_FLAGS} ${LIBS} test.c -lpthread -otest
	./test
	rm test
