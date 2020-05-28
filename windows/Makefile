all: win32 win64 verify

win64:
	x86_64-w64-mingw32-windres -i Keyboard-Auto-Lock_private.rc --input-format=rc -o Keyboard-Auto-Lock_private.res -O coff 
	x86_64-w64-mingw32-gcc -o Keyboard-Auto-Lock-win64.exe main.c Keyboard-Auto-Lock_private.res -static-libgcc -mwindows

win32:
	i686-w64-mingw32-windres -i Keyboard-Auto-Lock_private.rc --input-format=rc -o Keyboard-Auto-Lock_private.res -O coff 
	i686-w64-mingw32-gcc -o Keyboard-Auto-Lock-win32.exe main.c Keyboard-Auto-Lock_private.res -static-libgcc -mwindows

verify:
	@file *.exe

clean:
	@rm -f *.exe
