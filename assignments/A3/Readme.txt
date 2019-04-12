
███████╗██╗██╗     ███████╗    ███████╗██╗   ██╗███████╗████████╗███████╗███╗   ███╗
██╔════╝██║██║     ██╔════╝    ██╔════╝╚██╗ ██╔╝██╔════╝╚══██╔══╝██╔════╝████╗ ████║
█████╗  ██║██║     █████╗      ███████╗ ╚████╔╝ ███████╗   ██║   █████╗  ██╔████╔██║
██╔══╝  ██║██║     ██╔══╝      ╚════██║  ╚██╔╝  ╚════██║   ██║   ██╔══╝  ██║╚██╔╝██║
██║     ██║███████╗███████╗    ███████║   ██║   ███████║   ██║   ███████╗██║ ╚═╝ ██║
╚═╝     ╚═╝╚══════╝╚══════╝    ╚══════╝   ╚═╝   ╚══════╝   ╚═╝   ╚══════╝╚═╝     ╚═╝
                                                                                    

Name: Corbin Graham
VNUM: V00865941

Put a good 35 hours into this but was not able to finish all the way. I've listed the limitations below.

In facebook group with:
	Cole Lance Lansing
	Aiden Bull
	Trevor Rutherford
	Andrew Calich
	Richard Cuikshank
	Carter Heckman
	Craig Mathieson
	Mark Guebert
	Scott Andreen
	Christian Knowles


***NOTE: File.c is the the "disk" directory. There is no "io" directory. 

HOW TO RUN:
	- In the "A3" directory run "make"
	- While still in the root directory, run "apps/test"
	- A list of tests will be run and completed

FILE SYSTEM IMPLEMENTAITON:
	DECISIONS:
		- The file system works by creating a root directory and baseing all files from that point. 
			- The structure of a directory is 2 bytes for referencing the files inode and 30 bytes for a filename.
			- The first entry in a new directory is self referencing. This allows me to know which directory I'm in a quick glance.

		- Any newly created file is placed in the root directory.
			- Inodes appear BEFORE it's associated data blocks.
			- Contents are placed in subsequent free blocks that the file system finds.

	INODE MAPPING:
		- I used block 2 as a bitmap (similar to the free block vector) to indicate if a block had an inode in it or not.
			- 1 = inode in block location 0 = no inode in block location.
		- 1 inode = 1 block (inefficient).

	DELETION:
		- Files are found based on a given filename.
			- First, all blocks pointed to by the files Inode direct blocks are deleted. These blocks are also freed.
			- Next, the inode is deleted and it's block is freed. Its inode mapping is also set to 0.
			- Finally, the files directory entry is removed.
			- The file is now fully stripped from the system.

	WRITING:
		- Data is broken down into block sized pieces and written one at a time.
			- The whole process is just the reversal of the DELETION process.

	READING:
		- Works similar to both deletion and writing. Returns a char* of the read data.


FILE SYSTEM LIMITATIONS:
	- File size is capped at 10 blocks worth of data. I wasn't able to get indirection blocks done in time :(
	- You can create and delete directories in the root directory. However, you can't fully access them.
		- This is because I wasn't able to figure out how to break down the file paths.
		- There is a mess of code still in there that was my work in progress of this task. It is in the findDirectory() method.