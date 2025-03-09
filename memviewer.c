#include "raylib.h"
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080
#define MEM_SIZE (WIDTH * HEIGHT)

static jmp_buf env;
static volatile sig_atomic_t jmp_set;

void segv_handler(int signal) {
	if (jmp_set == 1) {
		jmp_set = 0;
		longjmp(env, 1);
	}
}

int main() {

	printf("IF YOU SCROLL TOO FAR, YOU WILL GET A SEGFAULT");

	InitWindow(WIDTH, HEIGHT, "Memory Viewer");
	SetTargetFPS(30);

	Color memo[256];
	// cache the rgb values for every hsv input
	// this is maybe a weird way to do it, but the ColorFromHSV function is p expensive, so why not 
	for(int i = 0;i<255;i++){
		memo[i] = ColorFromHSV(i*(360.0f/255.0f),1.0f,(float)i/255.0f);
	}
	// signal handler
	struct sigaction sa;
	sa.sa_handler = segv_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGSEGV, &sa, NULL);

	unsigned char* base_pointer = (unsigned char *)malloc(1);
	unsigned char* mem_ptr = base_pointer - 0x100000; 
	unsigned char* raw_pixel_data = (unsigned char*)malloc(MEM_SIZE*4);
	Image img = GenImageColor(WIDTH, HEIGHT, BLACK);
	Texture2D texture = LoadTextureFromImage(img);
	UnloadImage(img);
	SetTargetFPS(20);
	while (!WindowShouldClose()) {
	    for (int i = 0; i < MEM_SIZE; i++) {
			unsigned char value = 0;
			jmp_set = 1;
			// this does a try-catch for a segfault
			if (setjmp(env) == 0) {
			    value = *(mem_ptr + i);
			} else {
			    value = 0;
			}

			Color col = memo[value];
			raw_pixel_data[i*4 + 0] = col.r;
			raw_pixel_data[i*4 + 1] = col.g;
			raw_pixel_data[i*4 + 2] = col.b;
			raw_pixel_data[i*4 + 3] = 255;
		}

		UpdateTexture(texture, raw_pixel_data);

		BeginDrawing();
		ClearBackground(BLACK);
		DrawTexture(texture, 0, 0, WHITE);
		EndDrawing();

		// Move to a different memory region
		if (IsKeyPressed(KEY_RIGHT)) mem_ptr += MEM_SIZE/2;
		if (IsKeyPressed(KEY_LEFT)) mem_ptr -= MEM_SIZE/2;
	}

	free(raw_pixel_data);
	UnloadTexture(texture);
	free(base_pointer);
	CloseWindow();
	return 0;
}