#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_COMMAND_SIZE 60
#define NUM_DIR_RECORDS 16


/*#################################
#  Author: Vaibhav Murkute
#  Project: FAT-32 Filesystem Reader
#  Version: 1.2
#  Date: 05/04/2018
##################################*/


char* StrDup (char *str);
int LBAToOffset(int32_t sector, int16_t BPB_BytesPerSec, int16_t BPB_RsvdSecCnt, int8_t BPB_NumFATs, int32_t BPB_FATSz32);
int16_t NextLB(uint32_t sector, FILE* fp, int16_t BPB_BytesPerSec, int16_t BPB_RsvdSecCnt);

struct __attribute__((__packed__)) DirectoryEntry{
	char DIR_Name[11];
	uint8_t DIR_Attr;
	uint8_t Unused1[8];
	uint16_t DIR_FirstClusterHigh;
	uint8_t Unused2[4];
	uint16_t DIR_FirstClusterLow;
	uint32_t DIR_FileSize;
};

struct CurrentDirectory{
	char directory[12];
	int directoryOffset;
};

int readDirectory(FILE* fp, int offset, struct DirectoryEntry *dir);

int main()
{

    char input[MAX_COMMAND_SIZE];
    char* command[MAX_COMMAND_SIZE];
    memset(command,'\0',sizeof(command));
    memset(input,'\0',sizeof(input));

    FILE *fp = NULL;

    int16_t BPB_BytesPerSec;
    int8_t BPB_SecPerClus;
    int16_t BPB_RsvdSecCnt;
    int8_t BPB_NumFATs;
    int32_t BPB_FATSz32;
    char BS_VolLab[12];
    memset(BS_VolLab,'\0',sizeof(BS_VolLab));

    struct DirectoryEntry dir[NUM_DIR_RECORDS];
    int totalBytesPerCluster;
    int FATOffset=0;
    int rootOffset=0;

    struct CurrentDirectory directoryHistory[10]; // to keep track of last 10 changed directories
    // also supports multilevel relative paths like: cd ../../../name
    int currDirPointer = -1;	// position of curr Directory in directionHistory

    char cwd[500];	// current working directory
    strcpy(cwd,"");


    while(1){
        int count=0;
        memset(input,'\0',sizeof(input));   // clearing input for fresh start

	int x;
	memset(cwd,'\0',sizeof(cwd));
	strcpy(cwd,"");
	// sets current working directory
	for(x=0; x<=currDirPointer; x++){
		strcat(cwd,directoryHistory[x].directory);
		strcat(cwd,"/");
	}

        printf("mfs: %s>\t",cwd);   // prints out the msf prompt
        while( !fgets (input, MAX_COMMAND_SIZE, stdin) );     // scans all the input

        char* str = strtok(input," ");  // Tokenizing the first word

        while(str != NULL){
            str = StrDup(str);
            command[count] = str;       // tokenizing rest of the input string
            str = strtok(NULL," ");
            count += 1;
        }

        command[count] = NULL;      // NULL terminating input string

        if(strcmp(command[0],"open")==0){
		if(fp == NULL){
			fp = fopen(command[1],"r");
			if(fp==NULL){
				printf("<!>File System not found.\n\n\r");
				continue;
			}
				
			// reading Bios Parameters
			fseek(fp,11,SEEK_SET);
			fread(&BPB_BytesPerSec,1,2,fp);

			fseek(fp,13,SEEK_SET);
			fread(&BPB_SecPerClus,1,1,fp);
			
			fseek(fp,14,SEEK_SET);
			fread(&BPB_RsvdSecCnt,1,2,fp);
		
			fseek(fp,16,SEEK_SET);
			fread(&BPB_NumFATs,1,1,fp);
	
			fseek(fp,36,SEEK_SET);
			fread(&BPB_FATSz32,1,4,fp);
			
			fseek(fp,71,SEEK_SET);
			fread(BS_VolLab,1,11,fp);
			
			// calculate FATOffset and rootOffset
			FATOffset = BPB_RsvdSecCnt * BPB_BytesPerSec;
			rootOffset = FATOffset + (BPB_NumFATs*BPB_FATSz32*BPB_BytesPerSec);
			totalBytesPerCluster = BPB_SecPerClus * BPB_BytesPerSec;

			// filling up dir[16] with Root Directory records
			currDirPointer++;
			directoryHistory[currDirPointer].directoryOffset = rootOffset;
			strcpy(directoryHistory[currDirPointer].directory,"");
			readDirectory(fp,directoryHistory[currDirPointer].directoryOffset,dir);

		}else{
			printf("<!>Error: File System Image already open.\n\n\r");
		}

	}else if(strcmp(command[0],"close")==0){
		if(fp != NULL){
			fclose(fp);	
			fp = NULL;	// to indicate no file is currently open 
			currDirPointer = -1;
			strcpy(cwd,"");
		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"exit")==0 || strcmp(command[0],"quit")==0){
		// to ensure file is closed before exiting
		if (fp != NULL)
			fclose(fp);
		exit(0);

	}else if(strcmp(command[0],"info")==0){
		if(fp != NULL){
			printf("BPB_BytesPerSec: %d\n\r",BPB_BytesPerSec);	
			printf("BPB_BytesPerSec: %x\n\r",BPB_BytesPerSec);	
			printf("\n\r");
			printf("BPB_SecPerClus: %d\n\r",BPB_SecPerClus);	
			printf("BPB_SecPerClus: %x\n\r",BPB_SecPerClus);	
			printf("\n\r");
			printf("BPB_RsvdSecCnt: %d\n\r",BPB_RsvdSecCnt);	
			printf("BPB_RsvdSecCnt: %x\n\r",BPB_RsvdSecCnt);	
			printf("\n\r");
			printf("BPB_NumFATs: %d\n\r",BPB_NumFATs);	
			printf("BPB_NumFATs: %x\n\r",BPB_NumFATs);	
			printf("\n\r");
			printf("BPB_FATSz32: %d\n\r",BPB_FATSz32);	
			printf("BPB_FATSz32: %x\n\r",BPB_FATSz32);
			printf("\n\r");
		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"ls")==0){
		if(fp != NULL){
			int j;
			char fileName[12];
			char delFileIndicator[2];
			sprintf(delFileIndicator,"%c",229);	// if the file is deleted (0xe5 =229)

			for(j=0; j<NUM_DIR_RECORDS; j++){
				// display files with attr 0x01 (1), 0x10 (16), 0x20 (32), 0x30 (48)
				if(dir[j].DIR_Attr == 1 || dir[j].DIR_Attr == 16 || dir[j].DIR_Attr == 32 || dir[j].DIR_Attr == 48){
					memset(fileName,'\0',sizeof(fileName));
					memcpy(fileName,dir[j].DIR_Name,11);
					//check if file is deleted
					if(fileName[0] != delFileIndicator[0])
						printf("%s\n\r",fileName);
				}
			}
			printf("\n\r");
		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"stat")==0){
		if(fp != NULL){
			char fileName[13];
			strncpy(fileName,command[1],sizeof(fileName));
			fileName[sizeof(fileName)-1] = '\0';
			// converting file name to uppercase
			int k;
			for(k=0; fileName[k]; k++){
				fileName[k] = toupper(fileName[k]);
			}
		
			// modifying input to add spaces between file name and extension	
			char tempFileName[12];
			memset(tempFileName,' ',12);
			tempFileName[sizeof(tempFileName)-1] = '\0';

			char * token = strtok(fileName,".");
			strncpy(tempFileName,token,strlen(token));
			
			if(strchr(command[1],'.') != NULL){
				token = strtok(NULL,".");
				strncpy(&tempFileName[8],token,3);	//extension
			}
			
			int found=0;
			int p;
			for(p=0; p < NUM_DIR_RECORDS; p++){
				if(strncmp(dir[p].DIR_Name,tempFileName,11)==0){
					found=1;
					printf("%10s\t%10s\t%10s\n\r","Attribute","Size","Starting Cluster Number");
					printf("%10d\t%10d\t%10d\n\r",dir[p].DIR_Attr,dir[p].DIR_FileSize,dir[p].DIR_FirstClusterLow);
					printf("\n\r");
					break;
				}
			}

			if(!found)
				printf("<!> File Not Found.\n\n\r");

		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"cd")==0){
		if(fp != NULL){
			char dirName[20];
			memset(dirName,'\0',sizeof(dirName));
			strcpy(dirName,command[1]);
			dirName[sizeof(dirName)-1] = '\0';
			// converting directory name to uppercase
			int a;
			for(a=0; dirName[a]; a++){
				dirName[a] = toupper(dirName[a]);
			}


			char tempDirName[12];
			char origDirectory[12];	// cuz tempDirName will be modified with spaces n stuff
			memset(origDirectory,'\0',sizeof(origDirectory));
			memset(tempDirName,' ',sizeof(tempDirName));
			tempDirName[sizeof(tempDirName)-1] = '\0';
			strncpy(tempDirName,dirName,strlen(dirName));
			strncpy(origDirectory,dirName,strlen(dirName));
			
			// check if user entered relative paths like ../name or multilevel ../../name
			if(strchr(dirName,'/') != NULL){
				char *str = strtok(dirName,"/");
				char commDir[12];
				while(str != NULL && strcmp(str,"..")==0){
					if(currDirPointer > 0)
						currDirPointer--;
					readDirectory(fp,directoryHistory[currDirPointer].directoryOffset,dir);
					memset(commDir,'\0',sizeof(commDir));
					strcpy(commDir,str);
					str = strtok(NULL,"/");
				}

				if(str != NULL){
					memset(commDir,'\0',sizeof(commDir));
					strncpy(commDir,str,strlen(str));
				}

				memset(tempDirName,' ',sizeof(tempDirName));
				tempDirName[sizeof(tempDirName)-1] = '\0';

				strncpy(tempDirName,commDir,strlen(commDir));
				memset(origDirectory,'\0',sizeof(origDirectory));
				strncpy(origDirectory,commDir,strlen(commDir));

			}else if(strcmp(dirName,"..")==0){
				// if user enters just cd ..
				if(currDirPointer > 0)
					currDirPointer--;
				readDirectory(fp,directoryHistory[currDirPointer].directoryOffset,dir);
				continue;	
			}

			int found=0;	
			int b;
			for(b=0; b < NUM_DIR_RECORDS; b++){
				if(strncmp(dir[b].DIR_Name,tempDirName,11)==0){
					if(dir[b].DIR_Attr == 16){
						found = 1;
						uint16_t clusterLow = dir[b].DIR_FirstClusterLow;
						int blockOffset = LBAToOffset(clusterLow,BPB_BytesPerSec,BPB_RsvdSecCnt,BPB_NumFATs,BPB_FATSz32);
						memset(dir,0,sizeof(dir));	// clearing dir[] content

						currDirPointer++;
						directoryHistory[currDirPointer].directoryOffset = blockOffset;
						strcpy(directoryHistory[currDirPointer].directory,origDirectory);
						readDirectory(fp,directoryHistory[currDirPointer].directoryOffset,dir);		
						break;	

					}else{
						printf("<!> Invalid Directory Name.\n\n\r");
				
					}
					
				}
			}


			if(!found)
				printf("<!> Directory Not Found.\n\n\r");
			
		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"get")==0){
		if(fp != NULL){
			char fileName[13];
			strncpy(fileName,command[1],sizeof(fileName));
			fileName[sizeof(fileName)-1] = '\0';
			// converting file name to uppercase
			int k;
			for(k=0; fileName[k]; k++){
				fileName[k] = toupper(fileName[k]);
			}
		
			// modifying input to add spaces between file name and extension	
			char tempFileName[12];
			memset(tempFileName,' ',12);
			tempFileName[sizeof(tempFileName)-1] = '\0';

			char * token = strtok(fileName,".");
			strncpy(tempFileName,token,strlen(token));
			
			if(strchr(command[1],'.') != NULL){
				token = strtok(NULL,".");
				strncpy(&tempFileName[8],token,3);	//extension
			}
			
			int found=0;
			int p;
			for(p=0; p < NUM_DIR_RECORDS; p++){
				if(strncmp(dir[p].DIR_Name,tempFileName,11)==0){
					found=1;
					uint16_t clusterNumber = dir[p].DIR_FirstClusterLow;
					int blockOffset;
					char buffer[BPB_BytesPerSec+1];
//					int lastChunkSize = dir[p].DIR_FileSize % (BPB_SecPerClus*BPB_BytesPerSec);
					FILE *fpointer = fopen(command[1],"w");

				/*	while(clusterNumber > 0){
						memset(buffer,'\0',sizeof(buffer));
						blockOffset = LBAToOffset(clusterNumber,BPB_BytesPerSec,BPB_RsvdSecCnt,BPB_NumFATs,BPB_FATSz32);
						clusterNumber = NextLB(clusterNumber,fp,BPB_BytesPerSec,BPB_RsvdSecCnt);
						
						fseek(fp,blockOffset,SEEK_SET);

						if(clusterNumber == 0 && lastChunkSize > 0){
							// last chunk from the last file sector
							// last cluster should actually hold -1 bt here clusterNumber is uint16_t
							fread(buffer,lastChunkSize,1,fp);
							fwrite(buffer,strlen(buffer),1,fpointer);
							// size parameter here in fwrite is strlen to avoid writing chars after '\0'
							fflush(fpointer);
						}else{
							
							fread(buffer,BPB_BytesPerSec,1,fp);
							fwrite(buffer,BPB_BytesPerSec,1,fpointer);
							fflush(fpointer);
						}

					}	*/

					int copySize = dir[p].DIR_FileSize;
					while(copySize > 0){
						int numBytes;
						if(copySize < (BPB_SecPerClus*BPB_BytesPerSec)){
							numBytes = copySize;	
						}else{
							numBytes = (BPB_SecPerClus*BPB_BytesPerSec);
						}

						memset(buffer,'\0',sizeof(buffer));
						blockOffset = LBAToOffset(clusterNumber,BPB_BytesPerSec,BPB_RsvdSecCnt,BPB_NumFATs,BPB_FATSz32);
						
						fseek(fp,blockOffset,SEEK_SET);
						fread(buffer,numBytes,1,fp);
						fwrite(buffer,numBytes,1,fpointer);

						copySize -= (BPB_SecPerClus*BPB_BytesPerSec);

						clusterNumber = NextLB(clusterNumber,fp,BPB_BytesPerSec,BPB_RsvdSecCnt);
					
					}
					
					fclose(fpointer);
					break;
				}
			}

			if(!found)
				printf("<!> File Not Found.\n\n\r");

		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"read")==0){
		if(fp != NULL){
			char fileName[13];
			strncpy(fileName,command[1],sizeof(fileName));
			fileName[sizeof(fileName)-1] = '\0';
			// converting file name to uppercase
			int k;
			for(k=0; fileName[k]; k++){
				fileName[k] = toupper(fileName[k]);
			}
		
			// modifying input to add spaces between file name and extension	
			char tempFileName[12];
			memset(tempFileName,' ',12);
			tempFileName[sizeof(tempFileName)-1] = '\0';

			char * token = strtok(fileName,".");
			strncpy(tempFileName,token,strlen(token));
			
			if(strchr(command[1],'.') != NULL){
				token = strtok(NULL,".");
				strncpy(&tempFileName[8],token,3);	//extension
			}
			
			int inputOffset;
			int numBytes;

			if(strlen(command[2]) > 0 && strlen(command[3]) > 0){
				inputOffset = atoi(command[2]);
				numBytes = atoi(command[3]);
			}else{
				printf("<!> Invalid Command format.\n\n\r");
				continue;
			}

			int found=0;
			char buffer[numBytes + 1];
			int blockOffset;
			int division;
			uint16_t clusterNumber;
			int p;
			for(p=0; p < NUM_DIR_RECORDS; p++){
				if(strncmp(dir[p].DIR_Name,tempFileName,11)==0){
					found=1;
					if(inputOffset > dir[p].DIR_FileSize){
						printf("<!> Input offset exceeds file size.\n\n\r");
						break;

					}else if((inputOffset + numBytes) > dir[p].DIR_FileSize){
						printf("<!> Enter a valid amount of bytes to read.Input exceeds file size.\n\n\r");
						break;
					}

					clusterNumber = dir[p].DIR_FirstClusterLow;
					// to move the offset to the right place
					// if the inputOffset is higher than bytes per cluster	
					division = (inputOffset/totalBytesPerCluster);
					int c;
					for(c=0; c < division; c++){
						clusterNumber = NextLB(clusterNumber,fp,BPB_BytesPerSec,BPB_RsvdSecCnt);
					}

					inputOffset = inputOffset - (division * totalBytesPerCluster);

					blockOffset = LBAToOffset(clusterNumber,BPB_BytesPerSec,BPB_RsvdSecCnt,BPB_NumFATs,BPB_FATSz32);
					memset(buffer,'\0',sizeof(buffer));
					fseek(fp,blockOffset+inputOffset,SEEK_SET);
					fread(buffer,numBytes,1,fp);

					printf("%s\n\r",buffer);
					printf("\n\r");
					break;
				}
			}

			if(!found)
				printf("<!> File Not Found.\n\n\r");

		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"volume")==0){
		if(fp != NULL){
			printf("The Volume name of the file is \'%s\'.\n\r",BS_VolLab);
			printf("\n\r");
		
		}else{
			printf("<!>Error: File System Image must be opened first.\n\n\r");
		}

	}else if(strcmp(command[0],"")==0){
		continue;
	}else{
		printf("<!> Invalid Command.\n\n\r");
	}

    }
    return 0;
}

char* StrDup (char *str) {
    char *result = (char *) malloc (strlen (str) + 1);
    if (result == NULL)
        return NULL;
    str[strcspn(str,"\n")]='\0';   // to remove trailing new line character put by fgets()
    strcpy (result, str);
    return result;
}

// returns the offset of the provided cluster (sector) 
int LBAToOffset(int32_t sector, int16_t BPB_BytesPerSec, int16_t BPB_RsvdSecCnt, int8_t BPB_NumFATs, int32_t BPB_FATSz32){
	return ((sector-2)*BPB_BytesPerSec)+(BPB_BytesPerSec * BPB_RsvdSecCnt)+(BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

// looks up into the 1st FAT and returns the next logical block address
int16_t NextLB(uint32_t sector, FILE* fp, int16_t BPB_BytesPerSec, int16_t BPB_RsvdSecCnt){
	uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt)+(sector * 4);
	int16_t val;
	fseek(fp, FATAddress, SEEK_SET);
	fread(&val,2,1,fp);
	return val;
}

int readDirectory(FILE* fp, int offset, struct DirectoryEntry *dir){
	fseek(fp,offset,SEEK_SET);
	int i;
	for(i=0; i<NUM_DIR_RECORDS; i++){
		fread(&dir[i],1,32,fp);
	}

	return 0;
}
