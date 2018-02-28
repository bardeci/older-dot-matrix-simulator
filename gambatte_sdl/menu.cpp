#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_image.h>

#include <gambatte.h>
#include "src/blitterwrapper.h"

#include "libmenu.h"
#include "font12px.h"
#include "sfont_gameboy.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>


static SDL_Surface *screen;
static SFont_Font* font;

int init_fps_font() {
    SDL_Surface *font_bitmap_surface = NULL;
    SDL_RWops *RWops;
    
    RWops = SDL_RWFromMem(sfont_gameboy_fps, 1303);
    font_bitmap_surface = IMG_LoadPNG_RW(RWops);
    SDL_FreeRW(RWops);
    if (!font_bitmap_surface) {
        fprintf(stderr, "menu: font load error\n");
        exit(1);
    }
    font = SFont_InitFont(font_bitmap_surface);
    if (!font) {
        fprintf(stderr, "menu: font init error\n");
        exit(1);
    }

    libmenu_set_font(font);
    return 0;
}

int init_menu() {
    SDL_Surface *font_bitmap_surface = NULL;
    SDL_RWops *RWops;
    
	RWops = SDL_RWFromMem(sfont_gameboy_black, 1118);
    font_bitmap_surface = IMG_LoadPNG_RW(RWops);
    SDL_FreeRW(RWops);
    if (!font_bitmap_surface) {
        fprintf(stderr, "menu: font load error\n");
        exit(1);
    }
    font = SFont_InitFont(font_bitmap_surface);
    if (!font) {
        fprintf(stderr, "menu: font init error\n");
        exit(1);
    }

	libmenu_set_font(font);
	return 0;
}


void menu_set_screen(SDL_Surface *set_screen) {
	screen = set_screen;
	libmenu_set_screen(screen);
}

/* ============================ MAIN MENU =========================== */

static void callback_quit(menu_t *caller_menu);
static void callback_return(menu_t *caller_menu);
static void callback_savestate(menu_t *caller_menu);
static void callback_loadstate(menu_t *caller_menu);
static void callback_selectstate(menu_t *caller_menu);
static void callback_selectedstate(menu_t *caller_menu);
static void callback_options(menu_t *caller_menu);
static void callback_restart(menu_t *caller_menu);
static void callback_dmgpalette(menu_t *caller_menu); // TEST FUNCTION
static void callback_dmgborderimage(menu_t *caller_menu); // TEST FUNCTION

static gambatte::GB *gambatte_p;
BlitterWrapper *blitter_p;

void main_menu(gambatte::GB *gambatte, BlitterWrapper *blitter) {
    blitter_p = blitter;
    gambatte_p = gambatte;

    //blitter_p->force320x240(); /* force the menu to be at 320x240 screen resolution */
    SDL_EnableKeyRepeat(250, 83);
    init_menusurfaces();
    init_menu();

    menu_t *menu;
	menu_entry_t *menu_entry;
    enum {RETURN = 0, SAVE_STATE = 1, LOAD_STATE = 2, SELECT_STATE = 3, OPTIONS = 4, RESTART = 5, QUIT = 6};
    
    menu = new_menu();
    menu_set_header(menu, "GCW-Gambatte (WIP)");
	menu_set_title(menu, "Main Menu");
	menu->back_callback = callback_menu_quit;
	
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Return to game");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_return;

	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Save state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_savestate;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Load state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_loadstate;
	
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Select state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectstate;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Options");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_options;

	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Reset game");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_restart;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Quit");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_quit;

	menu_main(menu);
    
    delete_menu(menu);

    free_menusurfaces();
    SDL_EnableKeyRepeat(0, 100);
    init_fps_font();
    //blitter_p->setScreenRes(); /* switch to selected resolution */
}


static void callback_quit(menu_t *caller_menu) {
    caller_menu->quit = 1;
    exit(0);
}

static void callback_return(menu_t *caller_menu) {
    caller_menu->quit = 1;
}

static void callback_savestate(menu_t *caller_menu) {
    gambatte_p->saveState(blitter_p->inBuf().pixels, blitter_p->inBuf().pitch);
    caller_menu->quit = 1;
}

static void callback_loadstate(menu_t *caller_menu) {
	gambatte_p->loadState();
    caller_menu->quit = 1;
}

static void callback_restart(menu_t *caller_menu) {
    gambatte_p->reset();
    caller_menu->quit = 1;
}

/* ==================== SELECT STATE MENU =========================== */

static void callback_selectstate(menu_t *caller_menu) {
    #define N_STATES 10
    menu_t *menu;
	menu_entry_t *menu_entry;
    int i;
    char buffer[64];
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, "GCW-Gambatte (WIP)");
    menu_set_title(menu, "Select State");
	menu->back_callback = callback_menu_quit;
	
    for (i = 0; i < N_STATES; i++) {
        menu_entry = new_menu_entry(0);
        sprintf(buffer, "State %d", i);
        menu_entry_set_text(menu_entry, buffer);
        menu_add_entry(menu, menu_entry);
        menu_entry->callback = callback_selectedstate;
    }
    menu->selected_entry = gambatte_p->currentState();
    
	menu_main(menu);
    
    delete_menu(menu);
}

static void callback_selectedstate(menu_t *caller_menu) {
	gambatte_p->selectState(caller_menu->selected_entry);
	caller_menu->quit = 1;
}


/* ==================== OPTIONS MENU ================================ */
#define SHOW_FPS 0
#define SCALER 1
#define BACK 4

static void callback_options_back(menu_t *caller_menu);
static int is_showing_fps = 0;

static void callback_options(menu_t *caller_menu) {
    menu_t *menu;
	menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();
        
    menu_set_header(menu, "GCW-Gambatte (WIP)");
    menu_set_title(menu, "Options");
	menu->back_callback = callback_options_back;
	
    menu_entry = new_menu_entry(1); //0
    menu_entry_set_text(menu_entry, "Show FPS");
    menu_add_entry(menu, menu_entry);
    menu_entry_add_entry(menu_entry, "Off");
    menu_entry_add_entry(menu_entry, "On");    
    menu_entry->selected_entry = is_showing_fps;

    menu_entry = new_menu_entry(1); //1
    menu_entry_set_text(menu_entry, "Scaler");
    menu_add_entry(menu, menu_entry);
    menu_entry_add_entry(menu_entry, "None");
    menu_entry_add_entry(menu_entry, "Sw 1.5x");
    menu_entry_add_entry(menu_entry, "Sw Full");
    menu_entry_add_entry(menu_entry, "Hw 1.5x");
    menu_entry_add_entry(menu_entry, "Hw Aspect");
    menu_entry_add_entry(menu_entry, "Hw Full");
    menu_entry->selected_entry = selectedscaler;

    menu_entry = new_menu_entry(0); //2
    menu_entry_set_text(menu_entry, "DMG Palette"); // TEST FUNCTION
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_dmgpalette;

    menu_entry = new_menu_entry(0); //3
    menu_entry_set_text(menu_entry, "DMG Border"); // TEST FUNCTION
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_dmgborderimage;


    menu_entry = new_menu_entry(0); //4
    menu_entry_set_text(menu_entry, "Back");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_options_back;
    
	menu_main(menu);
    
    delete_menu(menu);
}

static void callback_options_back(menu_t *caller_menu) {
	is_showing_fps = caller_menu->entries[SHOW_FPS]->selected_entry;
    selectedscaler = caller_menu->entries[SCALER]->selected_entry;
    blitter_p->setScreenRes(); /* switch to selected resolution */
    caller_menu->quit = 1;
}

#undef SHOW_FPS
#undef SCALER
#undef BACK

void show_fps(SDL_Surface *surface, int fps) {
	char buffer[64];
	sprintf(buffer, "%d", fps);
	if (is_showing_fps) {
		SFont_Write(surface, font, 0, 0, buffer);
	}
}

/* ==================== DMG PALETTE MENU =========================== */

struct dirent **palettelist = NULL;
int numpalettes;

static void callback_nopalette(menu_t *caller_menu);
static void callback_selectedpalette(menu_t *caller_menu);
static void callback_dmgpalette_back(menu_t *caller_menu);
static int parse_ext_pal(const struct dirent *dir);

static void callback_dmgpalette(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, "GCW-Gambatte (WIP)");
    menu_set_title(menu, "DMG Palette");
    menu->back_callback = callback_dmgpalette_back;

    numpalettes = scandir("/usr/local/home/.gambatte/palettes", &palettelist, parse_ext_pal, alphasort);
    if (numpalettes <= 0) {
        printf("scandir for ./gambatte/palettes/ failed.");
    } else {
        for (int i = 2; i < numpalettes; ++i){ //first 2 entries are "." and ".." so we skip those.
            menu_entry = new_menu_entry(0);
            menu_entry_set_text_no_ext(menu_entry, palettelist[i]->d_name);
            menu_add_entry(menu, menu_entry);
            menu_entry->callback = callback_selectedpalette;
        }
    }
    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No palette");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_nopalette;

    menu->selected_entry = currentEntryInList(menu, palname); 
    
    menu_main(menu);
    delete_menu(menu);

    for (int i = 0; i < numpalettes; ++i){
        free(palettelist[i]);
    }
    free(palettelist);
}

/*static int currentPalette(menu_t *menu){
    int i = menu->n_entries;
    return i;
    

}*/

static int parse_ext_pal(const struct dirent *dir) {
    if(!dir)
        return 0;

    if(dir->d_type == DT_REG) {
        const char *ext = strrchr(dir->d_name,'.');
        if((!ext) || (ext == dir->d_name)) {
            return 0;
        } else {
            if(strcmp(ext, ".pal") == 0)
                return 1;
        }
    }
}

static void callback_nopalette(menu_t *caller_menu) {
    Uint32 value;
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 4; ++k) {
            if(k == 0)
                value = 0xF8FCF8;
            if(k == 1)
                value = 0xA8A8A8;
            if(k == 2)
                value = 0x505450;
            if(k == 3)
                value = 0x000000;
            gambatte_p->setDmgPaletteColor(i, k, value);
        }
    }
    set_menu_palette(0xF8FCF8, 0xA8A8A8, 0x505450, 0x000000);
    palname = "No palette";
    caller_menu->quit = 0;
}

static void callback_selectedpalette(menu_t *caller_menu) {
    Uint32 values[12];
    std::string fullfilepath = "/usr/local/home/.gambatte/palettes/";
    fullfilepath += palettelist[caller_menu->selected_entry + 2]->d_name; // we previously skipped 2 entries, so we have to do "+ 2" here.
    FILE * fpal;
    fpal = fopen(fullfilepath.c_str(), "r");
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
        printf("error reading: %s:\n",fullfilepath.c_str());
        printf("bad file format or file does not exist.\n");
    }

    palname = palettelist[caller_menu->selected_entry + 2]->d_name;
    palname = palname.substr(0, palname.size() - 4);

    caller_menu->quit = 0;
}

static void callback_dmgpalette_back(menu_t *caller_menu) {
    caller_menu->quit = 1;
}

/* ==================== DMG BORDER IMAGE MENU =========================== */

struct dirent **dmgborderlist = NULL;
int numdmgborders;

static void callback_nodmgborder(menu_t *caller_menu);
static void callback_selecteddmgborder(menu_t *caller_menu);
static void callback_dmgborderimage_back(menu_t *caller_menu);
static int parse_ext_png(const struct dirent *dir);

static void callback_dmgborderimage(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, "GCW-Gambatte (WIP)");
    menu_set_title(menu, "DMG Border");
    menu->back_callback = callback_dmgborderimage_back;

    numdmgborders = scandir("/usr/local/home/.gambatte/borders", &dmgborderlist, parse_ext_png, alphasort);
    if (numdmgborders <= 0) {
        printf("scandir for ./gambatte/borders/ failed.");
    } else {
        for (int i = 2; i < numdmgborders; ++i){ //first 2 entries are "." and ".." so we skip those.
            menu_entry = new_menu_entry(0);
            menu_entry_set_text_no_ext(menu_entry, dmgborderlist[i]->d_name);
            menu_add_entry(menu, menu_entry);
            menu_entry->callback = callback_selecteddmgborder;
        }
    }
    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No border");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_nodmgborder;

    menu->selected_entry = currentEntryInList(menu, dmgbordername); 
    
    menu_main(menu);
    delete_menu(menu);

    for (int i = 0; i < numdmgborders; ++i){
        free(dmgborderlist[i]);
    }
    free(dmgborderlist);
}

static int parse_ext_png(const struct dirent *dir) {
    if(!dir)
        return 0;

    if(dir->d_type == DT_REG) {
        const char *ext = strrchr(dir->d_name,'.');
        if((!ext) || (ext == dir->d_name)) {
            return 0;
        } else {
            if(strcmp(ext, ".png") == 0)
                return 1;
        }
    }
}

static void callback_nodmgborder(menu_t *caller_menu) {
    dmgbordername = "NONE";
    load_border(dmgbordername);
    caller_menu->quit = 0;
}

static void callback_selecteddmgborder(menu_t *caller_menu) {
    dmgbordername = dmgborderlist[caller_menu->selected_entry + 2]->d_name;
    dmgbordername = dmgbordername.substr(0, dmgbordername.size() - 4);
    load_border(dmgbordername + ".png");
    caller_menu->quit = 0;
}

static void callback_dmgborderimage_back(menu_t *caller_menu) {
    caller_menu->quit = 1;
}


