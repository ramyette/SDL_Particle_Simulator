all:
	gcc -I src/include -L src/lib -o ParticleSimulator main.c -lmingw32 -lSDL2main -lSDL2
mac:
	clang main.c -o ParticleSimulator `sdl2-config --cflags --libs`