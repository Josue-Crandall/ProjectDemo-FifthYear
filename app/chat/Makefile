include ../../lib/macros/MakefileMacros

LIBS=
LINKS=-lpthread
NAME=chat

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}_test.elf
	#./${NAME}_test.elf test.txt
	#rm ${NAME}_test.elf
	cp ${NAME}_test.elf ./c1/${NAME}_test.elf
	cp ${NAME}_test.elf ./c2/${NAME}_test.elf

thread:
	${CC} ${THREAD_FLAGS} ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}_test.elf
	./${NAME}_test.elf test.txt
	rm ${NAME}_test.elf

prod:
	${CC} -O3 -static ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}.elf
	strip --strip-all ${NAME}.elf
