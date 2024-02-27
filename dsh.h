#define MAXBUF 256  // max number of characteres allowed on command line

// Function declarations
void shell(void);

void commandMode1(char *cmd, char **list, int run);
void commandMode2(char *cmd, char **list);

void runForeground(char *line, char **list);
void runBackground(char *line, char **list);

char *trim(char *str);
void changeDirectory(char *path, int pathGiven);