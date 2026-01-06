#include "commands.h"
#include "ui.h"
#include "fsHelpers.h"

#include <stdio.h>     
#include <string.h>   
#include <sys/stat.h>
#include <errno.h> 
#include <unistd.h>
#include <ncurses.h>

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


// Ask the user for a subdirectory to burn from
int burnDirectory() {
    // Prompt user into a temporary buffer
    char userInput[256] = "";
    promptDirectory(
        "Burn CD",
        "Enter album folder name: (e.g. The_Beatles/Disc01)"
    );

    // Copy & clean input from musicDir (where promptDirectory wrote)
    strncpy(userInput, musicDir, sizeof(userInput)-1);
    userInput[sizeof(userInput)-1] = '\0';

    // Trim trailing spaces and newlines
    size_t len = strlen(userInput);
    while (len > 0 && (userInput[len-1] == '\n' || userInput[len-1] == ' ')) {
        userInput[len-1] = '\0';
        len--;
    }

    // Build the final full path
    if (len > 0) {
        snprintf(musicDir, sizeof(musicDir), "%s/%s", baseMusicDir, userInput);
    } else {
        strncpy(musicDir, baseMusicDir, sizeof(musicDir)-1);
        musicDir[sizeof(musicDir)-1] = '\0';
    }

    // Check if directory exists
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
             "cdrdao write %s/disc.toc && eject",
             musicDir);
    runCommand(cmd);
}