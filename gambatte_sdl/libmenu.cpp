/* libmenu.c
 * code for generating simple menus
 * public domain
 * by abhoriel
 */

#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#include "libmenu.h"
#include "SFont.h"
#include "menu.h"

static void display_menu(SDL_Surface *surface, menu_t *menu);
static void display_menu_cheat(SDL_Surface *surface, menu_t *menu);
static void redraw(menu_t *menu);
static void redraw_cheat(menu_t *menu);
static void invert_rect(SDL_Surface* surface, SDL_Rect *rect);
static void put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
static Uint32 get_pixel(SDL_Surface *surface, int x, int y);

void load_border(std::string borderfilename);

static int quit_menu;
static SDL_Surface *screen = NULL;
static SFont_Font* font = NULL;

SDL_Surface *menuscreen;
SDL_Surface *menuscreencolored;

// Default config values
int selectedscaler = 0, showfps = 0, ghosting = 1, gameiscgb = 0;
uint32_t menupalblack = 0x000000, menupaldark = 0x505450, menupallight = 0xA8A8A8, menupalwhite = 0xF8FCF8;
std::string dmgbordername = "No border.png", gbcbordername = "No border.png", palname = "No palette.png";
std::string homedir = getenv("HOME");


void libmenu_set_screen(SDL_Surface *set_screen) {
	screen = set_screen;
}

void libmenu_set_font(SFont_Font *set_font) {
	font = set_font;
}

int menu_main(menu_t *menu) {
    SDL_Event event;
	int dirty, loop;
	loop = 0;
	while((menu->entries[menu->selected_entry]->selectable == 0) && (loop < menu->n_entries)) { //ensure we select a selectable entry, if there is any.
		if (menu->selected_entry < menu->n_entries - 1) {
			++menu->selected_entry;
		} else {
			menu->selected_entry = 0;
		}
		loop++;
	}
    redraw(menu);
	quit_menu = 0;
    while (menu->quit == 0) {
        dirty = 0;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_UP:
							loop = 0;
							do {
								if (menu->selected_entry > 0) {
									--menu->selected_entry;
								} else {
									menu->selected_entry = menu->n_entries - 1;
								}
								loop++;
							} while((menu->entries[menu->selected_entry]->selectable == 0) && (loop < menu->n_entries)); //ensure we select a selectable entry, if there is any.
							dirty = 1;
							break;
						case SDLK_DOWN:
							loop = 0;
							do {
								if (menu->selected_entry < menu->n_entries - 1) {
									++menu->selected_entry;
								} else {
									menu->selected_entry = 0;
								}
								loop++;
							} while((menu->entries[menu->selected_entry]->selectable == 0) && (loop < menu->n_entries)); //ensure we select a selectable entry, if there is any.
							dirty = 1;
							break;
						case SDLK_LEFT:
							if (menu->entries[menu->selected_entry]->is_shiftable) {
								if (menu->entries[menu->selected_entry]->selected_entry > 0) {
									--menu->entries[menu->selected_entry]->selected_entry;
									dirty = 1;
								} else {
									menu->entries[menu->selected_entry]->selected_entry = menu->entries[menu->selected_entry]->n_entries - 1;
									dirty = 1;
								}
							}				
							break;
						case SDLK_RIGHT:
							if (menu->entries[menu->selected_entry]->is_shiftable) {
								if (menu->entries[menu->selected_entry]->selected_entry < menu->entries[menu->selected_entry]->n_entries - 1) {
									++menu->entries[menu->selected_entry]->selected_entry;
									dirty = 1;
								} else {
									menu->entries[menu->selected_entry]->selected_entry = 0;
									dirty = 1;
								}
							}
							break;
						case SDLK_RETURN: 	/* start button */
						case SDLK_LCTRL:	/* A button */
							if ((menu->entries[menu->selected_entry]->callback != NULL) && (!menu->entries[menu->selected_entry]->is_shiftable)) {
								menu->entries[menu->selected_entry]->callback(menu);
								redraw(menu);
							}
							break;
						case SDLK_LALT: /* B button, being used as 'back' */
							if (menu->back_callback != NULL) {
								menu->back_callback(menu);
							}
							break;
						default:
							break;
					}
				default:
					break;
			}
		}
		if (dirty) {
			redraw(menu);
		}
		SDL_Delay(10);
	}
	quit_menu = 0;
	/* doing this twice is just an ugly hack to get round an 
	 * opendingux pre-release hardware surfaces bug */

	clear_surface(screen, 0);
	redraw(menu);
	//SDL_Flip(screen);
	clear_surface(screen, 0);
	redraw(menu); // redraw function also flips the screen. delete and restore sdl_flip if problematic
	//SDL_Flip(screen);
	
	return menu->selected_entry;
}

int menu_cheat(menu_t *menu) {
    SDL_Event event;
	int dirty, loop;
	loop = 0;
	while((menu->entries[menu->selected_entry]->selectable == 0) && (loop < menu->n_entries)) { //ensure we select a selectable entry, if there is any.
		if (menu->selected_entry < menu->n_entries - 1) {
			++menu->selected_entry;
		} else {
			menu->selected_entry = 0;
		}
		loop++;
	}
    redraw_cheat(menu);
	quit_menu = 0;
    while (menu->quit == 0) {
        dirty = 0;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_LEFT:
							loop = 0;
							do {
								if (menu->selected_entry > 0) {
									--menu->selected_entry;
								} else {
									menu->selected_entry = menu->n_entries - 1;
								}
								loop++;
							} while((menu->entries[menu->selected_entry]->selectable == 0) && (loop < menu->n_entries)); //ensure we select a selectable entry, if there is any.
							dirty = 1;
							break;
						case SDLK_RIGHT:
							loop = 0;
							do {
								if (menu->selected_entry < menu->n_entries - 1) {
									++menu->selected_entry;
								} else {
									menu->selected_entry = 0;
								}
								loop++;
							} while((menu->entries[menu->selected_entry]->selectable == 0) && (loop < menu->n_entries)); //ensure we select a selectable entry, if there is any.
							dirty = 1;
							break;
						case SDLK_DOWN:
							if (menu->entries[menu->selected_entry]->is_shiftable) {
								if (menu->entries[menu->selected_entry]->selected_entry > 0) {
									--menu->entries[menu->selected_entry]->selected_entry;
									dirty = 1;
								} else {
									menu->entries[menu->selected_entry]->selected_entry = menu->entries[menu->selected_entry]->n_entries - 1;
									dirty = 1;
								}
							}				
							break;
						case SDLK_UP:
							if (menu->entries[menu->selected_entry]->is_shiftable) {
								if (menu->entries[menu->selected_entry]->selected_entry < menu->entries[menu->selected_entry]->n_entries - 1) {
									++menu->entries[menu->selected_entry]->selected_entry;
									dirty = 1;
								} else {
									menu->entries[menu->selected_entry]->selected_entry = 0;
									dirty = 1;
								}
							}
							break;
						case SDLK_RETURN: 	/* start button */
						case SDLK_LCTRL:	/* A button, being used as 'accept' */
							if (menu->entries[menu->selected_entry]->callback != NULL) {
								menu->entries[menu->selected_entry]->callback(menu);
								redraw_cheat(menu);
							}
							break;
						case SDLK_LALT: /* B button, being used as 'back' */
							if (menu->back_callback != NULL) {
								menu->back_callback(menu);
							}
							break;
						default:
							break;
					}
				default:
					break;
			}
		}
		if (dirty) {
			redraw_cheat(menu);
		}
		SDL_Delay(10);
	}
	quit_menu = 0;
	/* doing this twice is just an ugly hack to get round an 
	 * opendingux pre-release hardware surfaces bug */

	clear_surface(screen, 0);
	redraw_cheat(menu);
	//SDL_Flip(screen);
	clear_surface(screen, 0);
	redraw_cheat(menu); // redraw function also flips the screen. delete and restore sdl_flip if problematic
	//SDL_Flip(screen);
	
	return menu->selected_entry;
}

static void display_menu(SDL_Surface *surface, menu_t *menu) {
    int font_height = SFont_TextHeight(font);
    int i;
    int line =  0;
    SDL_Rect highlight;
    char *text;
    char buffer[64];
    int width;

    int linelimit = 13;
    int posbegin = 0;
    int posend = menu->n_entries;
    int uparrow = 0;
    int downarrow = 0;
    int num_selectable = 0;
    if(menu->n_entries > linelimit){ // menu scrolling
    	if(menu->selected_entry <= 6){
    		posbegin = 0;
    		posend = posbegin + linelimit;
    		uparrow = 0;
    		downarrow = 1;
    	} else if(menu->selected_entry > 6){
    		if((menu->selected_entry + 6) < menu->n_entries){
    			posbegin = menu->selected_entry - 6;
    			posend = menu->selected_entry + 7;
    		} else {
    			posbegin = menu->n_entries - linelimit;
    			posend = menu->n_entries;
    		}
    		if((menu->selected_entry + 7) < menu->n_entries){
    			uparrow = 1;
    			downarrow = 1;
    		} else {
    			uparrow = 1;
    			downarrow = 0;
    		}
    	}
    } else {
    	uparrow = 0;
    	downarrow = 0;
    }
    const int highlight_margin = 0;
    paint_titlebar();
    SFont_WriteCenter(surface, font, (line * font_height), menu->header);
    line ++;
    SFont_WriteCenter(surface, font, (line * font_height), menu->title);

    if(menu->n_entries >= linelimit){ // menu items fill entire screen, do not require centering
	    if(uparrow == 1){
	    	line ++;
	    	SFont_WriteCenter(surface, font, line * font_height, "{"); // up arrow
	    	line ++;
	    } else {
	    	line += 2;
	    }
		for (i = posbegin; i < posend; i++) {
			if (menu->entries[i]->is_shiftable) {
				sprintf(buffer, "%s: <%s>", menu->entries[i]->text, menu->entries[i]->entries[menu->entries[i]->selected_entry]);
				text = buffer;
			} else {
				text = menu->entries[i]->text;
			}
			SFont_WriteCenter(surface, font, line * font_height, text);
			if ((menu->selected_entry == i) && (menu->entries[i]->selectable == 1)){ // only highlight selected entry if it's selectable
				width = SFont_TextWidth(font, text);
				highlight.x = ((surface->w - width) / 2) - highlight_margin;
				highlight.y = line * font_height;
				highlight.w = width + (highlight_margin * 2);
				highlight.h = font_height;
				invert_rect(surface, &highlight);
			}
			line++;
		}
		if(downarrow == 1){
	    	SFont_WriteCenter(surface, font, line * font_height, "}"); // down arrow
	    }
	} else { // few menu items, require centering

		int posoffset = floor((linelimit - menu->n_entries) / 2);
		line += 2;
		line += posoffset;
		for (i = posbegin; i < posend; i++) {
			if (menu->entries[i]->is_shiftable) {
				sprintf(buffer, "%s: <%s>", menu->entries[i]->text, menu->entries[i]->entries[menu->entries[i]->selected_entry]);
				text = buffer;
			} else {
				text = menu->entries[i]->text;
			}
			SFont_WriteCenter(surface, font, line * font_height, text);
			if ((menu->selected_entry == i) && (menu->entries[i]->selectable == 1)){ // only highlight selected entry if it's selectable
				width = SFont_TextWidth(font, text);
				highlight.x = ((surface->w - width) / 2) - highlight_margin;
				highlight.y = line * font_height;
				highlight.w = width + (highlight_margin * 2);
				highlight.h = font_height;
				invert_rect(surface, &highlight);
			}
			line++;
		}
	}
	for (i = 0; i < menu->n_entries; i++) {
		if(menu->entries[i]->selectable == 1){
			num_selectable++;
		}
	}
	if(num_selectable == 0){
		SFont_WriteCenter(surface, font, 17 * font_height, "B-Back        A-Back"); // 17 = last line of screen (footer)
	} else {
		SFont_WriteCenter(surface, font, 17 * font_height, "B-Back      A-Select"); // 17 = last line of screen (footer)
	}
    
}

static void display_menu_cheat(SDL_Surface *surface, menu_t *menu) {
    int font_height = SFont_TextHeight(font);
    int font_width = SFont_TextWidth(font, "F");
    int i;
    int line = 0, column = 0;
    int h_offset = 0;
    SDL_Rect highlight;
    char *text;
    char buffer[8];

    const int highlight_margin = 0;
    paint_titlebar();
    SFont_WriteCenter(surface, font, (line * font_height), menu->header);
    line ++;
    SFont_WriteCenter(surface, font, (line * font_height), menu->title);

    h_offset = (surface->w - (font_width * menu->n_entries)) / 2;

    line += 6;
    SFont_WriteCenter(surface, font, line * font_height, "Enter code:");
    line += 3;
    column = 0;
	for (i = 0; i < menu->n_entries; i++) {
		if (menu->entries[i]->is_shiftable) {
			sprintf(buffer, "%s", menu->entries[i]->entries[menu->entries[i]->selected_entry]);
			text = buffer;
		} else {
			text = menu->entries[i]->text;
		}
		SFont_Write(surface, font, (column * font_width) + h_offset, line * font_height, text);
		if ((menu->selected_entry == i) && (menu->entries[i]->selectable == 1)){ // only highlight selected entry if it's selectable
			if (menu->entries[i]->is_shiftable) {
				SFont_Write(surface, font, (column * font_width) + h_offset, (line - 1) * font_height, "{");
				SFont_Write(surface, font, (column * font_width) + h_offset, (line + 1) * font_height, "}");
			}
				highlight.x = (column * font_width) + h_offset - highlight_margin;
				highlight.y = line * font_height;
				highlight.w = font_width + (highlight_margin * 2);
				highlight.h = font_height;
			invert_rect(surface, &highlight);
		}
		if(column >= 10){
			line += 3;
			column = 0;
		} else {
			column++;
		}
	}

    SFont_WriteCenter(surface, font, 17 * font_height, "B-Back       A-Apply"); // 17 = last line of screen (footer)
}

menu_t *new_menu() {
	menu_t *menu = (menu_t *)malloc(sizeof(menu_t));
	menu->header = NULL;
	menu->title = NULL;
	menu->entries = NULL;
	menu->n_entries = 0;
	menu->selected_entry = 0;
	menu->quit = 0;
	menu->back_callback = NULL;
	return menu;
}

void delete_menu(menu_t *menu) {
	int i;
	if (menu->header != NULL)
		free(menu->header);
	if (menu->title != NULL)
		free(menu->title);
	if (menu->entries != NULL) {
		for (i = 0; i < menu->n_entries; i++) {
			delete_menu_entry(menu->entries[i]);
		}
		free(menu->entries);
	}
	free(menu);
}

void menu_set_header(menu_t *menu, const char *header) {
	if (menu->header != NULL)
		free(menu->header);
	menu->header = (char *)malloc(strlen(header) + 1);
	strcpy(menu->header, header);
}

void menu_set_title(menu_t *menu, const char *title) {
	if (menu->title != NULL)
		free(menu->title);
	menu->title = (char *)malloc(strlen(title) + 1);
	strcpy(menu->title, title);
}

void menu_add_entry(menu_t *menu, menu_entry_t *entry) {
	++menu->n_entries;
	menu->entries = (menu_entry_t **)realloc(menu->entries, sizeof(menu_entry_t) * menu->n_entries);
        menu->entries[menu->n_entries - 1] = entry;
}

menu_entry_t *new_menu_entry(int is_shiftable) {
	menu_entry_t *entry = (menu_entry_t *)malloc(sizeof(menu_entry_t));
	entry->entries = NULL;
    entry->text = NULL;
	entry->is_shiftable = is_shiftable;
	entry->selectable = 1;
	entry->n_entries = 0;
	entry->selected_entry = 0;
    entry->callback = NULL;
	return entry;
}

void delete_menu_entry(menu_entry_t *entry) {
	if (entry->entries != NULL)
		free(entry->entries);
	if (entry->text != NULL)
		free(entry->text);
	free(entry);
}

void menu_entry_set_text(menu_entry_t *entry, const char *text) {
	if (entry->text != NULL) {
		free(entry->text);
	}
	entry->text = (char *)malloc(strlen(text) + 1);
	strcpy(entry->text, text);
}

void menu_entry_set_text_no_ext(menu_entry_t *entry, const char *text) { // always removes last 4 characters
	if (entry->text != NULL) {
		free(entry->text);
	}
	entry->text = (char *)calloc((strlen(text)-3), sizeof(char));
	strncpy(entry->text, text, (strlen(text)-4));
}

void menu_entry_add_entry(menu_entry_t *entry, const char* text) {
	assert(entry->is_shiftable == 1);
	++entry->n_entries;
	entry->entries = (char **)realloc(entry->entries, sizeof(char *) * entry->n_entries);
	entry->entries[entry->n_entries - 1] = (char *)malloc(strlen(text) + 1);
	strcpy(entry->entries[entry->n_entries - 1], text);
}

void callback_menu_quit(menu_t *caller_menu) {
	caller_menu->quit = 1;
}

void set_menu_palette(uint32_t valwhite, uint32_t vallight, uint32_t valdark, uint32_t valblack) {
		menupalwhite = valwhite;
		menupallight = vallight;
		menupaldark = valdark;
		menupalblack = valblack;
}

void init_menusurfaces(){
	menuscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 144, 16, 0, 0, 0, 0);
	menuscreencolored = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 144, 32, 0, 0, 0, 0);
}

void free_menusurfaces(){
	SDL_FreeSurface(menuscreen);
	SDL_FreeSurface(menuscreencolored);
}

int currentEntryInList(menu_t *menu, std::string fname){
	fname = fname.substr(0, fname.length() - 4);
    int count = menu->n_entries;
    int i;
    for (i = 0; i < count; ++i) {
    	if(strcmp(fname.c_str(), menu->entries[i]->text) == 0){
    		return i;
    	}
    }
    return 0;
}

void paint_titlebar(){
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 160;
    rect.h = 16;
    SDL_FillRect(menuscreen, &rect, 0xA0A0A0);
    rect.x = 0;
    rect.y = 136;
    rect.w = 160;
    rect.h = 8;
    SDL_FillRect(menuscreen, &rect, 0xA0A0A0);
}

void load_border(std::string borderfilename){ //load border from menu
	SDL_FreeSurface(borderimg);
	std::string fullimgpath = (homedir + "/.gambatte/borders/");
	fullimgpath += (borderfilename);
    borderimg = IMG_Load(fullimgpath.c_str());
    if(!borderimg){
    	clear_surface(screen, 0);
    	blitter_p->scaleMenu();
		SDL_Flip(screen); // ugly workaround for double-buffer
		clear_surface(screen, 0);
		clear_surface(menuscreen, 0xFFFFFF);
		blitter_p->scaleMenu();
    	if(borderfilename != "No border.png"){
    		printf("error loading %s\n", fullimgpath.c_str());
    	}
    }
}

void paint_border(SDL_Surface *surface){
	SDL_Rect rect;
	switch(selectedscaler) {
		case 0:		/* no scaler */
			rect.x = 0;
    		rect.y = 0;
    		rect.w = 320;
    		rect.h = 240;
    		break;
		case 1:		/* Ayla's 1.5x scaler */
    		rect.x = 0;
    		rect.y = 240;
    		rect.w = 320;
    		rect.h = 240;
    		break;
		case 2:		/* Ayla's fullscreen scaler */
    		rect.x = 0;
    		rect.y = 0;
    		rect.w = 0;
    		rect.h = 0;
			break;
		case 3:		/* Hardware 1.25x */
			rect.x = 32;
    		rect.y = 23;
    		rect.w = 256;
    		rect.h = 192;
			break;
		case 4:		/* Hardware 1.36x */
			rect.x = 48;
    		rect.y = 31;
    		rect.w = 224;
    		rect.h = 176;
			break;
		case 5:		/* Hardware 1.5x */
			rect.x = 56;
    		rect.y = 39;
    		rect.w = 208;
    		rect.h = 160;
			break;
		case 6:		/* Hardware 1.66x*/
			rect.x = 64;
    		rect.y = 47;
    		rect.w = 192;
    		rect.h = 144;
			break;
		case 7:		/* Hardware Fullscreen */
			rect.x = 0;
    		rect.y = 0;
    		rect.w = 0;
    		rect.h = 0;
			break;
		default:
			rect.x = 0;
    		rect.y = 0;
    		rect.w = 320;
    		rect.h = 240;
			break;
	}
	SDL_BlitSurface(borderimg, &rect, surface, NULL);
}

uint32_t convert_hexcolor(SDL_Surface *surface, const uint32_t color){
	uint8_t colorr = ((color >> 16) & 0xFF);
	uint8_t colorg = ((color >> 8) & 0xFF);
	uint8_t colorb = ((color) & 0xFF);
	uint32_t result = SDL_MapRGB(surface->format, colorr, colorg, colorb);
	return result;
}

/************************************ COLORIZE MENU SCREEN *****************************************/

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

/********************************END OF COLORIZE MENU***********************************/

static void invert_rect(SDL_Surface* surface, SDL_Rect *rect) {
	/* FIXME: with 32 bit color modes, alpha will be inverted */
	int x, y;
	Uint32 pixel;
	int max_x, max_y;
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) < 0) {
			fprintf(stderr, "could not lock surface\n");
			return;
		}
	}
	
	max_y = rect->y + rect->h;
	max_x = rect->x + rect->w;
	
	if (max_x > surface->w)
		max_x = surface->w;
	if (max_y > surface->h)
		max_y = surface->h;
		
	for (y = rect->y; y < max_y; y++) {
		for (x = rect->x; x < max_x; x++) {
			/* smooth corners */
			//if ((y == rect->y) || (y == max_y - 1)) {
			//	if ((x == rect->x) || (x == max_x - 1))
			//		continue;
			//}
			pixel = get_pixel(surface, x, y);
			pixel = ~pixel;
			put_pixel(surface, x, y, pixel);
		}
	}
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
}

static void redraw(menu_t *menu) {
	clear_surface(menuscreen, 0xFFFFFF);
	if((!gambatte_p->isCgb()) && (dmgbordername != "No border.png")) { // if system is DMG
		clear_surface(screen, convert_hexcolor(screen, menupalwhite));
		paint_border(screen);
	} else if((gambatte_p->isCgb()) && (gbcbordername != "No border.png")) { // if system is GBC
		clear_surface(screen, 0xFFFFFF);
		paint_border(screen);
	}
		
	display_menu(menuscreen, menu);
	blitter_p->scaleMenu();
	SDL_Flip(screen);
}

static void redraw_cheat(menu_t *menu) {
	clear_surface(menuscreen, 0xFFFFFF);
	if((!gambatte_p->isCgb()) && (dmgbordername != "No border.png")) { // if system is DMG
		clear_surface(screen, convert_hexcolor(screen, menupalwhite));
		paint_border(screen);
	} else if((gambatte_p->isCgb()) && (gbcbordername != "No border.png")) { // if system is GBC
		clear_surface(screen, 0xFFFFFF);
		paint_border(screen);
	}
		
	display_menu_cheat(menuscreen, menu);
	blitter_p->scaleMenu();
	SDL_Flip(screen);
}

void clear_surface(SDL_Surface *surface, Uint32 color) {
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = surface->w;
	rect.h = surface->h;
	SDL_FillRect(surface, &rect, color);
}

static void put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	switch(bpp) {
		case 1:
		*p = (Uint8)pixel;
		break;
	case 2:
		*(Uint16 *)p = (Uint16)pixel;
		break;
	case 3:
		break;
	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

static Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	switch(bpp) {
		case 1:
		return *p;
		break;
	case 2:
		return *(Uint16 *)p;
		break;
	case 3:
		return 0;
		break;
	case 4:
		return *(Uint32 *)p;
		break;
	}
	return 0;
}

void loadPalette(std::string palettefile){
	Uint32 values[12];
	std::string filepath = (homedir + "/.gambatte/palettes/");
    filepath.append(palettefile);

	FILE * fpal;
    fpal = fopen(filepath.c_str(), "r");
    if (fpal == NULL) {
		printf("Failed to open palette file %s\n", filepath.c_str());
		return;
	}
    int j = 0;
    for (int i = 0; i < 20; ++i) { // i do 20 tries, but 12 is enough. TODO: Find a better way of parsing the palette values.
        if(fscanf(fpal, "%x", &values[j]) == 1){
            j++;
        }
    }
    if (j == 12){ // all 12 palette values were successfully loaded
        set_menu_palette(values[0], values[1], values[2], values[3]);
        int m = 0;
        for (int i = 0; i < 3; ++i) {
            for (int k = 0; k < 4; ++k) {
                gambatte_p->setDmgPaletteColor(i, k, values[m]);
                m++;
            }
        }
    } else {
        printf("error reading: %s:\n",filepath.c_str());
        printf("bad file format or file does not exist.\n");
    }
    fclose(fpal);
}

void saveConfig(){
	std::string configfile = (homedir + "/.gambatte/config.cfg");
	FILE * cfile;
    cfile = fopen(configfile.c_str(), "w");
    if (cfile == NULL) {
		printf("Failed to open config file for writing.\n");
		return;
	}
    fprintf(cfile,
		"SHOWFPS %d\n"
		"SELECTEDSCALER %d\n"
		"PALNAME %s\n"
		"DMGBORDERNAME %s\n"
		"GBCBORDERNAME %s\n"
		"GHOSTING %d\n",
		showfps,
		selectedscaler,
		palname.c_str(),
		dmgbordername.c_str(),
		gbcbordername.c_str(),
		ghosting);
    fclose(cfile);
}

void loadConfig(){
	std::string configfile = (homedir + "/.gambatte/config.cfg");
	FILE * cfile;
	char line[4096];
    cfile = fopen(configfile.c_str(), "r");
    if (cfile == NULL) {
		printf("Failed to open config file for reading.\n");
		return;
	}
	while (fgets(line, sizeof(line), cfile)) {
		char *arg = strchr(line, ' ');
		int value;
		char charvalue[32];
		std::string stringvalue;

		if (!arg) {
			continue;
		}

		*arg = '\0';
		arg++;

		if (!strcmp(line, "SHOWFPS")) {
			sscanf(arg, "%d", &value);
			showfps = value;
		} else if (!strcmp(line, "SELECTEDSCALER")) {
			sscanf(arg, "%d", &value);
			selectedscaler = value;
		} else if (!strcmp(line, "PALNAME")) {
			int len = strlen(arg);
			if (len == 0 || len > sizeof(charvalue) - 1) {
				continue;
			}
			if (arg[len-1] == '\n') {
				arg[len-1] = '\0';
			}
			strcpy(charvalue, arg);
			palname = std::string(charvalue);
		} else if (!strcmp(line, "DMGBORDERNAME")) {
			int len = strlen(arg);
			if (len == 0 || len > sizeof(charvalue) - 1) {
				continue;
			}
			if (arg[len-1] == '\n') {
				arg[len-1] = '\0';
			}
			strcpy(charvalue, arg);
			dmgbordername = std::string(charvalue);
		} else if (!strcmp(line, "GBCBORDERNAME")) {
			int len = strlen(arg);
			if (len == 0 || len > sizeof(charvalue) - 1) {
				continue;
			}
			if (arg[len-1] == '\n') {
				arg[len-1] = '\0';
			}
			strcpy(charvalue, arg);
			gbcbordername = std::string(charvalue);
		} else if (!strcmp(line, "GHOSTING")) {
			sscanf(arg, "%d", &value);
			ghosting = value;
		}
	}
	fclose(cfile);
}
