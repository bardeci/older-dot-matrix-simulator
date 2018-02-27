
#ifndef _SCALER_H
#define _SCALER_H

#include <stdint.h>
#include <SDL/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

void fullscreen_upscale(uint32_t *to, uint32_t *from);
void scale15x(uint32_t *to, uint32_t *from);
void convert_bw_surface_colors(SDL_Surface *surface, SDL_Surface *surface2, const uint32_t repl_col_black, const uint32_t repl_col_dark, const uint32_t repl_col_light, const uint32_t repl_col_white);

#ifdef __cplusplus
}
#endif

#endif
