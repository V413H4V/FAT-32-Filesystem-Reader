# FAT-32 Filesystem Reader

### It is a user space shell application that is capable of interpreting a FAT-32 file system image.

### Usage: 
- Compile with: 	gcc -Wall FatReader.c -o FatReader
- Run with: 		./FatReader

=================================================================================
### Supported commands:

  - ### <p><code> open <filename> </code></p> : This command opens a FAT-32 image file
  #####
  - ### <p><code> info </code></p> : This command will print out the  information about the file system in both- hexadecimal and base 10.
 1) BPB_BytesPerSec : Bytes per Sector
 
 2) BPB_SecPerClus :    Sectors per Cluster
 
 3) BPB_RsvdSecCnt :    Sectors on Reserved part
 
 4) BPB_NumFATs :   Number of File Allocation Tables
 
 5) BPB_FATSz32 :   Sectors in FAT
 

- ### <p><code> stat <filename> or <directory name> </code></p> : This command prints out the file attribute, file size and the starting cluster number.
######
- ### <p><code> get <filename> </code></p> : This command will retrieve the file from FAT-32 image and store it in your current working directory (in local hard-drive).
######
- ### <p><code> cd <directory> </code></p> : Changes the current working directory to the given directory. Also supports relative paths like ../name or ../../name
######
- ### <p><code> ls  </code></p> : Lists the directory content. Also supports listing "." and ".." directories. Hides the hidden files and system files.
######
- ### <p><code> read <filename> <offset position> <no. of bytes to read> </code></p> : Reads the number of bytes from the given file, from the given position (in bytes) and prints it out in the terminal.
######
- ### <p><code> volume </code></p> : Prints out the volume name of the file system image.
