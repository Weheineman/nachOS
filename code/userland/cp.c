#include "syscall.h"

int
main(int argc, char **argv)
{
	if(argc != 3){
		Write("Wrong amount of arguments.\n", 28, CONSOLE_OUTPUT);
		Exit(1);
	}
	
	char *sourceName = argv[1];
	char *destinationName = argv[2];
	
	if(!Create(destinationName)){
		Write("Failed to create the new file.\n", 32, CONSOLE_OUTPUT);
		Exit(1);
	}
	
    OpenFileId sourceId = Open(sourceName);
	if(sourceId == -1){
		Write("Failed to open the source file.\n", 33, CONSOLE_OUTPUT);
		Exit(1);
	}
	
	OpenFileId destinationId = Open(destinationName);
	if(destinationId == -1){
		Write("Failed to open the destination file.\n", 38, CONSOLE_OUTPUT);
		Exit(1);
	}
	
	char buffer[2];
	while(Read(buffer, 1, sourceId) != 0){
		Write(buffer, 1, destinationId);
	}
	
	Close(sourceId);
	Close(destinationId);
	    
    Exit(0);

}
