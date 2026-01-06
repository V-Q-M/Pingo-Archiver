#ifndef UI_H
#define UI_H

#define MAX_LINES 1000

void initColors();
void showStatus(const char *msg);
void showOutput();
void promptDirectory(const char *title, const char *prompt);
int confirmBurn();

extern char musicDir[256];
extern char output[MAX_LINES][256];
extern char baseMusicDir[256]; 
extern int line_count;

#endif