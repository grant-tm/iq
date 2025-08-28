const Clay_Color COLOR_BACKGROUND_HEIGHT_0 = (Clay_Color) {25, 27, 28, 255};
const Clay_Color COLOR_BACKGROUND_HEIGHT_1 = (Clay_Color) {31, 34, 35, 255};
const Clay_Color COLOR_BACKGROUND_HEIGHT_2 = (Clay_Color) {39, 42, 43, 255};

const Clay_Color COLOR_BORDER = (Clay_Color) {52, 58, 59, 255};

Clay_RenderCommandArray main_layout (void) {
	
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
		.border = { .width = {2, 2, 2, 2, 1}, .color = COLOR_BORDER } 
	}) {
		CLAY({
			.id = CLAY_ID("ApplicationBanner"),
			.layout = {
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(35) },
				.padding = CLAY_PADDING_ALL(0),
				.childGap = 0
			},
			.backgroundColor = COLOR_BACKGROUND_HEIGHT_2,
			.border = { .width = {1, 1, 1, 1, 1}, .color = COLOR_BORDER }
		}) {} 
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

Clay_RenderCommandArray base_layout (void) {
	
	Clay_BeginLayout();

	return Clay_EndLayout();
}
