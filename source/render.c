#include "render.h"

typedef struct Corner {
    f32 x, y;
    f32 sign_x, sign_y;
    i32 rect_index;
} Corner;

static const Corner CORNERS[4] = {
    {1, 1, -1, -1, 0}, // top left
    {0, 1,  1, -1, 1}, // top right
    {0, 0,  1,  1, 2}, // bottom right
    {1, 0, -1,  1, 3}, // bottom left
};

static inline void add_triangle(i32 *indices, i32 *index_count, i32 a, i32 b, i32 c) {
    indices[(*index_count)++] = a;
    indices[(*index_count)++] = b;
    indices[(*index_count)++] = c;
}

SDL_FColor clay_color_to_sdl_color (Clay_Color clay_color) {
	return (SDL_FColor) {clay_color.r / 255, clay_color.g / 255, clay_color.b / 255, clay_color.a / 255};
}

#define CLAY_COLOR_TO_SDL_COLOR(color) {(color).r / 255, (color).g / 255, (color).b / 255, (color).a / 255};

void render_rectangle (RenderContext *render_context, const SDL_FRect rect, const f32 corner_radius, const Clay_Color color) {
	
	SDL_SetRenderDrawBlendMode(render_context->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(render_context->renderer, color.r, color.g, color.b, color.a);

	if (corner_radius > 0) {
		const SDL_FColor sdl_color = CLAY_COLOR_TO_SDL_COLOR(color); 
		render_rounded_rectangle(render_context, rect, corner_radius, sdl_color);
	} else {
		SDL_RenderFillRect(render_context->renderer, &rect);
	}
}

void render_rounded_rectangle (RenderContext *render_context, const SDL_FRect rect, const f32 corner_radius, const SDL_FColor color) {
    
    const f32 min_radius = xtd_min(rect.w, rect.h) / 2.0f;
    const f32 radius = xtd_min(corner_radius, min_radius);
    const i32 num_circle_segments = xtd_max(NUM_CIRCLE_SEGMENTS, (i32) radius * 0.5f);

    i32 index_count = 0;
    i32 vertex_count = 0;

    const i32 total_indices  = 6 + (4 * (num_circle_segments * 3)) + 6*4;
    const i32 total_vertices = 4 + (4 * (num_circle_segments * 2)) + 2*4;

    i32 indices[total_indices];
    SDL_Vertex vertices[total_vertices];

    const f32 left   = rect.x + radius;
    const f32 right  = rect.x + rect.w - radius;
    const f32 top    = rect.y + radius;
    const f32 bottom = rect.y + rect.h - radius;

    // center rectangle
    vertices[vertex_count++] = (SDL_Vertex){{left,  top},    color, {0, 0}}; //0 center TL
	vertices[vertex_count++] = (SDL_Vertex){{right, top},    color, {1, 0}}; //1 center TR
    vertices[vertex_count++] = (SDL_Vertex){{right, bottom}, color, {1, 1}}; //2 center BR
    vertices[vertex_count++] = (SDL_Vertex){{left,  bottom}, color, {0, 1}}; //3 center BL

    add_triangle(indices, &index_count, 0, 1, 3);
    add_triangle(indices, &index_count, 1, 2, 3);

    const f32 step = (SDL_PI_F / 2) / num_circle_segments;
    for (i32 i = 0; i < num_circle_segments; i++) {
        const f32 angle1 = (f32) i * step;
        const f32 angle2 = ((f32) i + 1.f) * step;

        for (i32 j = 0; j < 4; j++) {
            const f32 cx = CORNERS[j].x ? left : right;
            const f32 cy = CORNERS[j].y ? top  : bottom;
            const f32 sign_x = CORNERS[j].sign_x;
            const f32 sign_y = CORNERS[j].sign_y;

            f32 vx = cx + SDL_cosf(angle1) * radius * sign_x;
            f32 vy = cy + SDL_sinf(angle1) * radius * sign_y;
            vertices[vertex_count++] = (SDL_Vertex){{vx, vy}, color, {0, 0}};

            vx = cx + SDL_cosf(angle2) * radius * sign_x;
            vy = cy + SDL_sinf(angle2) * radius * sign_y;
            vertices[vertex_count++] = (SDL_Vertex){{vx, vy}, color, {0, 0}};

            add_triangle(indices, &index_count, j, vertex_count - 2, vertex_count - 1);
        }
    }

    // Top edge
    vertices[vertex_count++] = (SDL_Vertex){{left,  rect.y}, color, {0, 0}}; // top left
    vertices[vertex_count++] = (SDL_Vertex){{right, rect.y}, color, {1, 0}}; // top right
    add_triangle(indices, &index_count, 0, vertex_count - 2, vertex_count - 1);
    add_triangle(indices, &index_count, 1, 0, vertex_count - 1);

    // Right edge
    vertices[vertex_count++] = (SDL_Vertex){{rect.x + rect.w, top},    color, {1, 0}}; // right top
    vertices[vertex_count++] = (SDL_Vertex){{rect.x + rect.w, bottom}, color, {1, 1}}; // right bottom
    add_triangle(indices, &index_count, 1, vertex_count - 2, vertex_count - 1);
    add_triangle(indices, &index_count, 2, 1, vertex_count - 1);

    // Bottom edge
    vertices[vertex_count++] = (SDL_Vertex){{right, rect.y + rect.h}, color, {1, 1}}; // bottom right
    vertices[vertex_count++] = (SDL_Vertex){{left,  rect.y + rect.h}, color, {0, 1}}; // bottom left
    add_triangle(indices, &index_count, 2, vertex_count - 2, vertex_count - 1);
    add_triangle(indices, &index_count, 3, 2, vertex_count - 1);

    // Left edge
    vertices[vertex_count++] = (SDL_Vertex){{rect.x, bottom}, color, {0, 1}}; // left bottom
    vertices[vertex_count++] = (SDL_Vertex){{rect.x, top},    color, {0, 0}}; // left top
    add_triangle(indices, &index_count, 3, vertex_count - 2, vertex_count - 1);
    add_triangle(indices, &index_count, 0, 3, vertex_count - 1);

    // Render everything
    SDL_RenderGeometry(render_context->renderer, NULL, vertices, vertex_count, indices, index_count);
}

void render_arc (RenderContext *render_context, const SDL_FPoint center, const f32 radius,
    const f32 start_angle, const float end_angle, const float thickness, const Clay_Color color) {

    SDL_SetRenderDrawColor(render_context->renderer, color.r, color.g, color.b, color.a);

    const f32 rad_start = start_angle * (SDL_PI_F / 180.0f);
    const f32 rad_end   = end_angle   * (SDL_PI_F / 180.0f);

    const i32 boosted_arc_resolution = (i32) radius * 1.5f; // increase resolution for large circles (1.5 is an arbitrary coefficient)
    const i32 num_circle_segments = xtd_max(NUM_CIRCLE_SEGMENTS, boosted_arc_resolution);

    const f32 angle_step = (rad_end - rad_start) / (f32) num_circle_segments;
    const f32 thickness_step = 0.4f; // arbitrary value to avoid overlapping lines.
									 // Changing thickness_step or num_circle_segments might cause artifacts.
    
	for (f32 t = thickness_step; t < thickness - thickness_step; t += thickness_step) {
        SDL_FPoint points[num_circle_segments + 1];
        const f32 radius_t = xtd_max(radius - t, 1.0f);

        for (i32 i = 0; i <= num_circle_segments; i++) {
            const f32 angle = rad_start + i * angle_step;
            const f32 x = SDL_roundf(center.x + SDL_cosf(angle) * radius_t);
            const f32 y = SDL_roundf(center.y + SDL_sinf(angle) * radius_t);
            points[i] = (SDL_FPoint){x, y};
        }

        SDL_RenderLines(render_context->renderer, points, num_circle_segments + 1);
    }
}

void render_text (f32 x_position, f32 y_position, TTF_TextEngine *text_engine, TTF_Font *font, i32 font_size, const char *text, const u32 text_length, Clay_Color color) { 
	TTF_SetFontSize(font, font_size);

	TTF_Text *ttf_text = TTF_CreateText(text_engine, font, text, text_length);
	TTF_SetTextColor(ttf_text, color.r, color.g, color.b, color.a);
	
	TTF_DrawRendererText(ttf_text, x_position, y_position);
	
	TTF_DestroyText(ttf_text);
}

void render_clay_commands (RenderContext *render_context, Clay_RenderCommandArray *render_commands) {
    for (i32 i = 0; i < render_commands->length; i++) {
        Clay_RenderCommand *render_command = Clay_RenderCommandArray_Get(render_commands, i);
        
		const Clay_BoundingBox bounding_box = render_command->boundingBox;
        const SDL_FRect rect = {
            (i32) bounding_box.x,
            (i32) bounding_box.y,
            (i32) bounding_box.width,
            (i32) bounding_box.height
        };

        switch (render_command->commandType) {
            
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: 
			{
            	Clay_RectangleRenderData *config = &render_command->renderData.rectangle; 
				render_rectangle(render_context, rect, config->cornerRadius.topLeft, config->backgroundColor);
			} 
			break;

            case CLAY_RENDER_COMMAND_TYPE_TEXT: 
			{
				Clay_TextRenderData *config = &render_command->renderData.text;
				render_text(rect.x, rect.y, 
					render_context->text_engine, 
					render_context->fonts[config->fontId], config->fontSize, 
					config->stringContents.chars, config->stringContents.length, 
					config->textColor
				);
			}
			break;

            case CLAY_RENDER_COMMAND_TYPE_BORDER: 
			{
                Clay_BorderRenderData *config = &render_command->renderData.border;

                const f32 min_radius = xtd_min(rect.w, rect.h) / 2.0f;
                const Clay_CornerRadius clampedRadii = {
                    .topLeft     = xtd_min(config->cornerRadius.topLeft,     min_radius),
                    .topRight    = xtd_min(config->cornerRadius.topRight,    min_radius),
                    .bottomLeft  = xtd_min(config->cornerRadius.bottomLeft,  min_radius),
                    .bottomRight = xtd_min(config->cornerRadius.bottomRight, min_radius)
                };

                // edges
                SDL_SetRenderDrawColor(render_context->renderer, config->color.r, config->color.g, config->color.b, config->color.a);

                if (config->width.left > 0) {
                    const f32 starting_y = rect.y + clampedRadii.topLeft;
                    const f32 length = rect.h - clampedRadii.topLeft - clampedRadii.bottomLeft;
                    SDL_FRect line = { rect.x - 1, starting_y, config->width.left, length };
                    SDL_RenderFillRect(render_context->renderer, &line);
                }

                if (config->width.right > 0) {
                    const f32 starting_x = rect.x + rect.w - (f32) config->width.right + 1;
                    const f32 starting_y = rect.y + clampedRadii.topRight;
                    const f32 length = rect.h - clampedRadii.topRight - clampedRadii.bottomRight;
                    SDL_FRect line = { starting_x, starting_y, config->width.right, length };
                    SDL_RenderFillRect(render_context->renderer, &line);
                }

                if (config->width.top > 0) {
                    const f32 starting_x = rect.x + clampedRadii.topLeft;
                    const f32 length = rect.w - clampedRadii.topLeft - clampedRadii.topRight;
                    SDL_FRect line = { starting_x, rect.y - 1, length, config->width.top };
                    SDL_RenderFillRect(render_context->renderer, &line);
                }

                if (config->width.bottom > 0) {
                    const f32 starting_x = rect.x + clampedRadii.bottomLeft;
                    const f32 starting_y = rect.y + rect.h - (f32) config->width.bottom + 1;
                    const f32 length = rect.w - clampedRadii.bottomLeft - clampedRadii.bottomRight;
                    SDL_FRect line = { starting_x, starting_y, length, config->width.bottom };
                    SDL_SetRenderDrawColor(render_context->renderer, config->color.r, config->color.g, config->color.b, config->color.a);
                    SDL_RenderFillRect(render_context->renderer, &line);
                }

                // corners
                if (config->cornerRadius.topLeft > 0) {
                    const f32 centerX = rect.x + clampedRadii.topLeft - 1;
                    const f32 centerY = rect.y + clampedRadii.topLeft - 1;
                    render_arc(
                        render_context,
                        (SDL_FPoint){centerX, centerY},
                        clampedRadii.topLeft,
                        180.0f, 270.0f,
                        config->width.top,
                        config->color
                    );
                }

                if (config->cornerRadius.topRight > 0) {
                    const f32 centerX = rect.x + rect.w - clampedRadii.topRight;
                    const f32 centerY = rect.y + clampedRadii.topRight - 1;
                    render_arc(
                        render_context,
                        (SDL_FPoint){centerX, centerY},
                        clampedRadii.topRight,
                        270.0f, 360.0f,
                        config->width.top,
                        config->color
                    );
                }

                if (config->cornerRadius.bottomLeft > 0) {
                    const f32 centerX = rect.x + clampedRadii.bottomLeft - 1;
                    const f32 centerY = rect.y + rect.h - clampedRadii.bottomLeft;
                    render_arc(
                        render_context,
                        (SDL_FPoint){centerX, centerY},
                        clampedRadii.bottomLeft,
                        90.0f, 180.0f,
                        config->width.bottom,
                        config->color
                    );
                }

                if (config->cornerRadius.bottomRight > 0) {
                    const f32 centerX = rect.x + rect.w - clampedRadii.bottomRight;
                    const f32 centerY = rect.y + rect.h - clampedRadii.bottomRight;
                    render_arc(
                        render_context,
                        (SDL_FPoint){centerX, centerY},
                        clampedRadii.bottomRight,
                        0.0f, 90.0f,
                        config->width.bottom,
                        config->color
                    );
                }
            } 
			break;

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                Clay_BoundingBox boundingBox = render_command->boundingBox;
                currentClippingRectangle = (SDL_Rect){
                    .x = boundingBox.x,
                    .y = boundingBox.y,
                    .w = boundingBox.width,
                    .h = boundingBox.height,
                };
                SDL_SetRenderClipRect(render_context->renderer, &currentClippingRectangle);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                SDL_SetRenderClipRect(render_context->renderer, NULL);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                SDL_Texture *texture = (SDL_Texture *) render_command->renderData.image.imageData;
                const SDL_FRect dest = { rect.x, rect.y, rect.w, rect.h };
                SDL_RenderTexture(render_context->renderer, texture, NULL, &dest);
                break;
            }

            default:
                SDL_Log("Unknown render command type: %d", render_command->commandType);
        }
    }
}
