all: linux32 linux64

linux32:
	$(CC) -m32 -o keyboard-auto-lock-linux32 keyboard-auto-lock.c

linux64:
	$(CC) -m64 -o keyboard-auto-lock-linux64 keyboard-auto-lock.c

clean:
	rm -f keyboard-auto-lock-linux*
