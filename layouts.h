const Clay_Color COLOR_LIGHT = (Clay_Color) {224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color) {168, 66, 28, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color) {225, 138, 50, 255};

Clay_RenderCommandArray main_layout (void) {
	
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
			.backgroundColor = COLOR_LIGHT}){	
			CLAY({
				.id = CLAY_ID("MainContent"), 
				.layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } }, 
				.backgroundColor = COLOR_LIGHT }) {}
		}
	}
	
	return Clay_EndLayout();
}
