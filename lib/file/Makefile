include ../macros/MakefileMacros

test:
	$(CC) $(DEBUG_FLAGS) test.c $(LINKS) -otest
	./test
	rm test
	cp backupb64.txt b64.txt
