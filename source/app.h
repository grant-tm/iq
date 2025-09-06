#ifndef APP_H
#define APP_H

#include "ui.h"
#include "render.h"

typedef enum EdgeMask {
	EDGE_NONE 	= 0,
	EDGE_LEFT 	= 1 << 0,
	EDGE_RIGHT 	= 1 << 1,
	EDGE_TOP 	= 1 << 2,
	EDGE_BOTTOM = 1 << 3 
} EdgeMask;

typedef struct MouseState {
    i32 global_position_x;
	i32 global_position_y;
	i32 position_x;
    i32 position_y;
    
	f32 wheel_x;
	f32 wheel_y;
    
	bool is_down;
	i32 global_drag_start_x;
	i32 global_drag_start_y;
	i32 drag_start_x;
	i32 drag_start_y;

} MouseState;

typedef struct ApplicationState {

	SDL_Window *window;
	SDL_Texture **icons; 
	RenderContext render_context;
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

	Directory *directories;
	u32 num_directories;

	Clay_ElementId last_element_clicked;

} ApplicationState;

#endif // APP_H
