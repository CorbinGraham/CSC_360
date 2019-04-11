#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Used to check one bit at a time https://stackoverflow.com/questions/523724/c-c-check-if-one-bit-is-set-in-i-e-int-variable
#define SetBit(A,k) (A[(k/8)] |= (1 << (k%8)))
#define ClearBit(A,k) (A[(k/8)] &= ~(1 << (k%8)))
#define TestBit(A,k) (A[(k/8)] & (1 << (k%8)))

void createSuperBlock(FILE* disk);
void createFreeBlockVector(FILE* disk);
void createEmptyBlocks(FILE* disk);
void init(FILE* disk);
void readBlock(FILE* disk, int blockNum, char* buffer);
void writeBlock(FILE* disk, int blockNum, char* data);
struct inode createEmptyInode();
char* addBlock(char* inode, int block);
char* intToHex(char* buf, int val);
char* shortToHex(char* buf, int val);
int findFirstFreeBlock(FILE* disk);
int findInode(FILE* disk);
int writeToFile(FILE* disk, char* data, int simulate);
char* readFile(FILE* disk, char* buffer);
void createDirectory(FILE* disk, char* filename);
void setBitandWrite(FILE* disk, int location, int blockNum);
void clearBitandWrite(FILE* disk, int location, int blockNum);


struct inode 
{ 
   int fileSize; 
   int flag; 
   short directBlock[10]; 
   short singleIndirect;
   short doubleIndirect; 
};