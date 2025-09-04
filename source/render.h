#ifndef RENDER_H
#define RENDER_H

#include <xtdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "clay.h"

#define NUM_CIRCLE_SEGMENTS 32

typedef struct {
    SDL_Renderer *renderer;
    TTF_TextEngine *text_engine;
    TTF_Font **fonts;
} RenderContext;

static SDL_Rect currentClippingRectangle;

void fill_rounded_rectangle (RenderContext *render_context, const SDL_FRect rect, const f32 corner_radius, const Clay_Color clay_color);

void render_arc (RenderContext *render_context, const SDL_FPoint center, const f32 radius, const f32 startAngle, const f32 endAngle, const f32 thickness, const Clay_Color color);

void render_clay_commands (RenderContext *render_context, Clay_RenderCommandArray *rcommands);

#endif // RENDER_H
