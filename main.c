#define XTDLIB_IMPLEMENTATION
#include "xtdlib.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

i32 main(i32 argc, char *argv[]) {
   	
	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    SDL_Window *window = SDL_CreateWindow(
        "SDL3 OpenGL Test",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    SDL_SetWindowTitle(window, "IQ");
    SDL_GLContext glctx = SDL_GL_CreateContext(window);

	// Primary Loop
    bool program_running = true;
    while (program_running) {
        SDL_Event e;

		// Handle Events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                program_running = false;
            }
        }
    }

    // Cleanup
    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
