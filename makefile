compileAll: files
	gcc allocation.h allocation.c test.c -o test64 -m64
	gcc allocation.h allocation.c test.c -o test32 -m32
	#rm *.h.gch

compile64: files
	gcc allocation.h allocation.c test.c -o test64 -m64
	#rm *.h.gch

compile32: files
	gcc allocation.h allocation.c test.c -o test32 -m32
	#rm *.h.gch

files: allocation.h allocation.h test.c
