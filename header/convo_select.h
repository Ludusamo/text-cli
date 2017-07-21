#ifndef CONVO_SELECT_H
#define CONVO_SELECT_H

#include <ncurses.h>
#include <menu.h>

typedef struct {
	WINDOW *win, *subwin;
	MENU *menu;
	ITEM **items;
	int width, height;
	int num_convo;
} Convo_Select;

void select_convo(char *name);
void init_convo_select(Convo_Select *cs);
void deinit_convo_select(Convo_Select *cs);
void render_convo_select(const Convo_Select *cs);
void update_convo_select(Convo_Select *cs);

void add_convo(Convo_Select *cs, const char *contact, const char *num_msg);
void print_in_middle(WINDOW *win, int starty, int startx,
                     int width, char *string, chtype color);

#endif // CONVO_SELECT_H
