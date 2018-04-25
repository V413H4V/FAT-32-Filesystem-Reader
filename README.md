# FAT-32 Filesystem Reader

### It is a user space shell application that is capable of interpreting a FAT-32 file system image.
=================================================================================
### Supported commands:

  - ### open <filename> : This command opens a FAT-32 image file
  #####
  - ### info : This command will print out the  information about the file system in both- hexadecimal and base 10.
 1) BPB_BytesPerSec : Bytes per Sector
 
 2) BPB_SecPerClus :    Sectors per Cluster
 
 3) BPB_RsvdSecCnt :    Sectors on Reserved part
 
 4) BPB_NumFATs :   Number of File Allocation Tables
 
 5) BPB_FATSz32 :   Sectors in FAT
 

- ### stat <filename> or <directory name> : This command prints out the file attribute, file size and the starting cluster number.
######
- ### get <filename> : This command will retrieve the file from FAT-32 image and store it in your current working directory (in local hard-drive).
######
- ### cd <directory> : Changes the current working directory to the given directory. Also supports relative paths like ../name or ../../name
######
- ### ls  : Lists the directory content. Also supports listing "." and ".." directories. Hides the hidden files and system files.
######
- ### read <filename> <offset position> <no. of bytes to read> : Reads the number of bytes from the given file, from the given position (in bytes) and prints it out in the terminal.
######
- ### volume : Prints out the volume name of the file system image.
