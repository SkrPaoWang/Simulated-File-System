/* This program simulates a file system within an actual file.  The
 * simulated file system has only one directory, and two types of
 * metadata structures defined in simfstypes.h.
 */

/* Simfs is run as:
 * simfs -f myfs command args
 * where the -f option specifies the actual file that the simulated file system
 * occupies, the command is the command to be run on the simulated file system,
 * and a list of arguments for the command is represented by args.  Note that
 * different commands take different numbers of arguments.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"
#include <ctype.h>

// We using the ops array to match the file system command entered by the user.
#define MAXOPS  6
char *ops[MAXOPS] = {"initfs", "printfs", "createfile", "readfile",
                     "writefile", "deletefile"};
int find_command(char *);
void checkisnumber(char*);

char filedata[BLOCKSIZE * MAXBLOCKS];
char line[BLOCKSIZE * MAXBLOCKS];
int
main(int argc, char **argv)
{
    
    int oc;       /* option character */
    char *cmd;    /* command to run on the file system */
    char *fsname; /* name of the simulated file system file */
    char *arg1;
    char *arg2;
    char *arg3;
    
    char *usage_string = "Usage: simfs -f file cmd arg1 arg2 ...\n";
    /* Get and check the arguments */
    if(argc < 4) {
        fputs(usage_string, stderr);
        exit(1);
    }
    while((oc = getopt(argc, argv, "f:")) != -1) {
        switch(oc) {
        case 'f' :
            fsname = optarg;
            break;
        default:
            fputs(usage_string, stderr);
            exit(1);
        }
    }
    if ((sizeof(fentry)*MAXFILES + sizeof(fnode) *MAXBLOCKS) > BLOCKSIZE*MAXBLOCKS){
        fprintf(stderr, "Error: overflow, metadata can not fit into block\n");
        exit(1);
    }

    /* Get the command name */
    cmd = argv[optind];
    optind++;
    if (argc >= 5){
        arg1 = argv[optind];
    }
     if (argc >= 6){
         optind++;
          arg2 = argv[optind];
     }
     if (argc >= 7){
         optind++;
         arg3 = argv[optind];
     }

    char* end1;
    int start1;
    int length1;
    switch((find_command(cmd))) {
    case 0: /* initfs */
        initfs(fsname);
        break;
    case 1: /* printfs */
        printfs(fsname);
        break;
    case 2: /* createfile */
        if (argc != 5){
            fprintf(stderr, "Error: createfile command  takes one argument\n");
            exit(1);
        }
        if (strlen(argv[3])!=10){
            fprintf(stderr, "Error: You put more characters in word createfile, the cmd is createfile\n");
            exit(1);
        }
        createfile(fsname,arg1);
        break;
    case 3: /* readfile */
        if (argc != 7){
            fprintf(stderr, "Error: readfile command takes three argument\n");
            exit(1);
        }
        if (strlen(argv[3])!=8){
            fprintf(stderr, "Error: You put more characters in word readfile, the cmd is readfile\n");
            exit(1);
        }
        checkisnumber(arg2);
        checkisnumber(arg3);
        start1 =  (int)strtol(arg2, &end1, 10);
        length1 =  (int)strtol(arg3, &end1, 10);
        if (start1 < 0 || length1 < 0){
            fprintf(stderr, "Error: start or length should be positive\n");
            exit(1);
        }
        readfile(fsname,arg1,start1,length1);
        break;
    case 4: /* writefile */
        if (argc != 7){
            fprintf(stderr, "Error: writefile command takes three argument\n");
            exit(1);
        }
        if (strlen(argv[3])!=9){
            fprintf(stderr, "Error: You put more characters in word writefile, the cmd is writefile\n");
            exit(1);
        }
        checkisnumber(arg2);
        checkisnumber(arg3);
        while (fgets(line, BLOCKSIZE * MAXBLOCKS, stdin)) {
            strncat(filedata,line,strlen(line));
        }
        char* end;
        int offset =  (int)strtol(arg2, &end, 10);
        int length =  (int)strtol(arg3, &end, 10);
        if (offset < 0 || length < 0){
            fprintf(stderr, "Error: start or length should be positive\n");
            exit(1);
        }
        writefile(fsname,arg1,offset,length,filedata);
        break;
    case 5: /* deletefile */
        if (strlen(argv[3])!=10){
            fprintf(stderr, "Error: You put more characters in word deletefile,the cmd is deletefile\n");
            exit(1);
        }
        if (argc != 5){
            fprintf(stderr, "Error: deletefile command takes one argument\n");
            exit(1);
        }
        deletefile(fsname,arg1);
        break;
    default:
        fprintf(stderr, "Error: Invalid command\n");
        exit(1);
    }

    return 0;
}

/* Returns a integer corresponding to the file system command that
 * is to be executed
 */
int
find_command(char *cmd)
{
    int i;
    for(i = 0; i < MAXOPS; i++) {
        if ((strncmp(cmd, ops[i], strlen(ops[i]))) == 0) {
            return i;
        }
    }
    fprintf(stderr, "Error: Command %s not found\n", cmd);
    return -1;
}

void checkisnumber(char* input){
    for (int i=0;i<strlen(input); i++){
        // if(input[i] >= '0' && input[i] <= '9'){
        //     continue;
        // }else{
        //     fprintf(stderr, "Error: start or length parameter is not a number\n");
        //     exit(1);

        // }
             
        if (!isdigit(input[i])){
            fprintf(stderr, "Error: start or length parameter is not a number\n");
            exit(1);
        }
    }
}
