#include "fsHelpers.h"

#include <sys/stat.h>
#include <errno.h>
#include <string.h>


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