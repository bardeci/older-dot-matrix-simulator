/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <time.h>
#include "sdlblitter.h"

#include "scalebuffer.h"
#include "../menu.h"
#include "../scaler.h"

#include <string.h>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

SDL_Surface *lastframe;
SDL_Surface *currframe;
SDL_Surface *borderimg;

SdlBlitter::SdlBlitter(const bool startFull, const Uint8 scale, const bool yuv) :
screen(NULL),
surface(NULL),
overlay(NULL),
startFlags(SDL_HWSURFACE | SDL_DOUBLEBUF | (startFull ? SDL_FULLSCREEN : 0)),
scale(scale),
scaler(0),
yuv(yuv)
{}

SdlBlitter::~SdlBlitter() {
	if (overlay) {
		SDL_UnlockYUVOverlay(overlay);
		SDL_FreeYUVOverlay(overlay);
	}
	
	if (surface != screen)
		SDL_FreeSurface(surface);
}

void init_ghostframes() {
	lastframe = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 144, 16, 0, 0, 0, 0);
	currframe = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 144, 16, 0, 0, 0, 0);
	SDL_SetAlpha(lastframe, SDL_SRCALPHA, 128);
}

void init_border_dmg(SDL_Surface *dst){ //load border on emulator start
	if (!dst){
		printf("init_border: screen is not initialized");
		return;
	}
	SDL_FreeSurface(borderimg);
	std::string fullimgpath;
    fullimgpath = (homedir + "/.gambatte/borders/");
	fullimgpath += (dmgbordername);
	borderimg = IMG_Load(fullimgpath.c_str());
	if(!borderimg){
	    printf("error loading %s\n", fullimgpath.c_str());
	    return;
	}
	if(dmgbordername != "No border.png") { // if system is DMG
		clear_surface(dst, convert_hexcolor(dst, menupalwhite));
		paint_border(dst);
	}
}

void init_border_gbc(SDL_Surface *dst){ //load border on emulator start
	if (!dst){
		printf("init_border: screen is not initialized");
		return;
	}
	SDL_FreeSurface(borderimg);
	std::string fullimgpath;
    fullimgpath = (homedir + "/.gambatte/borders/");
	fullimgpath += (gbcbordername);
	borderimg = IMG_Load(fullimgpath.c_str());
	if(!borderimg){
	    printf("error loading %s\n", fullimgpath.c_str());
	    return;
	}
	if(dmgbordername != "No border.png") { // if system is DMG
		clear_surface(dst, 0xFFFFFF);
		paint_border(dst);
	}
}

void SdlBlitter::setBufferDimensions(const unsigned int width, const unsigned int height) {
	//surface = screen = SDL_SetVideoMode(width * scale, height * scale, SDL_GetVideoInfo()->vfmt->BitsPerPixel == 16 ? 16 : 32, screen ? screen->flags : startFlags);
	FILE* aspect_ratio_file = fopen("/sys/devices/platform/jz-lcd.0/keep_aspect_ratio", "w");
	switch(selectedscaler) {
		case 0:		/* no scaler */
		case 1:		/* Ayla's 1.5x scaler */
		case 2:		/* Ayla's fullscreen scaler */
			surface = screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			break;
		case 3:		/* Hardware 1.5x */
			surface = screen = SDL_SetVideoMode(208, 160, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			if (aspect_ratio_file)
			{ 
				fwrite("1", 1, 1, aspect_ratio_file);
			}
			break;
		case 4:		/* Hardware Aspect */
			surface = screen = SDL_SetVideoMode(192, 144, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			if (aspect_ratio_file)
			{ 
				fwrite("1", 1, 1, aspect_ratio_file);
			}
			break;
		case 5:		/* Hardware Fullscreen */
			surface = screen = SDL_SetVideoMode(160, 144, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			if (aspect_ratio_file)
			{ 
				fwrite("0", 1, 1, aspect_ratio_file);
			}
			break;
		default:
			surface = screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			break;
	}
	fclose(aspect_ratio_file);

	//surface = screen = SDL_SetVideoMode(320, 240, 16, screen ? screen->flags : startFlags);
	menu_set_screen(screen);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0, 0, 0, 0);
	init_ghostframes();

	//fprintf(stderr, "surface w: %d, h: %d, pitch: %d, bpp: %d\n", surface->w, surface->h, surface->pitch, surface->format->BitsPerPixel);
	//fprintf(stderr, "hwscreen w: %d, h: %d, pitch: %d, bpp %d\n", screen->w, screen->h, screen->pitch, screen->format->BitsPerPixel);
	//surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, screen->format->BitsPerPixel, 0, 0, 0, 0);
	/*
	if (scale > 1 && screen) {
		if (yuv) {
			if ((overlay = SDL_CreateYUVOverlay(width * 2, height, SDL_UYVY_OVERLAY, screen)))
				SDL_LockYUVOverlay(overlay);
		} else
			surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, screen->format->BitsPerPixel, 0, 0, 0, 0);
	}
	*/
}

void SdlBlitter::setScreenRes() {
	FILE* aspect_ratio_file = fopen("/sys/devices/platform/jz-lcd.0/keep_aspect_ratio", "w");

	switch(selectedscaler) {
		case 0:		/* no scaler */
		case 1:		/* Ayla's 1.5x scaler */
		case 2:		/* Ayla's fullscreen scaler */
			if(screen->w != 320 || screen->h != 240)
				screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			break;
		case 3:		/* Hardware 1.5x */
			if(screen->w != 208 || screen->h != 160)
				screen = SDL_SetVideoMode(208, 160, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			if (aspect_ratio_file)
			{ 
				fwrite("1", 1, 1, aspect_ratio_file);
			}
			break;
		case 4:		/* Hardware Aspect */
			if(screen->w != 192 || screen->h != 144)
				screen = SDL_SetVideoMode(192, 144, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			if (aspect_ratio_file)
			{ 
				fwrite("1", 1, 1, aspect_ratio_file);
			}
			break;
		case 5:		/* Hardware Fullscreen */
			if(screen->w != 160 || screen->h != 144)
				screen = SDL_SetVideoMode(160, 144, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			if (aspect_ratio_file)
			{ 
				fwrite("0", 1, 1, aspect_ratio_file);
			}
			break;
		default:
			if(screen->w != 320 || screen->h != 240)
				screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
			break;
	}
	fclose(aspect_ratio_file);
}

void SdlBlitter::force320x240() {
	printf("forcing 320x240...\n");
	screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
}

const SdlBlitter::PixelBuffer SdlBlitter::inBuffer() const {
	PixelBuffer pb;
	if (overlay) {
		pb.pixels = overlay->pixels[0];
		pb.format = UYVY;
		pb.pitch = overlay->pitches[0] >> 2;
	} else if (surface) {
		pb.pixels = (Uint8*)(surface->pixels) + surface->offset;
		pb.format = surface->format->BitsPerPixel == 16 ? RGB16 : RGB32;
		pb.pitch = surface->pitch / surface->format->BytesPerPixel;
	}
	
	return pb;
}

template<typename T>
inline void SdlBlitter::swScale() {
	scaleBuffer<T>((T*)((Uint8*)(surface->pixels) + surface->offset), (T*)((Uint8*)(screen->pixels) + screen->offset), surface->w, surface->h, screen->pitch / screen->format->BytesPerPixel, scale);
}

void blend_frames(SDL_Surface *surface) {
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 160;
	rect.h = 144;
	SDL_BlitSurface(surface, NULL, currframe, &rect);
	SDL_BlitSurface(lastframe, NULL, currframe, &rect);
}

void store_lastframe(SDL_Surface *surface) {
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 160;
	rect.h = 144;
	SDL_BlitSurface(surface, NULL, lastframe, &rect);
}

void store_lastframe2(SDL_Surface *surface) { // test function - currently not used - can delete
	SDL_SetAlpha(surface, SDL_SRCALPHA, 224); // 0-255 opacity
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 160;
	rect.h = 144;
	SDL_BlitSurface(surface, NULL, lastframe, &rect);
	SDL_SetAlpha(surface, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
}

static int frames = 0;
static clock_t old_time = 0;
static int fps = 0;
static int firstframe = 0;
	
void SdlBlitter::draw() {
	clock_t cur_time;
	size_t offset;
	++frames;
	cur_time = SDL_GetTicks();

	if (cur_time > old_time + 1000) {
		fps = frames;
		frames = 0;
		old_time = cur_time;
	}
	if (!screen || !surface)
		return;

	if(firstframe <= 1){ // paints border on frames 0 and 1 to avoid double-buffer flickering
		if(gameiscgb == 1)
			init_border_gbc(screen);
		else
			init_border_dmg(screen);
	}

	if(firstframe >= 8){ // Ensure firstframe variable only gets value 0 and 1 once.
		firstframe = 2;
	} else {
		firstframe++;
	}
	
	if(ghosting == 0){
		switch(selectedscaler) {
			case 0:		/* no scaler */
				SDL_Rect dst;
				dst.x = (screen->w - surface->w) / 2;
				dst.y = ((screen->h - surface->h) / 2)-1;
				dst.w = surface->w;
				dst.h = surface->h;
				SDL_BlitSurface(surface, NULL, screen, &dst);
				break;
			case 1:		/* Ayla's 1.5x scaler */
				SDL_LockSurface(screen);
				SDL_LockSurface(surface);
				offset = (2 * (320 - 240) / 2) + ((240 - 216) / 2) * screen->pitch;
				scale15x((uint32_t*)((uint8_t *)screen->pixels + offset), (uint32_t*)surface->pixels);
				SDL_UnlockSurface(surface);
				SDL_UnlockSurface(screen);
				break;
			case 2:		/* Ayla's fullscreen scaler */
				SDL_LockSurface(screen);
				SDL_LockSurface(surface);
				fullscreen_upscale((uint32_t*)screen->pixels, (uint32_t*)surface->pixels);
				SDL_UnlockSurface(surface);
				SDL_UnlockSurface(screen);
				break;
			case 3:		/* Hardware 1.5x */
			case 4:		/* Hardware Aspect */
			case 5:		/* Hardware Fullscreen */
			default:
				SDL_Rect dst2;
				dst2.x = (screen->w - surface->w) / 2;
				dst2.y = (screen->h - surface->h) / 2;
				dst2.w = surface->w;
				dst2.h = surface->h;
				SDL_BlitSurface(surface, NULL, screen, &dst2);
				break;
		}
	} else if(ghosting == 1){
		blend_frames(surface);
		store_lastframe(surface);
		switch(selectedscaler) {
			case 0:		/* no scaler */
				SDL_Rect dst;
				dst.x = (screen->w - currframe->w) / 2;
				dst.y = ((screen->h - currframe->h) / 2)-1;
				dst.w = currframe->w;
				dst.h = currframe->h;
				SDL_BlitSurface(currframe, NULL, screen, &dst);
				break;
			case 1:		/* Ayla's 1.5x scaler */
				SDL_LockSurface(screen);
				SDL_LockSurface(currframe);
				offset = (2 * (320 - 240) / 2) + ((240 - 216) / 2) * screen->pitch;
				scale15x((uint32_t*)((uint8_t *)screen->pixels + offset), (uint32_t*)currframe->pixels);
				SDL_UnlockSurface(currframe);
				SDL_UnlockSurface(screen);
				break;
			case 2:		/* Ayla's fullscreen scaler */
				SDL_LockSurface(screen);
				SDL_LockSurface(currframe);
				fullscreen_upscale((uint32_t*)screen->pixels, (uint32_t*)currframe->pixels);
				SDL_UnlockSurface(currframe);
				SDL_UnlockSurface(screen);
				break;
			case 3:		/* Hardware 1.5x */
			case 4:		/* Hardware Aspect */
			case 5:		/* Hardware Fullscreen */
			default:
				SDL_Rect dst2;
				dst2.x = (screen->w - currframe->w) / 2;
				dst2.y = (screen->h - currframe->h) / 2;
				dst2.w = currframe->w;
				dst2.h = currframe->h;
				SDL_BlitSurface(currframe, NULL, screen, &dst2);
				break;
		}
	}
	
	/*
	if (!overlay && surface != screen) {
		if (surface->format->BitsPerPixel == 16)
			swScale<Uint16>();
		else
			swScale<Uint32>();
	}
	*/
	
	show_fps(screen, fps);
}

void SdlBlitter::scaleMenu() {
	size_t offset;

	if (!screen || !menuscreen)
		return;

	if(gambatte_p->isCgb()){
		convert_bw_surface_colors(menuscreen, menuscreencolored, 0xC00000, 0xA0A030, 0xFFFF80, 0xFFFFFF); //if game is GBC, then menu has different colors
	} else {
		convert_bw_surface_colors(menuscreen, menuscreencolored, menupalblack, menupaldark, menupallight, menupalwhite); //if game is DMG, then menu matches DMG palette
	}

	switch(selectedscaler) {
		case 0:		/* no scaler */
			SDL_Rect dst;
			dst.x = (screen->w - menuscreen->w) / 2;
			dst.y = ((screen->h - menuscreen->h) / 2)-1;
			dst.w = menuscreen->w;
			dst.h = menuscreen->h;
			SDL_BlitSurface(menuscreen, NULL, screen, &dst);
			break;
		case 1:		/* Ayla's 1.5x scaler */
			SDL_LockSurface(screen);
			SDL_LockSurface(menuscreen);
			offset = (2 * (320 - 240) / 2) + ((240 - 216) / 2) * screen->pitch;
			scale15x((uint32_t*)((uint8_t *)screen->pixels + offset), (uint32_t*)menuscreen->pixels);
			SDL_UnlockSurface(menuscreen);
			SDL_UnlockSurface(screen);
			break;
		case 2:		/* Ayla's fullscreen scaler */
			SDL_LockSurface(screen);
			SDL_LockSurface(menuscreen);
			fullscreen_upscale((uint32_t*)screen->pixels, (uint32_t*)menuscreen->pixels);
			SDL_UnlockSurface(menuscreen);
			SDL_UnlockSurface(screen);
			break;
		case 3:		/* Hardware 1.5x */
		case 4:		/* Hardware Aspect */
		case 5:		/* Hardware Fullscreen */
		default:
			SDL_Rect dst2;
			dst2.x = (screen->w - menuscreen->w) / 2;
			dst2.y = (screen->h - menuscreen->h) / 2;
			dst2.w = menuscreen->w;
			dst2.h = menuscreen->h;
			SDL_BlitSurface(menuscreen, NULL, screen, &dst2);
			break;
	}
}

void SdlBlitter::present() {
	if (!screen || !surface)
		return;
	if (overlay) {
		SDL_Rect dstr = { 0, 0, screen->w, screen->h };
		SDL_UnlockYUVOverlay(overlay);
		SDL_DisplayYUVOverlay(overlay, &dstr);
		SDL_LockYUVOverlay(overlay);
	} else {
		//SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
		SDL_Flip(screen);
	}
}

void SdlBlitter::toggleFullScreen() {
	//if (screen)
	//	screen = SDL_SetVideoMode(screen->w, screen->h, screen->format->BitsPerPixel, screen->flags ^ SDL_FULLSCREEN);
	//menu_set_screen(screen);	
}


