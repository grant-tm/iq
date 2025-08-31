#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

typedef enum FontId {
	FONT_ID_ICONS,
	FONT_ID_ROBOTO_REGULAR,
	FONT_ID_NUM_FONT_IDS
} FontId;

typedef enum IconId {
	ICON_ID_CLOSE,
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
const Clay_Color COLOR_HIGHLIGHT_RED  = (Clay_Color) {117, 64, 64, 255};
const Clay_Color COLOR_BORDER = (Clay_Color) {52, 58, 59, 255};

void application_header (SDL_Texture **icons) {
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
			CLAY({
				.id = CLAY_ID("ApplicationMaximizeButtonIcon"),
            	.layout = {
                	.sizing = { .width = CLAY_SIZING_FIXED(20), .height = CLAY_SIZING_GROW(0) }
            	},
            	.aspectRatio = { 1.0 / 1.0 },
            	.image = {
                	.imageData = icons[ICON_ID_MAXIMIZE],
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
			CLAY({
				.id = CLAY_ID("ApplicationCloseButtonIcon"),
            	.layout = {
                	.sizing = { .width = CLAY_SIZING_FIXED(24), .height = CLAY_SIZING_FIXED(24) }
            	},
            	.aspectRatio = { 1.0 / 1.0 },
            	.image = {
                	.imageData = icons[ICON_ID_CLOSE],
            	}
        	});
		}
	} 
}

Clay_RenderCommandArray main_layout (SDL_Texture **icons) {
	
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
		application_header(icons);
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

