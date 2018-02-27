
#include "scaler.h"
#include <SDL/SDL.h>

#ifdef __GNUC__
#       define unlikely(x)     __builtin_expect((x),0)
#       define prefetch(x, y)  __builtin_prefetch((x),(y))
#else
#       define unlikely(x)     (x)
#   define prefetch(x, y)
#endif


/* Ayla's fullscreen upscaler */
/* Upscale from 160x144 to 320x240 */
void fullscreen_upscale(uint32_t *to, uint32_t *from)
{
    uint32_t reg1, reg2, reg3, reg4;
    unsigned int x,y;

    /* Before:
     *    a b
     *    c d
     *    e f
     *
     * After (parenthesis = average):
     *    a      a      b      b
     *    (a,c)  (a,c)  (b,d)  (b,d)
     *    c      c      d      d
     *    (c,e)  (c,e)  (d,f)  (d,f)
     *    e      e      f      f
     */

    for (y=0; y < 240/5; y++) {
        for(x=0; x < 320/4; x++) {
            prefetch(to+4, 1);

            /* Read b-a */
            reg2 = *from;
            reg1 = reg2 & 0xffff0000;
            reg1 |= reg1 >> 16;

            /* Write b-b */
            *(to+1) = reg1;
            reg2 = reg2 & 0xffff;
            reg2 |= reg2 << 16;

            /* Write a-a */
            *to = reg2;

            /* Read d-c */
            reg4 = *(from + 160/2);
            reg3 = reg4 & 0xffff0000;
            reg3 |= reg3 >> 16;

            /* Write d-d */
            *(to + 2*320/2 +1) = reg3;
            reg4 = reg4 & 0xffff;
            reg4 |= reg4 << 16;

            /* Write c-c */
            *(to + 2*320/2) = reg4;

            /* Write (b,d)-(b,d) */
            if (unlikely(reg1 != reg3))
                reg1 = ((reg1 & 0xf7def7de) >> 1) + ((reg3 & 0xf7def7de) >> 1);
            *(to + 320/2 +1) = reg1;

            /* Write (a,c)-(a,c) */
            if (unlikely(reg2 != reg4))
                reg2 = ((reg2 & 0xf7def7de) >> 1) + ((reg4 & 0xf7def7de) >> 1);
            *(to + 320/2) = reg2;

            /* Read f-e */
            reg2 = *(from++ + 2*160/2);
            reg1 = reg2 & 0xffff0000;
            reg1 |= reg1 >> 16;

            /* Write f-f */
            *(to + 4*320/2 +1) = reg1;
            reg2 = reg2 & 0xffff;
            reg2 |= reg2 << 16;

            /* Write e-e */
            *(to + 4*320/2) = reg2;

            /* Write (d,f)-(d,f) */
            if (unlikely(reg2 != reg4))
                reg2 = ((reg2 & 0xf7def7de) >> 1) + ((reg4 & 0xf7def7de) >> 1);
            *(to++ + 3*320/2) = reg2;

            /* Write (c,e)-(c,e) */
            if (unlikely(reg1 != reg3))
                reg1 = ((reg1 & 0xf7def7de) >> 1) + ((reg3 & 0xf7def7de) >> 1);
            *(to++ + 3*320/2) = reg1;
        }

        to += 4*320/2;
        from += 2*160/2;
    }
}

/* Ayla's 1.5x Upscaler - 160x144 to 240x216 */
void scale15x(uint32_t *to, uint32_t *from)
{
    /* Before:
     *    a b c d
     *    e f g h
     *
     * After (parenthesis = average):
     *    a      (a,b)      b      c      (c,d)      d
     *    (a,e)  (a,b,e,f)  (b,f)  (c,g)  (c,d,g,h)  (d,h)
     *    e      (e,f)      f      g      (g,h)      h
     */

    uint32_t reg1, reg2, reg3, reg4, reg5;
    unsigned int x, y;

    for (y=0; y<216/3; y++) {
        for (x=0; x<240/6; x++) {
            prefetch(to+4, 1);

            /* Read b-a */
            reg1 = *from;
            reg5 = reg1 >> 16;
            if (unlikely((reg1 & 0xffff) != reg5)) {
                reg2 = (reg1 & 0xf7de0000) >> 1;
                reg1 = (reg1 & 0xffff) + reg2 + ((reg1 & 0xf7de) << 15);
            }

            /* Write (a,b)-a */
            *to = reg1;

            /* Read f-e */
            reg3 = *(from++ + 160/2);
            reg2 = reg3 >> 16;
            if (unlikely((reg3 & 0xffff) != reg2)) {
                reg4 = (reg3 & 0xf7de0000) >> 1;
                reg3 = (reg3 & 0xffff) + reg4 + ((reg3 & 0xf7de) << 15);
            }

            /* Write (e,f)-e */
            *(to + 2*320/2) = reg3;

            /* Write (a,b,e,f)-(a,e) */
            if (unlikely(reg1 != reg3))
                reg1 = ((reg1 & 0xf7def7de) >> 1) + ((reg3 & 0xf7def7de) >> 1);
            *(to++ + 320/2) = reg1;

            /* Read d-c */
            reg1 = *from;
            reg4 = reg1 << 16;

            /* Write c-b */
            reg5 |= reg4;
            *to = reg5;

            /* Read h-g */
            reg3 = *(from++ + 160/2);

            /* Write g-f */
            reg2 |= (reg3 << 16);
            *(to + 2*320/2) = reg2;

            /* Write (c,g)-(b,f) */
            if (unlikely(reg2 != reg5))
                reg2 = ((reg5 & 0xf7def7de) >> 1) + ((reg2 & 0xf7def7de) >> 1);
            *(to++ + 320/2) = reg2;

            /* Write d-(c,d) */
            if (unlikely((reg1 & 0xffff0000) != reg4)) {
                reg2 = (reg1 & 0xf7def7de) >> 1;
                reg1 = (reg1 & 0xffff0000) | ((reg2 + (reg2 >> 16)) & 0xffff);
            }
            *to = reg1;

            /* Write h-(g,h) */
            if (unlikely((reg3 & 0xffff) != reg3 >> 16)) {
                reg2 = (reg3 & 0xf7def7de) >> 1;
                reg3 = (reg3 & 0xffff0000) | ((reg2 + (reg2 >> 16)) & 0xffff);
            }
            *(to + 2*320/2) = reg3;

            /* Write (d,h)-(c,d,g,h) */
            if (unlikely(reg1 != reg3))
                reg1 = ((reg1 & 0xf7def7de) >> 1) + ((reg3 & 0xf7def7de) >> 1);
            *(to++ + 320/2) = reg1;
        }

        to += 2*360/2;
        from += 160/2;
    }
}


/************************************COLORIZE MENU*****************************************/

uint32_t getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;
 
    switch (bpp)
    {
        case 1:
            return *p;
            break;
 
        case 2:
            return *(uint16_t *)p;
            break;
 
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
            break;
 
        case 4:
            return *(uint32_t *)p;
            break;
 
        default:
            return 0;       /* shouldn't happen, but avoids warnings */
    }
}
 
void putpixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;
 
    switch (bpp)
    {
        case 1:
            *p = pixel;
            break;
 
        case 2:
            *(uint16_t *)p = pixel;
            break;
 
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
 
        case 4:
            *(uint32_t *)p = pixel;
            break;
    }
}
 
//----------------------------------------------------------------------------------------
// CALL THIS FUNCTION LIKE SO TO SWAP BLACK->WHITE
// uint8_t repl_black_r = 0xff,
//         repl_black_g = 0xff,
//         repl_black_b = 0xff;
// uint8_t repl_white_r = 0,
//         repl_white_g = 0,
//         repl_white_b = 0;
// uint32_t repl_col_black = SDL_MapRGB(my_surface->format, black_r, black_g, black_b);
// uint32_t repl_col_white = SDL_MapRGB(my_surface->format, white_r, white_g, white_b);
// convert_bw_surface_colors(my_surface, repl_col_black, repl_col_white);
//----------------------------------------------------------------------------------------
void convert_bw_surface_colors(SDL_Surface *surface, SDL_Surface *surface2, const uint32_t repl_col_black, const uint32_t repl_col_dark, const uint32_t repl_col_light, const uint32_t repl_col_white)
{
    const uint32_t col_black = SDL_MapRGB(surface->format, 0x40, 0x40, 0x40);
    const uint32_t col_dark = SDL_MapRGB(surface->format, 0x80, 0x80, 0x80);
    const uint32_t col_light = SDL_MapRGB(surface->format, 0xC0, 0xC0, 0xC0);
    const uint32_t col_white = SDL_MapRGB(surface->format, 0xff, 0xff, 0xff);
 
    SDL_LockSurface(surface);
    SDL_LockSurface(surface2);
 
    int x,y;
    for (y=0; y < surface->h; ++y)
    {
        for (x=0; x < surface->w; ++x)
        {
            const uint32_t pix = getpixel(surface, x, y);
            uint32_t new_pix = pix;
 
            if (pix <= col_black)
                new_pix = repl_col_black;
            else if (pix <= col_dark)
                new_pix = repl_col_dark;
            else if (pix <= col_light)
                new_pix = repl_col_light;
            else if (pix <= col_white)
                new_pix = repl_col_white;
 
            putpixel(surface2, x, y, new_pix);
        }
    }
 
    SDL_UnlockSurface(surface);
    SDL_UnlockSurface(surface2);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 160;
    rect.h = 144;
    SDL_BlitSurface(surface2, NULL, surface, &rect);

}
