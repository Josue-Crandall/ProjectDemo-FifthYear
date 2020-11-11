include ../../lib/macros/MakefileMacros

LIBS="../../lib/nacl/nacl.c"
LINKS=-lpthread -lnacl #-lsodium
NAME=proxy

test:
	${CC} ${DEBUG_FLAGS} ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}_test.elf
	#./${NAME}_test.elf test.txt
	#rm ${NAME}_test.elf
	cp ${NAME}_test.elf ../chat/c1/${NAME}.elf
	cp ${NAME}_test.elf ../chat/c2/${NAME}.elf


thread:
	${CC} ${THREAD_FLAGS} ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}_test.elf
	./${NAME}_test.elf test.txt
	rm ${NAME}_test.elf

prod:
	${CC} -O3 -static ${LIBS} ${NAME}.c ${LINKS} -o ${NAME}.elf
	strip --strip-all ${NAME}.elf
