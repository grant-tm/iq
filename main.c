#define XTDLIB_IMPLEMENTATION
#include "xtdlib.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "renderers/SDL3/clay_renderer_SDL3.c"

void clay_error_handler(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
    //switch(errorData.errorType) {}
}

const Clay_Color COLOR_LIGHT = (Clay_Color) {224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color) {168, 66, 28, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color) {225, 138, 50, 255};

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
		
		i32 screen_width, screen_height;
		SDL_GetWindowSize(application_state.window, &screen_width, &screen_height);
        Clay_SetLayoutDimensions((Clay_Dimensions) { screen_width, screen_height });
        Clay_SetPointerState((Clay_Vector2) { 
				application_state.mouse_state.position_x, 
				application_state.mouse_state.position_y }, 
				application_state.mouse_state.is_down);
		Clay_UpdateScrollContainers(true, (Clay_Vector2) { 
				application_state.mouse_state.wheel_x, application_state.mouse_state.wheel_y }, 0.1);
        
		Clay_BeginLayout(); 
        CLAY({ 	
			.id = CLAY_ID("OuterContainer"), 
			.layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, 
			.padding = CLAY_PADDING_ALL(16), 
			.childGap = 16 }, 
			.backgroundColor = {250,250,255,255} }) {
            CLAY({
                .id = CLAY_ID("SideBar"),
                .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, 
				.sizing = { .width = CLAY_SIZING_FIXED(300), 
				.height = CLAY_SIZING_GROW(0) }, 
				.padding = CLAY_PADDING_ALL(16), 
				.childGap = 16 },
                .backgroundColor = COLOR_LIGHT
            }){
            	CLAY({
					.id = CLAY_ID("MainContent"), 
					.layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } }, 
					.backgroundColor = COLOR_LIGHT }){}
        	}
		}
		Clay_RenderCommandArray render_commands = Clay_EndLayout();
    	
		SDL_SetRenderDrawColor(application_state.renderer_data.renderer, 0, 0, 0, 255);
    	SDL_RenderClear(application_state.renderer_data.renderer);
		SDL_Clay_RenderClayCommands(&application_state.renderer_data, &render_commands);
		SDL_RenderPresent(application_state.renderer_data.renderer);
	}

    // Cleanup
    SDL_GL_DestroyContext(application_state.gl_context);
    SDL_DestroyWindow(application_state.window);
    SDL_Quit();

    return 0;
}
