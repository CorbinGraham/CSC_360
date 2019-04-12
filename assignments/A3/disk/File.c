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

int writeToFile(FILE* disk, char* data, char* fileName, int filetype, int simulate) {
    //First we need to check that the directory has enough space for the file.
    int directoryLocation = findDirectory(disk, fileName);
    int freeDirectoryLocation = findFreeDirectorySpot(disk, directoryLocation);
    if(freeDirectoryLocation == -1) {
        printf("%s\n", "Directory is full ᕙ(⇀‸↼‶)ᕗ");
    }

    //Find a location to store the iNode for the new file
    struct inode inode = createEmptyInode();
    inode.fileSize = strlen(data);
    //Indicates that the file is a directory
    if (filetype == 0) {
        inode.flag = 0;
    } else {
        inode.flag = filetype;
    }

    int inodeLocation;
    if(simulate != 0) {
        inodeLocation = findFirstFreeBlock(disk);
    }else {
        inodeLocation = findFirstFreeBlock(disk);
        clearBitandWrite(disk, inodeLocation, 1);
        //Set the inode bit to indicate a new inode exists
        setBitandWrite(disk, inodeLocation, 2);
    }

    //We now need to write and entry to the directory block that the file will be added to
    if(simulate != 0){}
    else {
        insertDirectoryEntry(disk, directoryLocation, freeDirectoryLocation, inodeLocation, fileName);
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
        printf("        Wrote data to block location %d\n", freeBlock);
        //Have the inode point to the newly written block
        inode.directBlock[i] = freeBlock;
    }

    if(simulate != 0) {

    }else {
        printf("        Wrote %d block(s) of data\n", numBlocksRequired);
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

char* readFile(FILE* disk, char* filename, char* buffer) {
    int directoryLocation = findFileByName(disk, filename, 11);
    char* blockBuffer = malloc(BLOCK_SIZE);
    char inodeBlockNum[3];
    readBlock(disk, 11, blockBuffer);

    //Read in the files data block
    strncpy(inodeBlockNum, blockBuffer + directoryLocation, 2);
    int inodeBlockLocation = (int)strtol(inodeBlockNum, NULL, 16);

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
    for(i = 0; i < numBlockstoRead; i++) {
        readBlock(disk, inode.directBlock[i], blockBuffer);
        //Copy one blocks worth of data to the buffer.
        strncpy(fileBuffer + BLOCK_SIZE*i, blockBuffer , BLOCK_SIZE);
    }

    free(fileBuffer);
    free(blockBuffer);

    return fileBuffer;
}

//Filename needs to be exact in order to find it.
int findFileByName(FILE* disk, char* fileName, int blockNum) {
    char* blockBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    readBlock(disk, 11, blockBuffer);

    char currentFile[32];

    int i;
    for (i = 0; i < BLOCK_SIZE/INODE_SIZE; i++) {
        strncpy(currentFile, blockBuffer + i*INODE_SIZE + 2, INODE_SIZE-2);
        if(strcmp(currentFile, fileName) == 0) {
            free(blockBuffer);
            return i*INODE_SIZE;
        }
    }    
    free(blockBuffer);
    return -1;
}

//==============================+++++++++++++++++++********* FILE DELETION **************+++++++++++++++++++=======================

void deleteFile(FILE* disk, char* fileName) {
    char* blockBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    readBlock(disk, 11, blockBuffer);

    int dirLocation = findFileByName(disk, fileName, 11);
    if(dirLocation == -1) {
        printf("%s\n", "Unable to find file to delete");
        return;
    }

    char inodeLocation[3];

    strncpy(inodeLocation, blockBuffer + dirLocation, 2);

    //Delete the contents of the inode
    int inodeValue = (int)strtol(inodeLocation, NULL, 16);
    deleteInodeContents(disk, inodeValue);
    deleteInode(disk, inodeValue);

    //Clean up filename in the directory.
    char emptyFilename[32];
    int i;
    for (i = 0; i < 32; i++){
        emptyFilename[i] = 0;
    }

    strncpy(blockBuffer + inodeValue, emptyFilename, INODE_SIZE);

    writeBlock(disk, 11, blockBuffer);

    free(blockBuffer);
}

void deleteInode(FILE* disk, int inodeBlockNum) {
    char* emptyBlock = malloc(sizeof(char) * BLOCK_SIZE);

    int i;
    for (i = 0; i < BLOCK_SIZE; ++i){   
        emptyBlock[i] = 0;
    }
    printf("%s %d\n", "        Deleted inode at block location",inodeBlockNum);
    //Clear the inode block
    writeBlock(disk, inodeBlockNum, emptyBlock);
    //Free the block in the free block vector.
    setBitandWrite(disk, inodeBlockNum, 1);
    //Indicate an Inode does not exist at the old location.
    clearBitandWrite(disk, inodeBlockNum, 2);

    free(emptyBlock);
}

//Only deletes what is inside the inodes direct blocks and does not delete the Inode itself.
void deleteInodeContents(FILE* disk, int inodeBlockNum) {
    struct inode inode;

    fseek(disk, inodeBlockNum * BLOCK_SIZE, SEEK_SET);
    fread(&inode, sizeof(inode), 1, disk);

    int numBlockstoDelete = inode.fileSize / BLOCK_SIZE;
    //Covers the remainder of the data length
    if (inode.fileSize % BLOCK_SIZE != 0) {
        numBlockstoDelete++;
    }

    char* emptyBlock = malloc(sizeof(char) * BLOCK_SIZE);

    int i;
    for (i = 0; i < BLOCK_SIZE; ++i){   
        emptyBlock[i] = 0;
    }

    for(i = 0; i < numBlockstoDelete; i++) {
        //Copy one blocks worth of data to the buffer.
        writeBlock(disk, inode.directBlock[i], emptyBlock);
        //Make the block available again
        setBitandWrite(disk, inode.directBlock[i], 1);
        printf("%s %d\n", "        Deleted file contents at block location",inode.directBlock[i]);
    }

    free(emptyBlock);
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
    shortToHex(blockBuffer, writeToFile(disk, blockBuffer, fileName, 0, 1));

    strncpy(blockBuffer + 2, fileName, INODE_SIZE-2);

    //Actually finish the file write.
    writeToFile(disk, blockBuffer, fileName, 0, 0);

    free(contents);
    free(blockBuffer);
}

void insertDirectoryEntry(FILE* disk, int directoryBlock, int directorySpot, int inodeLocation, char* fileName){
    char* blockBuffer = malloc(sizeof(char) * BLOCK_SIZE);
    readBlock(disk, directoryBlock, blockBuffer);

    shortToHex(blockBuffer + directorySpot, (short)inodeLocation);
    strncpy(blockBuffer + directorySpot + 2, fileName, INODE_SIZE-2);

    writeBlock(disk, directoryBlock, blockBuffer);

    free(blockBuffer);
}

int findDirectory(FILE* disk, char* fileName) {
    char* tokenized;

    //We need to make a copy of fileName otherwise it tries to tokenize a literal.
    char cpy[30];
    strncpy(cpy, fileName, 29);

    //Counts the number of tokens that will be created
    int i;
    int tokenCount = 0;
    for(i = 0; i < strlen(cpy); i++) {
        if(cpy[i] == '/') {
            tokenCount++;
        }
    }
    //Covers the case where we need to use the root directory
    if (tokenCount == 0 || tokenCount == 1) {
        return 11;
    }

    //Covers the case where we need to use the root directory
    if (tokenCount == 0 || tokenCount == 1) {
        return 11;
    }

    struct inode inode = createEmptyInode();
    char* blockBuffer = malloc(sizeof(char) * BLOCK_SIZE);

    int counter = 0;
    tokenized = strtok(cpy, "/");
    while (tokenized != NULL){
        char inodeBlockNum[3];
        int directoryLocation;
        int dataBlockNum;
        int directBlockLocation;

        if (strcmp(strrchr(cpy,'/')+1, tokenized) == 0) {
            printf("%d\n", directoryLocation);
            return directoryLocation;
        }

        tokenized = strtok (NULL, "/");

        if(counter == 0) {
            //Always start the search from the root directory.
            directoryLocation = findFileByName(disk, tokenized, 11);
            readBlock(disk, 11, blockBuffer);
        }else {
            directoryLocation = findFileByName(disk, tokenized, directBlockLocation);
            readBlock(disk, directBlockLocation, blockBuffer);
        }
        strncpy(inodeBlockNum, blockBuffer + directoryLocation, INODE_SIZE);
        dataBlockNum = (int)strtol(inodeBlockNum, NULL, 16);

        //Read in the newly found inode
        fseek(disk, dataBlockNum * BLOCK_SIZE, SEEK_SET);
        fread(&inode, sizeof(inode), 1, disk);

        directBlockLocation = inode.directBlock[0];
        counter++;
    }
    free(blockBuffer);
    return -1;
}

int findFreeDirectorySpot(FILE* disk, int directoryBlockNum) {
    char* directory = malloc(sizeof(char) * BLOCK_SIZE);
    readBlock(disk, directoryBlockNum, directory);
    char currentFile[32];

    //Always skip the first inode spot.
    int i;
    for(i = 1; i < BLOCK_SIZE/INODE_SIZE; i++) {
        strncpy(currentFile, directory + i*INODE_SIZE, INODE_SIZE);
        char inodeLocation[2];

        strncpy(inodeLocation, currentFile, 2);

        long inodeValue = strtol(inodeLocation, NULL, 16);
        if (inodeValue == 0) {
            free(directory);
            return i * INODE_SIZE;
        }
    }

    free(directory);
    return -1;
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