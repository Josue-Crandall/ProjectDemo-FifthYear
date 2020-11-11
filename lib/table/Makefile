include ../macros/MakefileMacros

LIBS=

# clang -E grabs expanded macros

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} test.c -o test
	./test
	rm test
