#include "convo_select.h"
#include <stdlib.h>
#include <string.h>

void init_convo_select(Convo_Select *cs) {
	cs->num_convo = 1;
	cs->items = calloc(1, sizeof(ITEM *));
	cs->width = strlen("Conversations") + 2;
	cs->height = LINES - 2;

	cs->menu = new_menu((ITEM **) cs->items);
	cs->win = newwin(cs->height, cs->width, 0, 4);
	keypad(cs->win, TRUE);

	set_menu_win(cs->menu, cs->win);
	cs->subwin = derwin(cs->win, 0, 0, 3, 1);
	set_menu_sub(cs->menu, cs->subwin);

	set_menu_mark(cs->menu, " * ");

	post_menu(cs->menu);
	wrefresh(cs->win);
}

void deinit_convo_select(Convo_Select *cs) {
	unpost_menu(cs->menu);
	free_menu(cs->menu);
	for (int i = 0; i < cs->num_convo; i++) free_item(cs->items[i]);
	free(cs->items);
}

void render_convo_select(const Convo_Select *cs) {
	box(cs->win, 0, 0);
	print_in_middle(cs->win, 1, 0, cs->width, "Conversations", COLOR_PAIR(1));
	mvwaddch(cs->win, 2, 0, ACS_LTEE);
	mvwhline(cs->win, 2, 1, ACS_HLINE, cs->width - 2);
	mvwaddch(cs->win, 2, cs->width - 1, ACS_RTEE);
	mvprintw(LINES - 2, 0, "F1 to exit");
	refresh();
	wrefresh(cs->win);
}

void update_convo_select(Convo_Select *cs) {
	int ch;
	while ((ch = wgetch(cs->win)) != KEY_F(1)) {
		switch (ch) {
			case 'j':
				menu_driver(cs->menu, REQ_DOWN_ITEM);
				break;
			case 'k':
				menu_driver(cs->menu, REQ_UP_ITEM);
				break;
		}
		render_convo_select(cs);
	}
}

void add_convo(Convo_Select *cs, const char *contact, const char *num_msg) {
	unpost_menu(cs->menu);
	cs->items = realloc(cs->items, ++cs->num_convo);
	cs->items[cs->num_convo - 2] = new_item(contact, num_msg);
	cs->items[cs->num_convo - 1] = 0;
	set_menu_items(cs->menu, cs->items);
	int width_row = strlen(contact) + strlen(num_msg) + 10;
	if (cs->width < width_row) cs->width = width_row;
	wresize(cs->subwin, cs->height, cs->width - 2);
	wresize(cs->win, LINES - 2, cs->width);
	post_menu(cs->menu);
}

void print_in_middle(WINDOW *win, int starty, int startx,
                     int width, char *string, chtype color) {
	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}
