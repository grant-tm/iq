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

void render_rectangle (SDL_Renderer *render_context, const SDL_FRect rectangle_bounds, const f32 corner_radius, const Clay_Color color);	
void render_rounded_rectangle(SDL_Renderer *render_context, const SDL_FRect rect, const f32 corner_radius, const SDL_FColor clay_color);

void render_arc (SDL_Renderer *render_context, const SDL_FPoint center, const f32 radius, 
		const f32 startAngle, const f32 endAngle, const f32 thickness, const Clay_Color color);

void render_text (f32 x_position, f32 y_position, TTF_TextEngine *text_engine, TTF_Font *font, i32 font_size, 
		const char *text, const u32 text_length, Clay_Color color);

void render_border (SDL_Renderer *renderer, const SDL_FRect rect, const Clay_BorderWidth width, const Clay_CornerRadius corner_radius, const Clay_Color color);

void render_clay_commands (RenderContext *render_context, Clay_RenderCommandArray *rcommands);

#endif // RENDER_H
