#include <stdio.h>
#include "simfstypes.h"

/* File system operations */
void printfs(char *);
void initfs(char *);
void createfile(char *, char *);
void writefile(char *,char *,int,int,char *);
void readfile(char *, char *, int,int);
void deletefile(char *, char *);
/* Internal functions */
FILE *openfs(char *filename, char *mode);
void closefs(FILE *fp);
