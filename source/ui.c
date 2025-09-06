#include "ui.h"

#include <SDL3/SDL.h>

#include "app.h"

//=============================================================================
// LAYOUTS
//=============================================================================

void application_header_layout (ApplicationState *app) {
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
		.border = { .width = {1, 1, 1, 1, 0}, .color = COLOR_BORDER },
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

void directory_component (ApplicationState *app, Directory *directory, i32 id) {
	CLAY({
		.id = CLAY_IDI("Directory", id),
		.layout = {
			.layoutDirection = CLAY_LEFT_TO_RIGHT,
			.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(24) },
			.padding = CLAY_PADDING_ALL(0),
			.childGap = 0,
			.childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
		},
		.border = { .width = {0, 0, 0, 0, 0}, .color = COLOR_BORDER },
	}) {
		CLAY({
			.id = CLAY_IDI("DirectoryExpandIcon", id),
			.layout = {
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
				.padding = CLAY_PADDING_ALL(0),
			},
			.aspectRatio = { 1.0 / 1.0 },
			.image = { .imageData = app->icons[ICON_ID_DIRECTORY_ARROW_RIGHT] },
		}) {
		}
		Clay_String directory_name = {false, 24, directory->name};
		CLAY_TEXT(directory_name, CLAY_TEXT_CONFIG({ .textColor = COLOR_TEXT_LIGHT, .fontId = FONT_ID_ROBOTO_REGULAR, .fontSize = 16 }));
	}
}

void file_explorer_layout (ApplicationState *app) {
	CLAY({
		.id = CLAY_ID("FileExplorer"),
		.layout = { 
			.layoutDirection = CLAY_TOP_TO_BOTTOM, 
			.sizing = { .width = CLAY_SIZING_FIXED(250), .height = CLAY_SIZING_GROW(0) },
			.padding = {0, 0, 4, 0}, 
			.childGap = 0 
		},
		.backgroundColor = COLOR_BACKGROUND_HEIGHT_1,
		.border = { .width = {1, 1, 0, 1, 0}, .color = COLOR_BORDER } 	
	}) {
		CLAY({
			.id = CLAY_ID("FileExplorerFilterArea"),
			.layout = {
				.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(20) },\
			},
			.backgroundColor = {255, 0, 255, 255},
		}) {}
		
		CLAY({
			.id = CLAY_ID("FileExplorerSearchResultsArea"),
			.layout = { 
				.layoutDirection = CLAY_LEFT_TO_RIGHT, 
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
				.childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
			},
		}) {
			
			CLAY({
				.id = CLAY_ID("FileExplorerSearchResultsList"),
				.layout = { 
					.layoutDirection = CLAY_TOP_TO_BOTTOM, 
					.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
					.childAlignment = { .x = CLAY_ALIGN_X_LEFT },
				},
				.clip = { .vertical = true, .childOffset = Clay_GetScrollOffset()},
			}) {
				for (u32 i = 0; i < app->num_directories; ++i) {
					directory_component(app, &app->directories[i], i);
				}
			}
			
			CLAY({
				.id = CLAY_ID("FileExplorerSearchResultScrollBar"),
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIXED(6), .height = CLAY_SIZING_GROW(0) },
				},
				.backgroundColor = {0, 255, 0, 255}
			}) {

			}			
		}	
	}
} 

Clay_RenderCommandArray application_layout (ApplicationState *app) {
	Clay_BeginLayout(); CLAY({ 	.id = CLAY_ID("TopLevelContainer"), .layout = { 
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, 
			.padding = CLAY_PADDING_ALL(0), 
			.childGap = 0 
		},
		.backgroundColor = COLOR_BACKGROUND_HEIGHT_0, 
		.border = { .width = {2, 2, 2, 2, 1}, .color = COLOR_BORDER },
	}) {
		application_header_layout(app);
		file_explorer_layout(app);
	}
	
	return Clay_EndLayout();
}

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

