#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_LINES 1000

char output[MAX_LINES][256];
int line_count = 0;
char musicDir[256] = "/home/vito/Music"; // base music directory

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

// Show output in a scrollable ncurses window
void showOutput() {
    int start = 0;
    int ch;
    while (1) {
        clear();
        mvprintw(0, 0, "Command Output (UP/DOWN to scroll, Q to quit):");
        int rows = LINES - 2;
        for (int i = 0; i < rows && (start + i) < line_count; i++) {
            mvprintw(i + 1, 2, "%s", output[start + i]);
        }
        refresh();

        ch = getch();
        if (ch == KEY_UP && start > 0) start--;
        else if (ch == KEY_DOWN && start + rows < line_count) start++;
        else if (ch == 'q' || ch == 'Q') break;
    }
}

// Ask the user for a subdirectory to rip into and create it
void ripDirectory() {
    char subDir[100] = "";
    echo();
    curs_set(1);

    clear();
    mvprintw(2, 5, "Specify subdirectory to rip into (ENTER for none): ");
    //move(LINES/2 + 1, 5);
    getnstr(subDir, 99);

    if (strlen(subDir) > 0) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", musicDir, subDir);

        // Create directory if it doesn't exist
        if (mkdir(fullPath, 0755) != 0) {
            if (errno != EEXIST) {
                mvprintw(LINES/2 + 3, 5, "Error creating directory: %s", strerror(errno));
                getch();
            }
        }

        strncpy(musicDir, fullPath, sizeof(musicDir) - 1);
        musicDir[sizeof(musicDir) - 1] = '\0';
    }

    noecho();
    curs_set(0);
}

// Run the CD ripping command
void ripCD() {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "cdrdao read-cd --read-raw --datafile %s/disc.bin %s/disc.toc",
             musicDir, musicDir);
    runCommand(cmd);
}

// Ask for Y/N confirmation before burning
int confirmBurn() {
    clear();
    mvprintw(LINES/2, 10, "Are you sure you want to burn the CD? (y/n)");
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
             "cdrdao write-cd %s/disc.toc",
             musicDir);
    runCommand(cmd);
}

int main() {
    int choice = 0;
    int ch;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Menu selection
    while (1) {
        clear();
        mvprintw(2, 5, "Use UP/DOWN to select, ENTER to run, Q to quit.");
        if (choice == 0) {
            attron(A_REVERSE); mvprintw(5, 10, "Rip CD"); attroff(A_REVERSE);
            mvprintw(6, 10, "Burn CD");
        } else {
            mvprintw(5, 10, "Rip CD");
            attron(A_REVERSE); mvprintw(6, 10, "Burn CD"); attroff(A_REVERSE);
        }
        refresh();

        ch = getch();
        if (ch == KEY_UP || ch == KEY_DOWN) choice = 1 - choice;
        else if (ch == '\n') break;
        else if (ch == 'q' || ch == 'Q') {
            endwin();
            return 0;
        }
    }

    line_count = 0;
    if (choice == 0) {
        ripDirectory(); // Ask for subdirectory before ripping
        mvprintw(2, 5, "Ripping CD into %s...", musicDir);
        refresh();
        ripCD();
    } else {
        if (confirmBurn()) {
            mvprintw(2, 5, "Burning CD from %s...", musicDir);
            refresh();
            burnCD();
        } else {
            mvprintw(2, 5, "Burn cancelled.");
            refresh();
        }
    }

    showOutput();
    endwin();
    return 0;
}

