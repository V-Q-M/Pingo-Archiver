#ifndef UI_H
#define UI_H

#define MAX_LINES 1000

void initColors();
void showStatus(const char *msg, int duration);
void showOutput();
int promptDirectory(const char *title, const char *description, const char *prompt);
int confirmBurn();

extern char musicDir[256];
extern char output[MAX_LINES][256];
extern char baseMusicDir[256]; 
extern int line_count;

#endif