#define XTDLIB_IMPLEMENTATION
#include "xtdlib.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
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

	bool redraw;
    
	SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GLContext gl_context;
    
	Clay_SDL3RendererData renderer_data;
    Clay_Arena clay_arena;
    
	MouseState mouse_state;

} ApplicationState;

void clay_error_handler (Clay_ErrorData errorData) {
    printf("%s\n", errorData.errorText.chars);
}

static void update_clay_dimensions_and_mouse_state (ApplicationState *app) {
    i32 screen_width, screen_height;
    SDL_GetWindowSize(app->window, &screen_width, &screen_height);
    Clay_SetLayoutDimensions((Clay_Dimensions){ screen_width, screen_height });

    Clay_SetPointerState(
        (Clay_Vector2){ app->mouse_state.position_x, app->mouse_state.position_y },
        app->mouse_state.is_down
    );

    float dt = 0.1f; // TODO: measure real frame delta
    Clay_UpdateScrollContainers(
        true,
        (Clay_Vector2){ app->mouse_state.wheel_x, app->mouse_state.wheel_y },
        dt
    );
}

static void render (ApplicationState *app) {
    Clay_RenderCommandArray cmds = main_layout();

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    SDL_Clay_RenderClayCommands(&app->renderer_data, &cmds);

    SDL_RenderPresent(app->renderer);
}

//=============================================================================
// SDL CALLBACKS
//=============================================================================

SDL_AppResult SDL_AppInit(void **out_state, int argc, char **argv) {
	xtd_ignore_unused(argc);
	xtd_ignore_unused(argv);

    ApplicationState *app = SDL_malloc(sizeof(ApplicationState));
    if (!app) return SDL_APP_FAILURE;
    SDL_memset(app, 0, sizeof(*app));
    *out_state = app;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
        return SDL_APP_FAILURE;

    if (!SDL_CreateWindowAndRenderer(
			"IQ",
            960, 540,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
            &app->window,
            &app->renderer))
        return SDL_APP_FAILURE;

    app->gl_context = SDL_GL_CreateContext(app->window);
    app->renderer_data.renderer = app->renderer;

    size_t clay_mem_size = Clay_MinMemorySize();
    app->clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_mem_size, malloc(clay_mem_size));
    Clay_Initialize(app->clay_arena, (Clay_Dimensions){960, 540}, (Clay_ErrorHandler){ clay_error_handler, 0 });

    app->redraw = true;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *s) {
    ApplicationState *app = (ApplicationState*)s;
    if (app->redraw) {
        update_clay_dimensions_and_mouse_state(app);
        render(app);
        app->redraw = false;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *s, SDL_Event *event) {
    ApplicationState *app = (ApplicationState*)s;
    switch (event->type) {
    
	case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_RESIZED:
        app->redraw = true;
        break;

    case SDL_EVENT_MOUSE_MOTION:
        app->mouse_state.position_x = event->motion.x;
        app->mouse_state.position_y = event->motion.y;
        //app->redraw = true;
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        app->mouse_state.wheel_x = event->wheel.x;
        app->mouse_state.wheel_y = event->wheel.y;
        //app->redraw = true;
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        app->mouse_state.is_down = true;
        //app->redraw = true;
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        app->mouse_state.is_down = false;
        //app->redraw = true;
        break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *s, SDL_AppResult result) {
    xtd_ignore_unused(result);

	ApplicationState *app = (ApplicationState*)s;
    if (!app) return;

    if (app->gl_context) SDL_GL_DestroyContext(app->gl_context);
    if (app->window) SDL_DestroyWindow(app->window);
    SDL_Quit();

    SDL_free(app);
}
