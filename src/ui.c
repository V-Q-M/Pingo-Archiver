#include "ui.h"
#include <ncurses.h>
#include <string.h>
#include <ctype.h>


// Initialize colors
void initColors() {
    if (!has_colors()) return;
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_CYAN);  // Highlighted menu option
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // Normal text
}

void showStatus(const char *msg) {
    int win_h = 5;
    int win_w = COLS - 10;
    int win_y = (LINES - win_h) / 2;
    int win_x = 5;

    WINDOW *win = newwin(win_h, win_w, win_y, win_x);
    box(win, 0, 0);

    wattron(win, A_BOLD);
//    mvwprintw(win, 2, 2, "%s", msg);
    mvwprintw(win, 2, (win_w - strlen(msg)) / 2, "%s", msg);

    wattroff(win, A_BOLD);

    wrefresh(win);
    delwin(win);
}

void showOutput() {
    int start = 0;
    int ch;
    int rows = LINES - 4;

    while (1) {
        clear();
        box(stdscr, 0, 0);

        attron(A_BOLD);
        mvprintw(0, 2, " Command Output (UP/DOWN to scroll, Q to quit) ");
        attroff(A_BOLD);

        for (int i = 0; i < rows && (start + i) < line_count; i++) {
            mvprintw(2 + i, 2, "%s", output[start + i]);
        }

        refresh();

        ch = getch();
        if (ch == KEY_UP || ch == 'k') {
            if (start > 0) start--;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (start + rows < line_count) start++;
        } else if (ch == 'q' || ch == 'Q') {
            clear();
            refresh();
            return;
        }
    }
}


void inputLoop(WINDOW* win, int win_w){
    char input[256] = "";
    int pos = 0;
    int ch;

    while(1){
        ch= wgetch(win);

        if (ch=='\n') {
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                input[pos] = '\0';
            }
        } else if (isprint(ch) && pos < sizeof(input)-1){
            if (ch == ' '){
                ch = '_'; // Replace space with underscore
            }
            input[pos++] = ch;
            input[pos] = '\0'; // Adjust for null terminator
        }

        // redraw inputline
        mvwprintw(win, 5, 2, "> %-*s", win_w-4, input); 

        if (pos >= 3){

        }

        wrefresh(win);
    }
    strncpy(musicDir, input, sizeof(musicDir)-1);
    musicDir[sizeof(musicDir)-1] = '\0';
}

void promptDirectory(const char *title, const char *prompt) {
    int win_h = 9;
    int win_w = 60;
    int win_y = (LINES - win_h) / 2;
    int win_x = (COLS - win_w) / 2;

    WINDOW *win = newwin(win_h, win_w, win_y, win_x);
    box(win, 0, 0);

    //echo();
    //curs_set(1);

    wattron(win, A_BOLD);
    mvwprintw(win, 1, (win_w - strlen(title)) / 2, "%s", title);
    wattroff(win, A_BOLD);

    mvwhline(win, 2, 1, ACS_HLINE, win_w - 2);
    mvwprintw(win, 3, 2, "%s", prompt);
    mvwprintw(win, 5, 2, "> ");

    wrefresh(win);
    // detect keypresses and "type" them like echo
    // after typing 3 letters get recommendations from the baseMusicDir
    // display 4 or 5 of these recommendations below


//    wgetnstr(win, musicDir, 255);
    inputLoop(win, win_w);

    //noecho();
    //curs_set(0);
    delwin(win);
}

// Ask for Y/N confirmation before burning
int confirmBurn() {
    int win_h = 7;
    int win_w = 44;
    int win_y = (LINES - win_h) / 2;
    int win_x = (COLS - win_w) / 2;

    WINDOW *win = newwin(win_h, win_w, win_y, win_x);
    keypad(win, TRUE);

    while (1) {
        werase(win);
        box(win, 0, 0);

        wattron(win, A_BOLD);
        mvwprintw(win, 1, (win_w - 20) / 2, "Confirm Burn");
        wattroff(win, A_BOLD);

        mvwhline(win, 2, 1, ACS_HLINE, win_w - 2);
        mvwprintw(win, 3, 2, "Are you sure you want to burn a CD?");
        mvwprintw(win, 5, 2, "Y = Yes    N = No");

        wrefresh(win);

        int ch = wgetch(win);
        if (ch == 'y' || ch == 'Y') {
            delwin(win);
            return 1;
        }
        if (ch == 'n' || ch == 'N') {
            delwin(win);
            return 0;
        }
    }
}
