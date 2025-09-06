#ifndef UI_H
#define UI_H

#include "xtdlib.h"
#include "clay.h"

typedef struct ApplicationState ApplicationState; // forward declaration

typedef struct File {
	char *path;
	char *name;
	char *extension;
} File;

typedef struct Directory {
	char *name;
	char *path;

	struct Directory **child_directories;
	u32 num_child_directories;
	
	struct File **child_files;
	u32 num_child_files;

} Directory;

//=============================================================================
// UI CONSTANTS
//=============================================================================

// path relative to project root
#define FONT_DIRECTORY "assets/fonts"
#define ICON_DIRECTORY "assets/icons"

// construct path from project_root/bin
#define FONT_PATH(ttf_file_name) "../" FONT_DIRECTORY "/" ttf_file_name
#define ICON_PATH(svg_file_name) "../" ICON_DIRECTORY "/" svg_file_name

typedef enum FontId {
	FONT_ID_ROBOTO_REGULAR,
	FONT_ID_NUM_FONT_IDS
} FontId;

typedef enum IconId {
ICON_ID_CLOSE,
	ICON_ID_RESTORE_WINDOW,
	ICON_ID_MAXIMIZE,
	ICON_ID_MINIMIZE,
	ICON_ID_DIRECTORY_ARROW_RIGHT,
	ICON_ID_DIRECTORY_ARROW_DOWN,
	NUM_ICON_IDS
} IconId;

static const Clay_Color COLOR_TRANSPARENT = (Clay_Color) {0, 0, 0, 0};
static const Clay_Color COLOR_MAGENTA = (Clay_Color) {255, 0, 255, 255};
static const Clay_Color COLOR_BACKGROUND_HEIGHT_0 = (Clay_Color) {25, 27, 28, 255};
static const Clay_Color COLOR_BACKGROUND_HEIGHT_1 = (Clay_Color) {31, 34, 35, 255};
static const Clay_Color COLOR_BACKGROUND_HEIGHT_2 = (Clay_Color) {39, 42, 43, 255};
static const Clay_Color COLOR_TEXT_LIGHT = (Clay_Color) {170, 170, 170, 255};
static const Clay_Color COLOR_HIGHLIGHT_BLUE = (Clay_Color) {25, 70, 86, 255};
static const Clay_Color COLOR_HIGHLIGHT_RED  = (Clay_Color) {117, 64, 64, 255};
static const Clay_Color COLOR_BORDER = (Clay_Color) {52, 58, 59, 255};

//=============================================================================
// LAYOUTS
//=============================================================================

Clay_RenderCommandArray application_layout (ApplicationState *app);
void application_header_layout (ApplicationState *app);

void file_explorer_layout (ApplicationState *app);
void file_explorer_file_layout (ApplicationState *app, File *file, i32 id);
void file_explorer_directory_layout (ApplicationState *app, Directory *directory, i32 id);

//=============================================================================
// INTERACTIONS
//=============================================================================

// application header interactions
void handle_application_minimize_button (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data);
void handle_application_maximize_button (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data);
void handle_application_close_button    (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data);

// file explorerer interactions
void handle_directory_expand_button (Clay_ElementId id, Clay_PointerData pointer_data, intptr_t user_data);

#endif // UI_H
