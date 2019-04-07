compileAll: files
	gcc allocation.h allocation.c test.c -o test64 -m64 -Wall -W
	gcc allocation.h allocation.c test.c -o test32 -m32 -Wall -W

compile64: files
	gcc allocation.h allocation.c test.c -o test64 -m64 -Wall -W

compile32: files
	gcc allocation.h allocation.c test.c -o test32 -m32 -Wall -W

files: allocation.h allocation.h test.c
