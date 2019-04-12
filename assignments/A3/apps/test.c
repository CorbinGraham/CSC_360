#include "../disk/File.c"

int testCreatingVdisk(FILE* disk) {
	printf("\n%s\n", "CREATING VDISK:");
	initLLC(disk);
	printf("%s\n\n\n", "	Succesfully created vdisk");
	return 0;
}

int testCreatingFile(FILE* disk) {
	printf("%s\n", "TESTING CREATING FILE:");
	writeToFile(disk, "This is the contents of the file", "Afilename", 1, 0);
	printf("%s\n\n\n", "	Wrote file \"Afilename\" to root directory");
	return 0;
}

int testReadingFile(FILE* disk, char* filename) {
	printf("%s\n", "TESTING READING FILE:");
	char buffer[10];
	char* read = malloc(BLOCK_SIZE * 10);
	read = readFile(disk, filename, buffer);

	printf("	Read in \"%s\" from \"Afilename\"\n\n\n", read);

	return 0;
}

int testDeleteingFile(FILE* disk, char* filename) {
	printf("%s\n", "TESTING DELETEING FILE:");
	deleteFile(disk, filename);
	printf("	Deleted \"Afilename\" from Vdisk\n\n\n");
	return 0;
}

int testCreatingMultiBlockFile(FILE* disk) {
	printf("%s\n", "TESTING CREATING MUTIBLOCK FILE:");
	writeToFile(disk, "The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitudeThe immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. TThe immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. The sacrifice pinches the attitude.The immature cloth fusses above a triumph. An apparatus kids with a low genetics. The innocent agenda speaks. T.", "Alongfile", 1, 0);
	printf("%s\n\n\n", "	Wrote file \"Alongfile\" to root directory");
	return 0;
}

int testReadingMultiBlockFile(FILE* disk, char* filename) {
	printf("%s\n", "TESTING READING MULTIBLOCK FILE:");
	char buffer[10];
	char* read = malloc(BLOCK_SIZE * 10);
	read = readFile(disk, filename, buffer);

	printf("	read in \n	\"%s\"\n from \"Afilename\"\n\n\n", read);
	return 0;
}

int testDeleteingMultiBlockFile(FILE* disk, char* filename) {
	printf("%s\n", "TESTING DELETEING MULTIBLOCK FILE:");
	deleteFile(disk, filename);
	printf("	Deleted \"Alongfile\" from Vdisk\n\n\n");
	return 0;
}

int testCreatingDirectory(FILE* disk) {
	printf("%s\n", "TESTING CREATING DIRECTORY:");
	createDirectory(disk, "Adirectory");
	printf("	Created \"Adirectory\" in root directory\n\n\n");
	return 0;
}

int testDeleteingDirectory(FILE* disk, char* filename) {
	printf("%s\n", "TESTING DELETEING DIRECTORY:");
	deleteFile(disk, filename);
	printf("	Deleted \"Adirectory\" from Vdisk\n\n\n");
	return 0;
}

int testWritingSeveralFiles(FILE* disk){
	printf("%s\n", "TESTING CREATION OF MANY FILES:");
	int i;
	for(i = 0; i < 10; i++) {
		char name = (char) i;
		writeToFile(disk, "This is the contents of the file", &name, 1, 0);
	}
	
	printf("%s\n\n\n", "	Wrote 10 files to root directory");
	return 0;
}

int testReadingSeveralFiles(FILE* disk){
	printf("%s\n", "TESTING READING OF MANY FILES:");
	char buffer[10];
	char* read = malloc(BLOCK_SIZE * 10);
	int i;
	for(i = 0; i < 10; i++) {
		char name = (char) i;
		read = readFile(disk, &name, buffer);
		printf("	Read in \"%s\" from %d\n",read,name);
	}
	
	printf("\n\n\n");
	return 0;
}

int main(int argc, char* argv[]) {
	FILE* disk = fopen("disk/vdisk", "w+b");
	testCreatingVdisk(disk);
	testCreatingFile(disk);
	testReadingFile(disk, "Afilename");
	testDeleteingFile(disk, "Afilename");

	testCreatingMultiBlockFile(disk);
	testReadingMultiBlockFile(disk, "Alongfile");
	testDeleteingMultiBlockFile(disk, "Alongfile");

	testCreatingDirectory(disk);
	testDeleteingDirectory(disk, "Adirectory");

	testWritingSeveralFiles(disk);
	testReadingSeveralFiles(disk);

	printf("%s\n", "Thanks for testing ^̮^");

	fclose(disk);

	return 0;
}