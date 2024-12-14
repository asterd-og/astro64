all:
	gcc $(shell find . -name "*.c") -o out/astro64
	./out/astro64 prg.bin
