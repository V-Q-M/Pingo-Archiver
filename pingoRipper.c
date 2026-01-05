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

// Run a shell command and store output in the buffer
void runCommand(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        snprintf(output[line_count++], 256, "Failed to run command.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL && line_count < MAX_LINES) {
        line[strcspn(line, "\n")] = 0; // remove newline
        strncpy(output[line_count++], line, 255);
    }

    pclose(fp);
}

// Show output in a scrollable boxed window
void showOutput() {
    int start = 0;
    int ch;

    while (1) {
        clear();
        box(stdscr, 0, 0);
        mvprintw(0, 2, " Command Output (UP/DOWN to scroll, Q to quit) ");
        int rows = LINES - 4; // leave space for border
        for (int i = 0; i < rows && (start + i) < line_count; i++) {
            mvprintw(i + 2, 2, "%s", output[start + i]);
        }
        refresh();

        ch = getch();
        if (ch == KEY_UP && start > 0) start--;
        else if (ch == KEY_DOWN && start + rows < line_count) start++;
        else if (ch == 'q' || ch == 'Q') break;
    }
}

// Ask the user for a subdirectory to rip into, create it
void ripDirectory() {
    char subDir[100] = "";
    echo();
    curs_set(1);

    clear();
    box(stdscr, 0, 0);
    mvprintw(2, 5, "Specify subdirectory to rip into (ENTER for none): ");
    move(3, 5);
    getnstr(subDir, 99);

    if (strlen(subDir) > 0) {
        snprintf(musicDir, sizeof(musicDir), "%s/%s", baseMusicDir, subDir);
    } else {
        strncpy(musicDir, baseMusicDir, sizeof(musicDir) - 1);
        musicDir[sizeof(musicDir) - 1] = '\0';
    }

    // Create directory if it doesn't exist
    if (mkdir(musicDir, 0755) != 0) {
        if (errno != EEXIST) {
            mvprintw(LINES/2, 5, "Error creating directory: %s", strerror(errno));
            getch();
        }
    }

    noecho();
    curs_set(0);
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
    clear();
    box(stdscr, 0, 0);
    mvprintw(LINES/2, (COLS - 35)/2, "Are you sure you want to burn the CD? (y/n)");
    refresh();
    int ch;
    while (1) {
        ch = getch();
        if (ch == 'y' || ch == 'Y') return 1;
        else if (ch == 'n' || ch == 'N') return 0;
    }
}

// Run the CD burning command
void burnCD() {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "cdrdao write-cd %s/disc.toc && eject",
             musicDir);
    runCommand(cmd);
}

// Centered menu loop with border and colors
void menuLoop() {
    int choice = 0;
    int ch;
    const char *options[] = {"Rip CD", "Burn CD"};

    while (1) {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, (COLS - 25)/2, "===== PINGO RIPPER =====");
        mvprintw(2, (COLS - 40)/2, "Use UP/DOWN arrows to navigate, Q to quit");

        for (int i = 0; i < 2; i++) {
            if (i == choice) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(5 + i, (COLS - 10)/2, "%s", options[i]);
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                attron(COLOR_PAIR(2));
                mvprintw(5 + i, (COLS - 10)/2, "%s", options[i]);
                attroff(COLOR_PAIR(2));
            }
        }

        refresh();

        ch = getch();
        if (ch == KEY_UP || ch == KEY_DOWN) choice = 1 - choice;
        else if (ch == '\n') {
            line_count = 0;
            if (choice == 0) {
                ripDirectory();
                clear();
                box(stdscr, 0, 0);
                mvprintw(2, 5, "Ripping CD into %s...", musicDir);
                refresh();
                ripCD();
            } else {
                if (confirmBurn()) {
                    clear();
                    box(stdscr, 0, 0);
                    mvprintw(2, 5, "Burning CD from %s...", musicDir);
                    refresh();
                    burnCD();
                } else {
                    clear();
                    box(stdscr, 0, 0);
                    mvprintw(2, 5, "Burn cancelled.");
                    refresh();
                }
            }

            showOutput();
            mvprintw(LINES-2, 5, "Press any key to return to the menu...");
            getch();
        }
        else if (ch == 'q' || ch == 'Q') break;
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

