#include "File.h"
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;
const int MAGIC_NUMBER = 177013;


//=============================++++++++++++++++++++++++*********** INIT METHODS ****************+++++++++++++++++++==========================
void initLLC(FILE* disk){
    createSuperBlock(disk);
    createFreeBlockVector(disk);
    createEmptyBlocks(disk);
    createDirectory(disk, "/");
}

//Indicates all the important information about the file system and its implementation.
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

    //Fill the remainder of the block with 0's after the important info.
    int i;
    len = strlen(buffer);
    for(i = len; i < BLOCK_SIZE; i++) {
        buffer[i] = 0;
    }
    
    writeBlock(disk, 0, buffer);

    readBlock(disk, 0, buffer);
    free(buffer);
}

//Indicates, by using a single bit, which blocks are free and which blocks are in use.
void createFreeBlockVector(FILE* disk){
    char* buffer = malloc(BLOCK_SIZE);
    
    //Reserving the first 10 blocks
    buffer[0] = 0;
    buffer[1] = 252;

    int i;
    for(i = 2; i < (BLOCK_SIZE); i++) {
        buffer[i] = 255;
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


//========================+++++++++++++++***************** FILES *******************+++++++++++++++++======================
/******* TODO: Break this down into meathods because it sucks right now.
This method allows the user to write their file to disk.
    The additionaly "simulate" int is used to indicate that no writing to disk will be done.
    This allows us to find where an Inode will appear without writing anything.
    this will be useful when creating directories.
*/

int writeToFile(FILE* disk, char* data, int simulate) {
    //Find a location to store the iNode for the new file
    struct inode inode = createEmptyInode();
    inode.fileSize = strlen(data);
    inode.flag = 0;

    int inodeLocation;
    if(simulate != 0) {
        inodeLocation = findFirstFreeBlock(disk);
    }else {
        inodeLocation = findFirstFreeBlock(disk);
        clearBitandWrite(disk, inodeLocation, 1);
        //Set the inode bit to indicate a new inode exists
        setBitandWrite(disk, inodeLocation, 2);
    }

    //This gets the first block that we can write.
    //TODO: implement add block function.
    int numBlocksRequired = strlen(data) / BLOCK_SIZE;
    //Covers the remainder of the data length
    if (strlen(data) % BLOCK_SIZE != 0) {
        numBlocksRequired++;
    }

    //Limit the size of a file to less than 10 blocks.
    if(numBlocksRequired > 10) {
        printf("File size is greater than 10 blocks. Not writing to disk.");
        return -1;
    }

    printf("%d\n", inodeLocation);
    //The block buffer holds enough chars to fill a block
    char* blockBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    int i;
    for(i = 0; i < numBlocksRequired; i++) {
        //Copy one blocks worth of data to the buffer.
        strncpy(blockBuffer, data + BLOCK_SIZE*i , BLOCK_SIZE);
        //Find a block to store it in
        int freeBlock;
        if(simulate != 0) {
            freeBlock = findFirstFreeBlock(disk);
        } else {
            freeBlock = findFirstFreeBlock(disk);
            clearBitandWrite(disk, freeBlock, 1);
            writeBlock(disk, freeBlock, blockBuffer);
        }

        printf("%d\n", freeBlock);

        //Have the inode point to the newly written block
        inode.directBlock[i] = freeBlock;
    }

    //Finally, write the Inode to the previously determined block location.
    if(simulate != 0) {

    }else {
        fseek(disk, inodeLocation*BLOCK_SIZE, SEEK_SET);
        fwrite(&inode, sizeof(inode), 1, disk);
    }
    free(blockBuffer);

    return inodeLocation;
}

char* readFile(FILE* disk, char* buffer) {
    int inodeBlockLocation = findInode(disk);
    struct inode inode;

    fseek(disk, inodeBlockLocation * BLOCK_SIZE, SEEK_SET);
    fread(&inode, sizeof(inode), 1, disk);

    int numBlockstoRead = inode.fileSize / BLOCK_SIZE;
    //Covers the remainder of the data length
    if (inode.fileSize % BLOCK_SIZE != 0) {
        numBlockstoRead++;
    }

    int i;
    char* fileBuffer = malloc(numBlockstoRead * BLOCK_SIZE);
    char* blockBuffer = malloc(BLOCK_SIZE);
    for(i = 0; i < numBlockstoRead; i++) {
        readBlock(disk, inode.directBlock[i], blockBuffer);
        //Copy one blocks worth of data to the buffer.
        strncpy(fileBuffer + BLOCK_SIZE*i, blockBuffer , BLOCK_SIZE);
    }

    free(fileBuffer);
    free(blockBuffer);

    return fileBuffer;
}


/*=================================++++++++++++++++********* DIRECTORIES ***********+++++++++++++++++++==========================
Structure:
    First 2 bytes are for Inode location (0 = no file)
    Next 30 bytes are for file name 
*/  
void createDirectory(FILE* disk, char* fileName) {
    char* blockBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    char* contents = malloc(INODE_SIZE);

    if(strlen(fileName) > 29) {
        printf("%s\n", "Filename is too long");
    }

    //First simulate creation to get block number
    shortToHex(blockBuffer, writeToFile(disk, blockBuffer, 1));

    strncpy(blockBuffer + 2, fileName, INODE_SIZE-2);

    //Actually finish the file write.
    writeToFile(disk, blockBuffer, 0);

    free(contents);
    free(blockBuffer);
}


//=============================++++++++++++++++++++++++*********** MISC ****************+++++++++++++++++++==========================
char* intToHex(char* buf, int val) {
    //Modified code from https://cboard.cprogramming.com/c-programming/117826-convert-integer-hex-array-chars.html to convert to hex from int
    sprintf( buf, "%08x", val);
    return buf;
}

char* shortToHex(char* buf, int val) {
    //Modified code from https://cboard.cprogramming.com/c-programming/117826-convert-integer-hex-array-chars.html to convert to hex from int
    sprintf( buf, "%02x", val);
    return buf;
}

void readBlock(FILE* disk, int blockNum, char* buffer){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}

void writeBlock(FILE* disk, int blockNum, char* data){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, BLOCK_SIZE, 1, disk); // Note: this will overwrite existing data in the block
}

struct inode createEmptyInode() {
    struct inode inode;
    inode.fileSize = 0;
    inode.flag = 0;

    int i;
    for(i = 0; i < 10; i++) {
        inode.directBlock[i] = 0;
    }
    inode.singleIndirect = 0;
    inode.doubleIndirect = 0;
    return inode;
}


int findFirstFreeBlock(FILE* disk) {
    int i;
    char* freeBlockVector = malloc(BLOCK_SIZE);

    readBlock(disk, 1, freeBlockVector);

    //Always skip the first 10 blocks
    for(i = 9; i < BLOCK_SIZE; i++) {
        if(TestBit(freeBlockVector, i) != 0) {
            free(freeBlockVector);
            return i;
        }
    }

    free(freeBlockVector);
    return -1;
}

void clearBitandWrite(FILE* disk, int location, int blockNum) {
    char* inodeBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    readBlock(disk, blockNum, inodeBuffer);
    ClearBit(inodeBuffer, location);
    writeBlock(disk, blockNum, inodeBuffer);

    free(inodeBuffer);
}

void setBitandWrite(FILE* disk, int location, int blockNum) {
    char* inodeBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    readBlock(disk, blockNum, inodeBuffer);
    SetBit(inodeBuffer, location);
    writeBlock(disk, blockNum, inodeBuffer);

    free(inodeBuffer);
}

//Indicates, by using a single bit, which blocks CONTAIN an Inode.
//This works almost identically to the free block vector.
//Legend:
    //0 = no Inode in block
    //1 = Inode contained in block
//For this to work properly, it will need to always ignore the first 10 block.
int findInode(FILE* disk) {
    int i;
    char* InodeBlockVector = malloc(BLOCK_SIZE);

    readBlock(disk, 2, InodeBlockVector);

    //Always skip the first 10 blocks

    //******* I NEED TO IMPLEMENT THIS ********
    for(i = 10; i < BLOCK_SIZE; i++) {
        if(TestBit(InodeBlockVector, i) != 0) {
            free(InodeBlockVector);
            return i;
        }
    }
    free(InodeBlockVector);
    return -1;
}

int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "w+b");
    // Maybe add more things to the inode
    initLLC(disk);

    char buffer[10];
    writeToFile(disk, "Scarcely on striking packages by so property in delicate. Up or well must less rent read walk so be. Easy sold at do hour sing spot. Any meant has cease too the decay. Since party burst am it match. By or blushes between besides offices noisier as. Sending do brought winding compass in. Paid day till shed only fact age its end. Scarcely on striking packages by so property in delicate. Up or well must less rent read walk so be. Easy sold at do hour sing spot. Any meant has cease too the decay. Since party burst am it match. By or blushes between besides offices noisier as. Sending do brought winding compass in. Paid day till shed only fact age its end. ", 0);
    readFile(disk, buffer);

    fclose(disk);
    return 0;
}