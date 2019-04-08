#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void createSuperBlock(FILE* disk);
void createFreeBlockVector(FILE* disk);
void createEmptyBlocks(FILE* disk);
void init(FILE* disk);
void readBlock(FILE* disk, int blockNum, char* buffer);
void writeBlock(FILE* disk, int blockNum, char* data);
char* createEmptyInode();
char* addBlock(char* inode, int block);
char* intToHex(char* buf, int val);