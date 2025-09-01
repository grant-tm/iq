#define XTDLIB_IMPLEMENTATION
#include "xtdlib.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "renderers/SDL3/clay_renderer_SDL3.c"

//=============================================================================
// UI CONSTANTS
//=============================================================================

typedef enum FontId {
	FONT_ID_ICONS,
	FONT_ID_ROBOTO_REGULAR,
	FONT_ID_NUM_FONT_IDS
} FontId;

typedef enum IconId {
	ICON_ID_CLOSE,
	ICON_ID_RESTORE_WINDOW,
	ICON_ID_MAXIMIZE,
	ICON_ID_MINIMIZE,
	NUM_ICON_IDS
} IconId;

const Clay_Color COLOR_TRANSPARENT = (Clay_Color) {0, 0, 0, 0};
const Clay_Color COLOR_BACKGROUND_HEIGHT_0 = (Clay_Color) {25, 27, 28, 255};
const Clay_Color COLOR_BACKGROUND_HEIGHT_1 = (Clay_Color) {31, 34, 35, 255};
const Clay_Color COLOR_BACKGROUND_HEIGHT_2 = (Clay_Color) {39, 42, 43, 255};
const Clay_Color COLOR_TEXT_LIGHT = (Clay_Color) {170, 170, 170, 255};
const Clay_Color COLOR_HIGHLIGHT_BLUE = (Clay_Color) {25, 70, 86, 255};

//=============================================================================
// APPLICATION STATE
//=============================================================================

const Clay_Color COLOR_HIGHLIGHT_RED  = (Clay_Color) {117, 64, 64, 255};
const Clay_Color COLOR_BORDER = (Clay_Color) {52, 58, 59, 255};

typedef enum EdgeMask {
	EDGE_NONE 	= 0,
	EDGE_LEFT 	= 1 << 0,
	EDGE_RIGHT 	= 1 << 1,
	EDGE_TOP 	= 1 << 2,
	EDGE_BOTTOM = 1 << 3 
} EdgeMask;

typedef struct ResizeRule {
	EdgeMask edge_mask;
	SDL_SystemCursor system_cursor;
} ResizeRule;

static const ResizeRule resize_rules[] = {
	{ EDGE_LEFT,                     SDL_SYSTEM_CURSOR_EW_RESIZE	},
    { EDGE_RIGHT,                    SDL_SYSTEM_CURSOR_EW_RESIZE	},
    { EDGE_TOP,                      SDL_SYSTEM_CURSOR_NS_RESIZE	},
    { EDGE_BOTTOM,                   SDL_SYSTEM_CURSOR_NS_RESIZE	},
    { EDGE_LEFT  | EDGE_TOP,         SDL_SYSTEM_CURSOR_NWSE_RESIZE	},
    { EDGE_RIGHT | EDGE_TOP,         SDL_SYSTEM_CURSOR_NESW_RESIZE	},
    { EDGE_LEFT  | EDGE_BOTTOM,      SDL_SYSTEM_CURSOR_NESW_RESIZE	},
    { EDGE_RIGHT | EDGE_BOTTOM,      SDL_SYSTEM_CURSOR_NWSE_RESIZE	},
};

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

typedef struct ApplicationState {

	SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GLContext gl_context;
	SDL_Texture **icons;
    
	Clay_SDL3RendererData renderer_data;
    Clay_Arena clay_arena;
 	
	SDL_Cursor *cursors[SDL_SYSTEM_CURSOR_COUNT];
	MouseState mouse_state;
	
	EdgeMask resize_mode;
	bool resize_started_from_hit_test;
	bool drag_started_from_hit_test;
	i32 window_resize_start_x;
	i32 window_resize_start_y;
	i32 window_resize_start_w;
	i32 window_resize_start_h;

	Clay_ElementId last_element_clicked;

} ApplicationState;

//=============================================================================
// Button Handlers
//=============================================================================

void handle_application_minimize_button (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data) {
	ApplicationState *app = (ApplicationState *) user_data;

	if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
		app->last_element_clicked = id;
	}

	if (app->last_element_clicked.id == id.id && pointer_data.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
		SDL_MinimizeWindow(app->window);
		app->last_element_clicked = CLAY_ID("null");
	}

}

void handle_application_maximize_button (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data) {
	ApplicationState *app = (ApplicationState *) user_data;

	if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
		app->last_element_clicked = id;
	}

	bool window_is_maximized = SDL_GetWindowFlags(app->window) & SDL_WINDOW_MAXIMIZED;
	bool button_triggered = (app->last_element_clicked.id == id.id) && (pointer_data.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME);	
	
	if (button_triggered && !window_is_maximized) {
		SDL_MaximizeWindow(app->window);
		app->last_element_clicked = CLAY_ID("null");
	}
	else if (button_triggered && window_is_maximized) {
		SDL_RestoreWindow(app->window);
		app->last_element_clicked = CLAY_ID("null");
	}

}

void handle_application_close_button (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data) {
	ApplicationState *app = (ApplicationState *) user_data;

	if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
		app->last_element_clicked = id;
	}

	if (app->last_element_clicked.id == id.id && pointer_data.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
		
		SDL_Event quit_event;
    	quit_event.type = SDL_EVENT_QUIT;
    	quit_event.quit.timestamp = SDL_GetTicksNS(); 		
		SDL_PushEvent(&quit_event);
		app->last_element_clicked = CLAY_ID("null");
	}
}

//=============================================================================
// LAYOUTS
//=============================================================================

void application_header (ApplicationState *app) {
	CLAY({
		.id = CLAY_ID("ApplicationHeader"),
		.layout = {
			.layoutDirection = CLAY_LEFT_TO_RIGHT,
			.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(35) },
			.padding = CLAY_PADDING_ALL(0),
			.childGap = 0,
			.childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
		},
		.backgroundColor = COLOR_BACKGROUND_HEIGHT_2,
		.border = { .width = {1, 1, 1, 1, 0}, .color = COLOR_BORDER }
	}) {
		// -- Minimize Button -----------------------------
        CLAY({
            .id = CLAY_ID("ApplicationMinimizeButton"),
            .layout = { 
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
			},
			.aspectRatio = { 1.0 / 1.0 },
            .backgroundColor = Clay_Hovered() ? COLOR_HIGHLIGHT_BLUE : COLOR_BACKGROUND_HEIGHT_2,
        }) {
			Clay_OnHover(handle_application_minimize_button, (intptr_t) app);
			CLAY({ 
				.id = CLAY_ID("ApplicationMinimizeButtonIcon"),	
				.layout = { .sizing = {.width = CLAY_SIZING_FIXED(12), .height = CLAY_SIZING_FIXED(1)}},
				.backgroundColor = COLOR_TEXT_LIGHT
			}) {}
		}
        // -- Maximize Button -----------------------------
        CLAY({
            .id = CLAY_ID("ApplicationMaximizeButton"),
            .layout = { 
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
			},
			.aspectRatio = { 1.0 / 1.0 },
            .backgroundColor = Clay_Hovered() ? COLOR_HIGHLIGHT_BLUE : COLOR_BACKGROUND_HEIGHT_2
        }) {
			Clay_OnHover(handle_application_maximize_button, (intptr_t) app);
			CLAY({
				.id = CLAY_ID("ApplicationMaximizeButtonIcon"),
            	.layout = {
                	.sizing = { .width = CLAY_SIZING_FIXED(20), .height = CLAY_SIZING_GROW(0) }
            	},
            	.aspectRatio = { 1.0 / 1.0 },
            	.image = {
                	.imageData = (SDL_GetWindowFlags(app->window) & SDL_WINDOW_MAXIMIZED) ? 
						app->icons[ICON_ID_RESTORE_WINDOW] : app->icons[ICON_ID_MAXIMIZE] 
            	}
        	});
		}
        // -- Close Button --------------------------------
        CLAY({
            .id = CLAY_ID("ApplicationCloseButton"),
            .layout = { 
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER } },
			.aspectRatio = { 1.0 / 1.0 },
            .backgroundColor = Clay_Hovered() ? COLOR_HIGHLIGHT_RED : COLOR_BACKGROUND_HEIGHT_2
        }) {
			Clay_OnHover(handle_application_close_button, (intptr_t) app);
			CLAY({
				.id = CLAY_ID("ApplicationCloseButtonIcon"),
            	.layout = {
                	.sizing = { .width = CLAY_SIZING_FIXED(24), .height = CLAY_SIZING_FIXED(24) }
            	},
            	.aspectRatio = { 1.0 / 1.0 },
            	.image = {
                	.imageData = app->icons[ICON_ID_CLOSE],
            	}
        	});
		}
	} 
}

Clay_RenderCommandArray main_layout (ApplicationState *app) {
	
	Clay_BeginLayout(); 
	
	CLAY({ 	
		.id = CLAY_ID("TopLevelContainer"), 
		.layout = { 
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, 
			.padding = CLAY_PADDING_ALL(0), 
			.childGap = 0 
		}, 
		.backgroundColor = COLOR_BACKGROUND_HEIGHT_0, 
		.border = { .width = {2, 2, 2, 2, 1}, .color = COLOR_BORDER },
	}) {
		application_header(app);
		CLAY({
			.id = CLAY_ID("LeftSideBar"),
			.layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, 
			.sizing = { .width = CLAY_SIZING_FIXED(250), 
			.height = CLAY_SIZING_GROW(0) }, 
			.padding = CLAY_PADDING_ALL(16), 
			.childGap = 16 },
			.backgroundColor = COLOR_BACKGROUND_HEIGHT_1,
			.border = { .width = {1, 1, 0, 1, 1}, .color = COLOR_BORDER } 	
		}) {}
	}
	
	return Clay_EndLayout();
}

//=============================================================================
// UPDATE AND RENDER
//=============================================================================

void clay_error_handler (Clay_ErrorData errorData) {
    printf("%s\n", errorData.errorText.chars);
}

static inline Clay_Dimensions measure_text (Clay_StringSlice text, Clay_TextElementConfig *config, void *userData)
{
    TTF_Font **fonts = userData;
    TTF_Font *font = fonts[config->fontId];
    int width, height;

    TTF_SetFontSize(font, config->fontSize);
    if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
    }

    return (Clay_Dimensions) { (float) width, (float) height };
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
    Clay_RenderCommandArray cmds = main_layout(app);

    SDL_SetRenderDrawColor(app->renderer_data.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer_data.renderer);

    SDL_Clay_RenderClayCommands(&app->renderer_data, &cmds);

    SDL_RenderPresent(app->renderer_data.renderer);
}

//=============================================================================
// WINDOW BEHAVIOR
//=============================================================================

bool check_resizing (ApplicationState *app) {
	
	//skip edge/corner hit testing if currently engaged in resize
	if (app->resize_started_from_hit_test) {
		return false;
	}
	
	f32 cursor_x, cursor_y;
	SDL_GetGlobalMouseState(&cursor_x, &cursor_y);

	SDL_Window *window = app->window;

	i32 window_left_x, window_top_y, window_width, window_height;
	SDL_GetWindowPosition(window, &window_left_x, &window_top_y);
	SDL_GetWindowSize(window, &window_width, &window_height);
	i32 window_right_x = window_left_x + window_width;
	i32 window_bottom_y = window_top_y + window_height;
	i32 margin = 6;


	b32 mask = EDGE_NONE;
    if (xtd_is_within_margin(cursor_x, window_left_x, 	margin)) mask |= EDGE_LEFT;
    if (xtd_is_within_margin(cursor_x, window_right_x,	margin)) mask |= EDGE_RIGHT;
    if (xtd_is_within_margin(cursor_y, window_top_y, 	margin)) mask |= EDGE_TOP;
    if (xtd_is_within_margin(cursor_y, window_bottom_y, margin)) mask |= EDGE_BOTTOM;

	SDL_Cursor *cursor = app->cursors[SDL_SYSTEM_CURSOR_DEFAULT];
	
	for (u32 i = 0; i < SDL_arraysize(resize_rules); i++) {
        if (resize_rules[i].edge_mask == mask) {
            cursor = app->cursors[resize_rules[i].system_cursor];
            break;
        }
    }

	SDL_SetCursor(cursor);
	app->resize_mode = mask;
	return (mask == 0);
}

void handle_resizing (ApplicationState *app) {
    if (!app->mouse_state.is_down || !app->resize_started_from_hit_test || app->drag_started_from_hit_test) {
		return;
	}

    i32 dx = app->mouse_state.global_position_x - app->mouse_state.global_drag_start_x; 
    i32 dy = app->mouse_state.global_position_y - app->mouse_state.global_drag_start_y;

    i32 new_x = app->window_resize_start_x;
    i32 new_y = app->window_resize_start_y;
    i32 new_w = app->window_resize_start_w;
    i32 new_h = app->window_resize_start_h;

    if (app->resize_mode & EDGE_LEFT) {
        new_x += dx;
        new_w -= dx;
    }
    if (app->resize_mode & EDGE_RIGHT) {
        new_w += dx;
    }
    if (app->resize_mode & EDGE_TOP) {
        new_y += dy;
        new_h -= dy;
    }
    if (app->resize_mode & EDGE_BOTTOM) {
        new_h += dy;
    }

    const i32 min_w = 430, min_h = 270;

    if (new_w < min_w) {
        if (app->resize_mode & EDGE_LEFT)
            new_x = (app->window_resize_start_x + app->window_resize_start_w) - min_w;
        new_w = min_w;
    }
    if (new_h < min_h) {
        if (app->resize_mode & EDGE_TOP)
            new_y = (app->window_resize_start_y + app->window_resize_start_h) - min_h;
        new_h = min_h;
    }

    SDL_SetWindowPosition(app->window, new_x, new_y);
    SDL_SetWindowSize(app->window, new_w, new_h);
}

bool check_dragging (ApplicationState *app) {
	
	if (app->drag_started_from_hit_test || app->resize_mode != EDGE_NONE) {
		return false;
	}
	
	i32 cursor_x = app->mouse_state.position_x;
	i32 cursor_y = app->mouse_state.position_y;

	Clay_ElementData header_data = Clay_GetElementData(CLAY_ID("ApplicationHeader"));
	xtd_assert(header_data.found == true);
	
	Clay_BoundingBox header_bounding_box = header_data.boundingBox;

	i32 drag_area_left_x = header_bounding_box.x;
	i32 drag_area_top_y = header_bounding_box.y;
	i32 drag_area_right_x = header_bounding_box.x + header_bounding_box.width;
	i32 drag_area_bottom_y = header_bounding_box.y + header_bounding_box.height;

	bool cursor_within_drag_area = 
		xtd_is_between(cursor_x, drag_area_left_x, drag_area_right_x) && 
		xtd_is_between(cursor_y, drag_area_top_y, drag_area_bottom_y);
	
	return cursor_within_drag_area;
}

void handle_dragging(ApplicationState *app) {
    
	if (!app->mouse_state.is_down || 
		!app->drag_started_from_hit_test ||
		app->resize_started_from_hit_test) { 
		return;
    }
	
	i32 window_x = app->window_resize_start_x;
	i32 window_y = app->window_resize_start_y;

	i32 dx = app->mouse_state.global_position_x - app->mouse_state.global_drag_start_x; 
	i32 dy = app->mouse_state.global_position_y - app->mouse_state.global_drag_start_y;
	
	i32 new_x = window_x + dx;
	i32 new_y = window_y + dy;

    SDL_SetWindowPosition(app->window, new_x, new_y);
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

	if (!TTF_Init()) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
        return SDL_APP_FAILURE;

    if (!SDL_CreateWindowAndRenderer(
			"IQ",
            960, 540,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS,
            &app->window,
            &app->renderer_data.renderer)) {
        return SDL_APP_FAILURE;
	}
	
	// -- Initialize Text Engine -----------------------------
	app->renderer_data.textEngine = TTF_CreateRendererTextEngine(app->renderer_data.renderer);
    if (!app->renderer_data.textEngine) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create text engine from renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app->renderer_data.fonts = SDL_calloc(FONT_ID_NUM_FONT_IDS, sizeof(TTF_Font *));
    if (!app->renderer_data.fonts) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate memory for the font array: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

	// -- Load Fonts -----------------------------------------
    TTF_Font *roboto_regular = TTF_OpenFont("resources/Roboto/Roboto-Regular.ttf", 24);
    if (!roboto_regular) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app->renderer_data.fonts[FONT_ID_ROBOTO_REGULAR] = roboto_regular;

	TTF_Font *icons = TTF_OpenFont("resources/Material-Design-Iconic-Font.ttf", 24);
    if (!icons) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app->renderer_data.fonts[FONT_ID_ICONS] = icons;

	// -- Load SVG Icons ----------------------------------
	app->icons = SDL_calloc(NUM_ICON_IDS, sizeof(SDL_Texture *));
	app->icons[ICON_ID_CLOSE] = IMG_LoadTexture(app->renderer_data.renderer, "resources/icons/close.svg");
    if (!app->icons[ICON_ID_CLOSE]) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	app->icons[ICON_ID_RESTORE_WINDOW] = IMG_LoadTexture(app->renderer_data.renderer, "resources/icons/restore_window.svg");
	if (!app->icons[ICON_ID_RESTORE_WINDOW]) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	app->icons[ICON_ID_MAXIMIZE] = IMG_LoadTexture(app->renderer_data.renderer, "resources/icons/square.svg");
    if (!app->icons[ICON_ID_MAXIMIZE]) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	app->icons[ICON_ID_MINIMIZE] = IMG_LoadTexture(app->renderer_data.renderer, "resources/icons/minimize.svg");
    if (!app->icons[ICON_ID_MINIMIZE]) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }	
	
	for (i32 i = 0; i < SDL_SYSTEM_CURSOR_COUNT; i++) {
		app->cursors[i] = SDL_CreateSystemCursor(i);
	}

	SDL_SetWindowMinimumSize(app->window, 430, 270);
	SDL_GetWindowPosition(app->window, &app->window_resize_start_x, &app->window_resize_start_y);
	SDL_GetWindowSize(app->window, &app->window_resize_start_w, &app->window_resize_start_h);

    app->gl_context = SDL_GL_CreateContext(app->window);

    size_t clay_mem_size = Clay_MinMemorySize();
    app->clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_mem_size, malloc(clay_mem_size));
    Clay_Initialize(app->clay_arena, (Clay_Dimensions){960, 540}, (Clay_ErrorHandler){ clay_error_handler, 0 });
	Clay_SetMeasureTextFunction(measure_text, app->renderer_data.fonts);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate (void *s) {
    ApplicationState *app = (ApplicationState *) s;
    	
	update_clay_dimensions_and_mouse_state(app);
	render(app);

	check_resizing(app);
	check_dragging(app);
	
	SDL_Delay(4);
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent (void *s, SDL_Event *event) {
    ApplicationState *app = (ApplicationState*)s;
    switch (event->type) {
    
	case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_RESIZED:
        break;

    case SDL_EVENT_MOUSE_MOTION:
    	
		app->mouse_state.position_x = event->motion.x;
    	app->mouse_state.position_y = event->motion.y;
    	
		f32 global_x, global_y;
		SDL_GetGlobalMouseState(&global_x, &global_y);
		app->mouse_state.global_position_x = (i32) global_x;
    	app->mouse_state.global_position_y = (i32) global_y;
		
		handle_resizing(app);
    	handle_dragging(app);
		
		Clay_SetPointerState((Clay_Vector2) { event->motion.x, event->motion.y }, app->mouse_state.is_down);
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
       	
		if (!app->resize_started_from_hit_test && app->resize_mode != EDGE_NONE) {
			SDL_GetWindowPosition(app->window, &app->window_resize_start_x, &app->window_resize_start_y);
			SDL_GetWindowSize(app->window, &app->window_resize_start_w, &app->window_resize_start_h);
			app->resize_started_from_hit_test = true;
		}
		
		if (!app->drag_started_from_hit_test && check_dragging(app)) {
			SDL_GetWindowPosition(app->window, &app->window_resize_start_x, &app->window_resize_start_y);
			app->drag_started_from_hit_test = true;
		}	

		break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        app->mouse_state.is_down = false;
		app->resize_started_from_hit_test = false;
		app->drag_started_from_hit_test = false;
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


