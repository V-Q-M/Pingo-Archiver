#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#define MAX_LINES 1000

char output[MAX_LINES][256];
int line_count = 0;
char baseMusicDir[256] = "/home/vito/Music"; // base music directory
char musicDir[256]; // actual directory for current operation

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


// Run a shell command and store output in the buffer
void runCommand(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        if (line_count < MAX_LINES) {
            snprintf(output[line_count++], 256, "Failed to run command.");
        }
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL && line_count < MAX_LINES) {
        line[strcspn(line, "\n")] = 0; // remove newline
        strncpy(output[line_count++], line, 255);
    }

    pclose(fp);
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


void promptDirectory(const char *title, const char *prompt) {
    int win_h = 9;
    int win_w = 60;
    int win_y = (LINES - win_h) / 2;
    int win_x = (COLS - win_w) / 2;

    WINDOW *win = newwin(win_h, win_w, win_y, win_x);
    box(win, 0, 0);

    echo();
    curs_set(1);

    wattron(win, A_BOLD);
    mvwprintw(win, 1, (win_w - strlen(title)) / 2, "%s", title);
    wattroff(win, A_BOLD);

    mvwhline(win, 2, 1, ACS_HLINE, win_w - 2);
    mvwprintw(win, 3, 2, "%s", prompt);
    mvwprintw(win, 5, 2, "> ");

    wrefresh(win);
    wgetnstr(win, musicDir, 255);

    noecho();
    curs_set(0);
    delwin(win);
}


int mkdir_recursive(const char *path, mode_t mode) {
    char tmp[512];
    char *p;
    size_t len;

    if (!path || !*path) return -1;

    strncpy(tmp, path, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    len = strlen(tmp);

    // Remove trailing slash
    if (tmp[len-1] == '/') tmp[len-1] = 0;

    // Iterate through path components
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, mode) != 0) {
                if (errno != EEXIST) return -1;
            }
            *p = '/';
        }
    }

    // Create the final directory
    if (mkdir(tmp, mode) != 0) {
        if (errno != EEXIST) return -1;
    }

    return 0;
}



// Ask the user for a subdirectory to rip into, create it
void ripDirectory() {
    promptDirectory(
        "Rip CD",
        "Enter album folder name: (e.g. The_Beatles/Disc01)"
    );

    char finalPath[256];

    if (strlen(musicDir) > 0) {
        // User entered something → concatenate
        snprintf(finalPath, sizeof(finalPath), "%s/%s", baseMusicDir, musicDir);
    } else {
        // User pressed ENTER → use base directory
        strncpy(finalPath, baseMusicDir, sizeof(finalPath)-1);
        finalPath[sizeof(finalPath)-1] = '\0';
    }

    // Copy back to musicDir safely
    strncpy(musicDir, finalPath, sizeof(musicDir)-1);
    musicDir[sizeof(musicDir)-1] = '\0';

    // Create directory if it doesn't exist
    if (mkdir_recursive(musicDir, 0755) != 0) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Error creating directory: %s", strerror(errno));
    showStatus(msg);
    getch();
}

}


// Run the CD ripping command
void ripCD() {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "cdrdao read-cd --read-raw --datafile %s/disc.bin %s/disc.toc && eject",
             musicDir, musicDir);
    runCommand(cmd);

    // Make the subdirectory read-only after ripping
    chmod(musicDir, 0555);
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


// Ask the user for a subdirectory to burn from
int burnDirectory() {
    promptDirectory(
        "Burn CD",
        "Enter album folder name: (e.g. The_Beatles/Disc01)"
    );

    if (strlen(musicDir) > 0) {
        snprintf(musicDir, sizeof(musicDir), "%s/%s", baseMusicDir, musicDir);
    } else {
        strncpy(musicDir, baseMusicDir, sizeof(musicDir));
    }

    struct stat st;
    if (stat(musicDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        showStatus("Directory does not exist.");
        getch();
        return 1;
    }

    return 0;
}


// Run the CD burning command
void burnCD() {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "cdrdao write-cd %s/disc.toc && eject",
             musicDir);
    runCommand(cmd);
}


void menuLoop() {
    int choice = 0;
    int ch;

    const char *options[] = { "Rip CD", "Burn CD" };
    const int optionCount = 2;

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
                    ripDirectory();
                    showStatus("Ripping CD...");
                    refresh();
                    ripCD();
                } else {
                    if (confirmBurn()) {
                        if (burnDirectory() == 0) {
                            //mvprintw(2, 5, "Burning CD from %s...", musicDir);
                            showStatus("Burning CD...");
                            refresh();
                            burnCD();
                        } else {
                            //mvprintw(2, 5, "Error burning CD...");
                            showStatus("Error burning CD");
                        }
                    } else {
                        mvprintw(2, 5, "Burn cancelled.");
                    }
                }

                //showOutput();
                showStatus("Press any key to return to the menu...");
                getch();

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

