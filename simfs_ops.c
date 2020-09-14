/* This file contains functions that are not part of the visible "interface".
 * They are essentially helper functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"

/* Internal helper functions first.
 */

FILE *
openfs(char *filename, char *mode)
{
    FILE *fp;
    if((fp = fopen(filename, mode)) == NULL) {
        perror("openfs");
        exit(1);
    }
    return fp;
}

void
closefs(FILE *fp)
{
    if(fclose(fp) != 0) {
        perror("closefs");
        exit(1);
    }
}
fentry files[MAXFILES];
fnode fnodes[MAXBLOCKS];


void storemetadata(char *simfs){
    FILE *fp = openfs(simfs, "r");
    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0 ||
     fread(fnodes, sizeof(fnode), MAXBLOCKS, fp) == 0) {
        fprintf(stderr, "Error: could not read file entries or fnodes\n");
        closefs(fp);
        exit(1);
    }
    for (int i = 0; i < MAXFILES; i++){
        if (strlen(files[i].name) > 11){
            fprintf(stderr, "Error: Bad fs for invald fentry name\n");
            closefs(fp);
            exit(1);
        }
         if (files[i].firstblock < -1 || files[i].firstblock >= MAXBLOCKS){
            fprintf(stderr, "Error: Bad fs for invald fentry firstblock\n");
            closefs(fp);
            exit(1);
        }
    }
    for (int i = 0; i < MAXBLOCKS; i++){
        if (fnodes[i].blockindex != i && fnodes[i].blockindex != -i){
            fprintf(stderr, "Error: The file system you provide is invalid\n");
            closefs(fp);
            exit(1);
        }
        if (fnodes[i].nextblock < -1 || fnodes[i].nextblock >= MAXBLOCKS){
            fprintf(stderr, "Error: The file system you provide is invalid in second test\n");
            closefs(fp);
            exit(1);
        }

    }
    closefs(fp);
}

void writemetadata(char *simfs){
    FILE *fp = openfs(simfs, "r+b");
    if (fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES ||
    fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on change fnode or fentry attribute\n");
        closefs(fp);
        exit(1);
    }
    char zerobuf[BLOCKSIZE] = {0};
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    if (bytes_to_write != 0  && fwrite(zerobuf, bytes_to_write, 1, fp) < 1) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }
    closefs(fp);
}

void createfile(char *simfs,char* file){
    if (strlen(file)>= 12){
        fprintf(stderr, "Error:  maximum length of filename is 11\n");
        exit(1);
    }
    if (strlen(file)== 0){
        fprintf(stderr, "Error: file name can not have zero length\n");
        exit(1);
    }
    storemetadata(simfs);
    for (int i = 0; i <= MAXFILES; i++) {
        if (i == MAXFILES){
            fprintf(stderr, "Error: maximum number of files have been occupied\n");
            exit(1);
        }
        if (strncmp(files[i].name, "",11) == 0){
            strncpy(files[i].name,file,11);
            break;
        }
        if (strncmp(files[i].name, file,11) == 0){
            fprintf(stderr, "Error:Can not have same filename\n");
            exit(1);
        }      
    }
    writemetadata(simfs);
}

short getfirsnode(){
    for (short i = 0; i <= MAXBLOCKS; i++) {
        if (fnodes[i].blockindex < 0){
            return i;
        }
    }
    return -1;
}

int getnegativenextblock(int i){
    fnode current = fnodes[i];
    while (current.nextblock != -1){
        current = fnodes[current.nextblock];
    }
    if (current.blockindex < 0){
        return - current.blockindex;
    }
    return current.blockindex;
}

int skipstart(char* simfs, int i, int start){
    char buffer1[1];
    FILE * fp = openfs(simfs,"r");
    int counter1 = 0;
    int block1 = 1;
    fnode current1 = fnodes[i];
    while (counter1 <= start){
        if (counter1 == block1 *BLOCKSIZE){
            block1 += 1;
            current1 = fnodes[current1.nextblock];
        }
        counter1 += fread(buffer1,sizeof(char),1,fp);
    }
    closefs(fp);
    return current1.blockindex;
}

void checkblockoverflow(int blockneeded){
    int count = 0;
    for (int i = 0; i < MAXBLOCKS; i++){
        if (fnodes[i].blockindex < 0){
            count += 1;
        }
    }
    if (blockneeded > count){
        fprintf(stderr, "Error: blockoverflow: the data you want to write exceed the lefting data block size\n");
        exit(1);
    }
}

void initializeblock(char *simfs, int i, int start, int length){
    float blockoccupy = (float)files[i].size / (float)BLOCKSIZE;
    int realblockoccupy = blockoccupy == (int)blockoccupy ? blockoccupy: (int)blockoccupy + 1;
    int total = realblockoccupy * BLOCKSIZE;
    int blocksizeneeded = start + length;
    float blockneeded = (float)blocksizeneeded / (float)BLOCKSIZE;
    int realblockneeded = blockneeded == (int)blockneeded ? blockneeded : (int)blockneeded + 1;
    realblockneeded = realblockneeded - realblockoccupy;
    checkblockoverflow(realblockneeded);
    FILE *fp = openfs(simfs, "r + b");
    short firstblock = getfirsnode();
    int input = files[i].firstblock == -1 ? firstblock : files[i].firstblock;
    int blockindex = getnegativenextblock(input);
    while(blocksizeneeded > total){
        short seek = getfirsnode();
        fseek(fp, (seek)*BLOCKSIZE,SEEK_SET);
        char zerobuf[BLOCKSIZE] = {0};
        fwrite(zerobuf, BLOCKSIZE, 1, fp);
        seek += 1;
        total += BLOCKSIZE;
        if (files[i].firstblock == -1){
            files[i].firstblock = firstblock;
            fnodes[firstblock].blockindex = firstblock;
            continue;
        }else{
            short nextblock = getfirsnode();
            fnodes[blockindex].nextblock = nextblock;
            fnodes[nextblock].blockindex = nextblock;
            blockindex = nextblock;
        }
    }
    writemetadata(simfs);
    closefs(fp);
}

void changefnodefentryfield(char* simfs, int start, int length, int i,char* userinput){
    if (length != strlen(userinput)){  // handle error for parameter length
        fprintf(stderr, "Error:The input size does not match the parameter length\n");
        exit(1);
    }
    if (files[i].size < start){ // handle error for parameter start
        fprintf(stderr, "Error: start offset can not be greater than file size\n");
        exit(1);
    }
    initializeblock(simfs, i, start, length); // handle blockoverflow error didn't do right now
    int fsizeafterinput = files[i].size;
    if (start == files[i].size){
        fsizeafterinput = length + files[i].size;
    }else if (length >= files[i].size - start){
        fsizeafterinput = start + length;
    }
    files[i].size = fsizeafterinput;      // change file size field
    writemetadata(simfs);
}

char  buffer2[1], bufferbig[MAXBLOCKS*BLOCKSIZE];
void processdata(char* simfs, char* userinput, int i, int start, int length, int mode){
    FILE * fp = openfs(simfs,"r+b");
    int blockindex = skipstart(simfs,i,start);
    int metaandstart = blockindex*BLOCKSIZE + start % BLOCKSIZE;//seek start (offset) and meta
    fseek(fp,metaandstart, SEEK_SET);
    int counter = 0, block = 1, size = 0;
    fnode current = fnodes[blockindex];
    char buffer[1];
    while (counter < length){
        if (block == 1){
            size = BLOCKSIZE - start % BLOCKSIZE;
        }else{
            size = block * BLOCKSIZE;
        }
        if (counter == size){
            fseek(fp,current.nextblock * BLOCKSIZE, SEEK_SET);
            current = fnodes[current.nextblock];
            block += 1;
        }
        if (mode == 0){
            counter += fread(buffer2,sizeof(char),1,fp);
            strncat(bufferbig,buffer2,sizeof(buffer2));
        }else{
            buffer[0] = userinput[counter]; 
            counter += fwrite(buffer,sizeof(char),1,fp);
        }
    }
    if (mode == 0){
        printf("%s",bufferbig);
    }
    closefs(fp);  
}

void writefile(char* simfs, char* file, int start, int length, char* userinput){
    storemetadata(simfs);
    for (int i = 0; i <= MAXFILES; i++){
        if (i == MAXFILES){  //if there is no this file in file system
             fprintf(stderr, "Error: There is no this file in file system\n");
             exit(1);
        }
        if (strncmp(files[i].name,file,11) == 0){
            changefnodefentryfield(simfs, start, length, i, userinput);
            processdata(simfs,userinput,files[i].firstblock,start,length,1);
            break;
        }
    }
}

void readfile(char* simfs, char* file, int start, int length){
    storemetadata(simfs);
    for (int i = 0; i <= MAXFILES; i++){
        if (i == MAXFILES){  //if there is no this file in file system
             fprintf(stderr, "Error: There is no this file in file system\n");
             exit(1);
        }
        if (strncmp(files[i].name,file,11) == 0){
            if ((start + length) > files[i].size || start == files[i].size){
                fprintf(stderr, "Error: Invalid read, the location you want to read is invalid\n");
                exit(1);
            }
            char null[1];
            processdata(simfs, null, files[i].firstblock,start,length,0);
            break;
        }
    }
}

void deletefile(char* simfs, char* file){
    FILE* fp = openfs(simfs,"r+b");
    storemetadata(simfs);
    for (int i = 0; i <= MAXFILES; i++){
        if (i == MAXFILES){  //if there is no this file in file system
             fprintf(stderr, "Error: There is no this file in file system\n");
             exit(1);
        }
        if (strncmp(files[i].name,file,11) == 0){
            files[i].name[0] ='\0';
            files[i].size = 0;
            int curr = files[i].firstblock;
            files[i].firstblock = -1;
            if (curr != -1){
                fnodes[curr].blockindex = - fnodes[curr].blockindex;
                char zerobuf[BLOCKSIZE] = {0};
                fseek(fp,curr*BLOCKSIZE,SEEK_SET);
                fwrite(zerobuf, BLOCKSIZE, 1, fp);
                while (fnodes[curr].nextblock != -1){
                    int next = fnodes[curr].nextblock;
                    fseek(fp,next*BLOCKSIZE,SEEK_SET);
                    fwrite(zerobuf, BLOCKSIZE, 1, fp);
                    fnodes[curr].nextblock = -1;
                    fnodes[next].blockindex = -fnodes[next].blockindex;
                    curr = next;
                }
            }
            break;
        }
    }
    writemetadata(simfs);
    closefs(fp);
}