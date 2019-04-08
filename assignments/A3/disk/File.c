#include "File.h"
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;
const int MAGIC_NUMBER = 177013;

char* intToHex(char* buf, int val) {
    //Modified code from https://cboard.cprogramming.com/c-programming/117826-convert-integer-hex-array-chars.html to convert to hex from int
    sprintf( buf, "%08x", val);
    return buf;
}
void createSuperBlock(FILE* disk){
    char* buffer = malloc(BLOCK_SIZE*sizeof(char));
    char magicBuf[sizeof(int)];
    char blockBuf[sizeof(int)];
    char inodeBuf[sizeof(int)];

    intToHex(magicBuf, MAGIC_NUMBER);
    intToHex(blockBuf, NUM_BLOCKS);
    intToHex(inodeBuf, BLOCK_SIZE);

    size_t len = strlen(magicBuf);

    strncpy(buffer, magicBuf, len);
    strcat(buffer, blockBuf);
    strcat(buffer, inodeBuf);

    int i;
    len = strlen(buffer);
    for(i = len; i < BLOCK_SIZE; i++) {
        buffer[i] = 0;
    }
    
    writeBlock(disk, 0, buffer);

    readBlock(disk, 0, buffer);
    free(buffer);
}

void createFreeBlockVector(FILE* disk){
    //I use BLOCK_SIZE instead of NUM_BLOCKS since it's more descript about what it's doing
    char* buffer = malloc(BLOCK_SIZE*8);
    
    int i;
    for(i = 0; i < (BLOCK_SIZE*8); i++) {
        //Reserving the first 10 blocks
        if(i < 10) {
            buffer[i] = 0;   
        }
        else {
            buffer[i] = 1;
        }
    }
    writeBlock(disk, 1, buffer);
    free(buffer);
}

void createEmptyBlocks(FILE* disk) {
    char* buffer = malloc(BLOCK_SIZE*8);

    int i;
    for (i = 0; i < BLOCK_SIZE*8; ++i)
    {   
        buffer[i] = 0;
    }

    for (i = 2; i < NUM_BLOCKS; ++i)
    {
        writeBlock(disk, i, buffer);
    }

    free(buffer);
}

void initLLC(FILE* disk){
    createSuperBlock(disk);
    createFreeBlockVector(disk);
    createEmptyBlocks(disk);
}

void readBlock(FILE* disk, int blockNum, char* buffer){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}

void writeBlock(FILE* disk, int blockNum, char* data){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, BLOCK_SIZE, 1, disk); // Note: this will overwrite existing data in the block
}

char* createEmptyInode() {
    char* inode = malloc(32);
    inode[10] = 3;  //First direct block value pointing to block number 3
    return inode;
}

//char* addBlock(char* inode, int block) {
    // You're going to probably want a bunch of functions like these
//}

int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "w+b");
    // Maybe add more things to the inode
    initLLC(disk);

    fclose(disk);
    return 0;
}