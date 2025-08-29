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

//=============================================================================
// APPLICATION STATE
//=============================================================================

typedef struct MouseState {
    i32 global_position_x;
	i32 global_position_y;
	i32 position_x;
    i32 position_y;
    
	i32 wheel_x;
    i32 wheel_y;
    
	bool is_down;
	i32 global_drag_start_x;
	i32 global_drag_start_y;
	i32 drag_start_x;
	i32 drag_start_y;

} MouseState;

typedef enum ResizeMode {
	NOT_RESIZING,
	RESIZE_LEFT,
	RESIZE_RIGHT,
	RESIZE_TOP,
	RESIZE_BOTTOM,
	RESIZE_TOP_LEFT,
	RESIZE_TOP_RIGHT,
	RESIZE_BOTTOM_LEFT,
	RESIZE_BOTTOM_RIGHT
} ResizeMode;

typedef struct ApplicationState {

	bool redraw;
    
	SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GLContext gl_context;
    
	Clay_SDL3RendererData renderer_data;
    Clay_Arena clay_arena;
    
	MouseState mouse_state;
	ResizeMode resize_mode;
	bool resize_started_from_hit_test;
	i32 window_resize_start_x;
	i32 window_resize_start_y;
	i32 window_resize_start_w;
	i32 window_resize_start_h;

} ApplicationState;

//=============================================================================
// UPDATE AND RENDER
//=============================================================================

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
// WINDOW BEHAVIOR
//=============================================================================

void check_resizing (ApplicationState *app) {

	// skip edge/corner hit testing if currently engaged in resize
	if (app->resize_started_from_hit_test) {
		return;
	}

	f32 cursor_x, cursor_y;
	SDL_GetGlobalMouseState(&cursor_x, &cursor_y);

	SDL_Window *window = app->window;

	i32 window_left_x, window_top_y;
	SDL_GetWindowPosition(window, &window_left_x, &window_top_y);
	
	i32 window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	i32 window_right_x = window_left_x + window_width;
	i32 window_bottom_y = window_top_y + window_height;
	
	i32 margin = 6;

	bool cursor_on_left_axis = xtd_is_within_margin(cursor_x, window_left_x, margin);
	bool cursor_on_right_axis = xtd_is_within_margin(cursor_x, window_right_x, margin);
	bool cursor_on_top_axis = xtd_is_within_margin(cursor_y, window_top_y, margin);
	bool cursor_on_bottom_axis = xtd_is_within_margin(cursor_y, window_bottom_y, margin);

	bool cursor_on_left_edge = cursor_on_left_axis && xtd_is_between(cursor_y, window_top_y, window_bottom_y);
	bool cursor_on_right_edge = cursor_on_right_axis && xtd_is_between(cursor_y, window_top_y, window_bottom_y);
	bool cursor_on_top_edge = cursor_on_top_axis && xtd_is_between(cursor_x, window_left_x, window_right_x);
	bool cursor_on_bottom_edge = cursor_on_bottom_axis && xtd_is_between(cursor_x, window_left_x, window_right_x);

	bool cursor_on_top_left_corner = cursor_on_left_edge && cursor_on_top_edge;
	bool cursor_on_top_right_corner = cursor_on_right_edge && cursor_on_top_edge;
	bool cursor_on_bottom_right_corner = cursor_on_right_edge && cursor_on_bottom_edge;
	bool cursor_on_bottom_left_corner = cursor_on_left_edge && cursor_on_bottom_edge;
	bool cursor_on_corner = cursor_on_top_left_corner || cursor_on_top_right_corner || cursor_on_bottom_left_corner || cursor_on_bottom_right_corner;

	if (cursor_on_left_edge && !cursor_on_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE));
		app->resize_mode = RESIZE_LEFT;
	}
	else if (cursor_on_right_edge && !cursor_on_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE));
		app->resize_mode = RESIZE_RIGHT;
	}
	else if (cursor_on_top_edge && !cursor_on_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE));
		app->resize_mode = RESIZE_TOP;
	}
	else if (cursor_on_bottom_edge && !cursor_on_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE));
		app->resize_mode = RESIZE_BOTTOM;
	}
	else if (cursor_on_top_left_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE));
		app->resize_mode = RESIZE_TOP_LEFT;
	}
	else if (cursor_on_top_right_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE));
		app->resize_mode = RESIZE_TOP_RIGHT;
	}
	else if (cursor_on_bottom_left_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE));
		app->resize_mode = RESIZE_BOTTOM_LEFT;
	}
	else if (cursor_on_bottom_right_corner) {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE));
		app->resize_mode = RESIZE_BOTTOM_RIGHT;
	} else {
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT));
		app->resize_mode = NOT_RESIZING;
	}
}

void handle_resizing (ApplicationState *app) {
	
	// stop resizing when mouse is released
	if (!app->mouse_state.is_down) {
		return;
	}
	
	i32 window_x = app->window_resize_start_x;
	i32 window_y = app->window_resize_start_y;
	i32 window_w = app->window_resize_start_w;
	i32 window_h = app->window_resize_start_h;

	i32 dx = app->mouse_state.global_position_x - app->mouse_state.global_drag_start_x; 
	i32 dy = app->mouse_state.global_position_y - app->mouse_state.global_drag_start_y;
	
	i32 new_x = window_x;
	i32 new_y = window_y;
	i32 new_w = window_w;
	i32 new_h = window_h;

	switch (app->resize_mode) {
    case RESIZE_LEFT:
        new_x = window_x + dx;
        new_w = window_w - dx;
		break;
    case RESIZE_RIGHT:
        new_w = window_w + dx;
        break;
    case RESIZE_TOP:
        new_y = window_y + dy;
        new_h = window_h - dy;
        break;
	case RESIZE_BOTTOM:
        new_h = window_h + dy;
        break;
    case RESIZE_TOP_LEFT:
        new_x = window_x + dx;
        new_w = window_w - dx;
        new_y = window_y + dy;
        new_h = window_h - dy;
        break;
    case RESIZE_TOP_RIGHT:
        new_w = window_w + dx;
        new_y = window_y + dy;
        new_h = window_h - dy;
        break;
    case RESIZE_BOTTOM_LEFT:
        new_x = window_x + dx;
        new_w = window_w - dx;
        new_h = window_h + dy;
        break;
    case RESIZE_BOTTOM_RIGHT:
        new_w = window_w + dx;
        new_h = window_h + dy;
        break;
    default: break;
	}

	// enforce minimum size
	if (new_w < 100) { new_w = 100; }
	if (new_h < 100) { new_h = 100; }

	// apply new geometry
	SDL_SetWindowPosition(app->window, new_x, new_y);
	SDL_SetWindowSize(app->window, new_w, new_h);
}

//=============================================================================
// SDL CALLBACKS
//=============================================================================

SDL_AppResult SDL_AppInit (void **out_state, int argc, char **argv) {
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
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS,
            &app->window,
            &app->renderer)) {
        return SDL_APP_FAILURE;
	}
	
	SDL_SetWindowMinimumSize(app->window, 430, 270);
	SDL_GetWindowPosition(app->window, &app->window_resize_start_x, &app->window_resize_start_y);
	SDL_GetWindowSize(app->window, &app->window_resize_start_w, &app->window_resize_start_h);

    app->gl_context = SDL_GL_CreateContext(app->window);
    app->renderer_data.renderer = app->renderer;

    size_t clay_mem_size = Clay_MinMemorySize();
    app->clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_mem_size, malloc(clay_mem_size));
    Clay_Initialize(app->clay_arena, (Clay_Dimensions){960, 540}, (Clay_ErrorHandler){ clay_error_handler, 0 });

    app->redraw = true;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate (void *s) {
    ApplicationState *app = (ApplicationState*)s;
    
	check_resizing(app);
	

	if (app->redraw) {
        update_clay_dimensions_and_mouse_state(app);
        render(app);
        app->redraw = false;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent (void *s, SDL_Event *event) {
    ApplicationState *app = (ApplicationState*)s;
    switch (event->type) {
    
	case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_RESIZED:
        app->redraw = true;
        break;

    case SDL_EVENT_MOUSE_MOTION:
        f32 global_x, global_y;
		SDL_GetGlobalMouseState(&global_x, &global_y);
		app->mouse_state.global_position_x = (i32) global_x;
		app->mouse_state.global_position_y = (i32) global_y;
		
		app->mouse_state.position_x = event->motion.x;
        app->mouse_state.position_y = event->motion.y;
        handle_resizing(app);
		break;
    case SDL_EVENT_MOUSE_WHEEL:
        app->mouse_state.wheel_x = event->wheel.x;
        app->mouse_state.wheel_y = event->wheel.y;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        app->mouse_state.is_down = true;
		
		app->mouse_state.global_drag_start_x = app->mouse_state.global_position_x;
		app->mouse_state.global_drag_start_y = app->mouse_state.global_position_y;
		app->mouse_state.drag_start_x = app->mouse_state.position_x;
		app->mouse_state.drag_start_y = app->mouse_state.position_y;
        
		if (app->resize_mode != NOT_RESIZING) {
			SDL_GetWindowPosition(app->window, &app->window_resize_start_x, &app->window_resize_start_y);
			SDL_GetWindowSize(app->window, &app->window_resize_start_w, &app->window_resize_start_h);
			app->resize_started_from_hit_test = true;
		}	
		
		break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        app->mouse_state.is_down = false;
		app->resize_started_from_hit_test = false;
        break;

	case SDL_EVENT_KEY_DOWN:
		if (event->key.key == SDLK_ESCAPE) {
			return SDL_APP_SUCCESS;
		}
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit (void *s, SDL_AppResult result) {
    xtd_ignore_unused(result);

	ApplicationState *app = (ApplicationState*)s;
    if (!app) return;

    if (app->gl_context) SDL_GL_DestroyContext(app->gl_context);
    if (app->window) SDL_DestroyWindow(app->window);
    SDL_Quit();

    SDL_free(app);
}
