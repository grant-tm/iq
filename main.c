#define XTDLIB_IMPLEMENTATION
#include "xtdlib.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "renderers/SDL3/clay_renderer_SDL3.c"

#include "layouts.h"

typedef struct MouseState {
	i32 position_x;
	i32 position_y;
	i32 wheel_x;
	i32 wheel_y;
	bool is_down;
} MouseState;

typedef struct ApplicationState {
	bool running;
	SDL_Window *window;
	SDL_GLContext gl_context;
	Clay_SDL3RendererData renderer_data;
	Clay_Arena clay_arena;
	MouseState mouse_state;
} ApplicationState;

void sdl_event_handler (ApplicationState *app, SDL_Event *event) {
	switch (event->type) {
		case SDL_EVENT_QUIT:
		{
			app->running = false;
			break;
		}
		case SDL_EVENT_MOUSE_MOTION:
		{
			app->mouse_state.position_x = event->motion.x;
			app->mouse_state.position_y = event->motion.y;
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
		{
			app->mouse_state.wheel_x = event->wheel.x;
			app->mouse_state.wheel_y = event->wheel.y;
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			app->mouse_state.is_down = true;
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			app->mouse_state.is_down = false;
			break;
		}
	}
}

i32 main(i32 argc, char *argv[]) {
	xtd_ignore_unused(argc);
	xtd_ignore_unused(argv);

	ApplicationState application_state;

	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	SDL_CreateWindowAndRenderer(
        "SDL3 OpenGL Test",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
		&application_state.window,
		&application_state.renderer_data.renderer
    );
    SDL_SetWindowTitle(application_state.window, "IQ");
    application_state.gl_context = SDL_GL_CreateContext(application_state.window);

	// Initialize Clay
	u64 clay_memory_size = Clay_MinMemorySize();
	application_state.clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, malloc(clay_memory_size));
    Clay_Initialize(application_state.clay_arena, (Clay_Dimensions) { 960, 540 }, (Clay_ErrorHandler) { clay_error_handler, 0 });
	
	// Initialize application state
	//application_state.mouse_state = {0};

    application_state.running = true;
    while (application_state.running) {
		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
        	sdl_event_handler(&application_state, &event);    
        }
		clay_frame_update(&application_state); // update layout size & mouse state after handling events
	
		Clay_RenderCommandArray render_commands = main_layout();
    	
		render(&application_state, render_commands);	
	}

    // Cleanup
    SDL_GL_DestroyContext(application_state.gl_context);
    SDL_DestroyWindow(application_state.window);
    SDL_Quit();

    return 0;
}
