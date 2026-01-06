#ifndef CD_H
#define CD_H

#define MAX_LINES 1000

void runCommand(const char *cmd);
void ripDirectory();
void ripCD();
int burnDirectory();
void burnCD();

extern char musicDir[256];
extern char baseMusicDir[256]; 
extern char output[MAX_LINES][256];
extern int line_count;

#endif