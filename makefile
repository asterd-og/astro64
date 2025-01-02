all:
	gcc $(shell find . -name "*.c") -o out/astro64 `pkg-config sdl3 --cflags --libs`
	./out/astro64 prg.bin
