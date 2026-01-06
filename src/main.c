#include "ui.h"
#include "commands.h"

#include <ncurses.h>

#define MAX_LINES 1000

char output[MAX_LINES][256];
int line_count = 0;
char baseMusicDir[256] = "/home/vito/Music"; // base music directory
char musicDir[256]; // actual directory for current operation

// The main and start menu
void menuLoop() {
    int choice = 0;
    int ch;

    const char *options[] = { "Backup CD", "Restore CD" , "Extract CD", "Burn CD"};
    const int optionCount = 4;

    int win_h = 13;
    int win_w = 42;
    int win_y = (LINES - win_h) / 2;
    int win_x = (COLS - win_w) / 2;

    WINDOW *menu = newwin(win_h, win_w, win_y, win_x);
    keypad(menu, TRUE);

    while (1) {
        werase(menu);
        box(menu, 0, 0);

        /* Title */
        wattron(menu, A_BOLD);
        mvwprintw(menu, 1, (win_w - 15) / 2, "PINGO ARCHIVER");
        wattroff(menu, A_BOLD);

        /* Separator */
        mvwhline(menu, 2, 1, ACS_HLINE, win_w - 2);

        /* Help text */
        mvwprintw(menu, 3, 2, "J-K Navigate   ENTER Select   Q Quit");

        /* Menu options */
        for (int i = 0; i < optionCount; i++) {
            int y = 6 + i;

            if (i == choice) {
                wattron(menu, COLOR_PAIR(1) | A_REVERSE | A_BOLD);
                mvwprintw(menu, y, 4, "%-*s", win_w - 8, options[i]);
                wattroff(menu, COLOR_PAIR(1) | A_REVERSE | A_BOLD);
            } else {
                wattron(menu, COLOR_PAIR(2));
                mvwprintw(menu, y, 4, "%-*s", win_w - 8, options[i]);
                wattroff(menu, COLOR_PAIR(2));
            }
        }

        wrefresh(menu);
        ch = wgetch(menu);

        switch (ch) {
            case KEY_UP:
            case 'k':
                choice = (choice - 1 + optionCount) % optionCount;
                break;

            case KEY_DOWN:
            case 'j':
                choice = (choice + 1) % optionCount;
                break;

            case '\n':
                delwin(menu);
                clear();
                box(stdscr, 0, 0);

                line_count = 0;

                if (choice == 0) {
                    if (backupDirectory() == 0){
                        backupDirectory();
                        showStatus("Reading CD...",1);
                        refresh();
                        backupCD();
                    }
                } else {
                    if (confirmBurn()) {
                        if (restoreDirectory() == 0) {
                            showStatus("Burning CD...", 1);
                            refresh();
                            restoreCD();
                        } else {
                            showStatus("Error burning CD!",1);
                        }
                    } else {
                        showStatus("Burn cancelled.",1);
                    }
                }

                //showOutput();
                showStatus("Returning to main menu...",1);
                //getch();

                clear();
                refresh();

                menu = newwin(win_h, win_w, win_y, win_x);
                keypad(menu, TRUE);
                break;

            case 'q':
            case 'Q':
                delwin(menu);
                return;
        }
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    initColors(); // Initialize color pairs

    menuLoop(); // start menu

    endwin();
    return 0;
}

