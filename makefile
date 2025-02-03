all:
	gcc $(shell find . -name "*.c") -o out/astro64 -lraylib -lm
	./out/astro64 -m 20M --firmware fba.bin --disk kernel.bin
