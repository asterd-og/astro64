all:
	gcc $(shell find . -name "*.c") -o out/astro64 -lraylib -lm
	./out/astro64 -m 20M --firmware prg.bin --disk disk.img --disk disk2.img
