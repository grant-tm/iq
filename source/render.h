#ifndef RENDER_H
#define RENDER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "clay.h"

#define NUM_CIRCLE_SEGMENTS 16

typedef struct {
    SDL_Renderer *renderer;
    TTF_TextEngine *textEngine;
    TTF_Font **fonts;
} Clay_SDL3RendererData;

static SDL_Rect currentClippingRectangle;

void fill_rounded_rectangle (Clay_SDL3RendererData *rendererData, const SDL_FRect rect, const float cornerRadius, const Clay_Color _color);

void render_arc (Clay_SDL3RendererData *rendererData, const SDL_FPoint center, const float radius, const float startAngle, const float endAngle, const float thickness, const Clay_Color color);

void render_clay_commands (Clay_SDL3RendererData *rendererData, Clay_RenderCommandArray *rcommands);

#endif // RENDER_H
