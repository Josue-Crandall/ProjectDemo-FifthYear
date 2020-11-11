include ../../lib/macros/MakefileMacros

LIBS=
LINKS=
NAME=test

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}_test.elf
	./${NAME}_test.elf
	rm ${NAME}_test.elf

thread:
	${CC} ${THREAD_FLAGS} ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}_test.elf
	./${NAME}_test.elf
	rm ${NAME}_test.elf

prod:
	${CC} -O3 -static ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}.elf
	strip --strip-all ${NAME}.elf
