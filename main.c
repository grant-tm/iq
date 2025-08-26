#define XTDLIB_IMPLEMENTATION
#include "xtdlib/xtdlib.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

i32 main(void) {
    
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	SDL_Window *window = SDL_CreateWindow("SDL3 OpenGL Test", 960, 540, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowTitle(window, "IQ");
	SDL_GLContext glctx = SDL_GL_CreateContext(window);

    bool program_running = true;
    while (program_running) {
        
		SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                program_running = false;
            }
        }

        glClearColor(.1f, .1f, .1f, .1f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
	return 0;
}

