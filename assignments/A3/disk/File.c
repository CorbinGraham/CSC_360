#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;


void createSuperBlock(FILE* disk){

}

void createFreeBlockVector(FILE* disk){
    //I use BLOCK_SIZE instead of NUM_BLOCKS since it's more descript about what it's doing
    char* buffer = malloc(BLOCK_SIZE*8);
    
    //Reserving the first 10 blocks
    for(int i = 0; i < 10; i++){
        buffer[i] = 0;
    }

    for(int i = 10; i < (BLOCK_SIZE*8); i++) {
        buffer[i] = 1;
    }
}

void init(FILE* disk){

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

char* addBlock(char* inode, int block) {
    // You're going to probably want a bunch of functions like these
}

int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "w+b");
    // Maybe add more things to the inode
    writeBlock(disk, 2, myInode);

    free(myInode);
    fclose(disk);
    return 0;
}