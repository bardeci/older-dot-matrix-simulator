/* libmenu.h
 * code for generating simple menus
 * public domain
 * by abhoriel
 */
 
#ifndef _LIBMENU_H
#define _LIBMENU_H

#ifdef __cplusplus
extern "C" {
#endif


#include <SDL/SDL.h>
#include <string.h>
#include <string>
#include "SFont.h"

typedef struct Menu_t menu_t;

typedef struct {
	char *text;
	char **entries;
	int is_shiftable;
	int n_entries;
	int selected_entry;
	void (*callback)(menu_t *);
} menu_entry_t;

struct Menu_t {
	char *header;
	char *title;
	menu_entry_t **entries;
	int n_entries;
	int selected_entry;
	int i;
	int quit;
	void (*back_callback)(menu_t *);
};

extern SDL_Surface *menuscreen;
extern SDL_Surface *menuscreencolored;
extern int selectedscaler;
extern uint32_t menupalblack, menupaldark, menupallight, menupalwhite;
extern std::string dmgbordername, palname;


void libmenu_set_screen(SDL_Surface *set_screen);
void libmenu_set_font(SFont_Font *set_font);
int menu_main(menu_t *menu);
void set_active_menu(menu_t *menu);
menu_t *new_menu();
void delete_menu(menu_t *menu);
void menu_set_header(menu_t *menu, const char *header);
void menu_set_title(menu_t *menu, const char *title);
void menu_add_entry(menu_t *menu, menu_entry_t *entry);
menu_entry_t *new_menu_entry(int is_shiftable);
void delete_menu_entry(menu_entry_t *entry);
void menu_entry_set_text(menu_entry_t *entry, const char *text);
void menu_entry_set_text_no_ext(menu_entry_t *entry, const char *text);
void menu_entry_add_entry(menu_entry_t *entry, const char* text);
void callback_menu_quit(menu_t *menu);
void set_menu_palette(uint32_t valwhite, uint32_t vallight, uint32_t valdark, uint32_t valblack);
void init_menusurfaces();
void free_menusurfaces();
void paint_titlebar();
void convert_bw_surface_colors(SDL_Surface *surface, SDL_Surface *surface2, const uint32_t repl_col_black, const uint32_t repl_col_dark, const uint32_t repl_col_light, const uint32_t repl_col_white);
void load_border(std::string borderfilename);
void paint_border(SDL_Surface *surface);
uint32_t convert_hexcolor(SDL_Surface *surface, const uint32_t color);
int currentEntryInList(menu_t *menu, std::string text);


#ifdef __cplusplus
}
#endif


#endif
