#include "syscall.h"


#define MAX_LINE_SIZE  128
#define MAX_ARG_COUNT  16
#define ARG_SEPARATOR  ' '
#define ARG_WRAPPER '"'
#define PARALLEL_MARKER '&'

#define NULL  ((void *) 0)

#define ERR_NULL_LINE (-1)
#define ERR_NULL_ARGV (-2)
#define ERR_ARGV_ZERO (-3)
#define ERR_BAD_SYNTAX (-4)
#define ERR_TOO_MANY_ARGS (-5)

#define PARSE_EMPTY (-1)
#define PARSE_ERR_NO_WRAPPER (-2)

static inline unsigned
strlen(const char *s)
{
    if(s == NULL) return 0;
    
    unsigned i;
    for (i = 0; s[i] != '\0'; i++);
    return i;
}

static inline void
WritePrompt(OpenFileId output)
{
    static const char PROMPT[] = "--> ";
    Write(PROMPT, sizeof PROMPT - 1, output);
}

static inline void
WriteError(const char *description, OpenFileId output)
{
    if(description != NULL){
		static const char PREFIX[] = "Error: ";
		static const char SUFFIX[] = "\n";

		Write(PREFIX, sizeof PREFIX - 1, output);
		Write(description, strlen(description), output);
		Write(SUFFIX, sizeof SUFFIX - 1, output);
	}
}

static unsigned
ReadLine(char *buffer, unsigned size, OpenFileId input)
{
    if (buffer == NULL) return 0;

    unsigned i;

    for (i = 0; i < size; i++) {
        Read(&buffer[i], 1, input);
        if (buffer[i] == '\0')
            break;
        
    }
    return i;
}

static char*
ParseToken (char* line, int* length){
	unsigned index = 0;
	
	while (line[index] == ARG_SEPARATOR) index++;
	
	if (line[index] == '\0'){
		*length = PARSE_EMPTY;
		return line;
	}
	
	if (line[index] == ARG_WRAPPER){
		char *start = line + index + 1;
		index = 0;
		
		while(start[index] != ARG_WRAPPER && start[index] != '\0') index ++;
		
		if(start[index] == ARG_WRAPPER){
			start[index] = '\0';
			*length = index;
			return start;
		}
		else{
			*length = PARSE_ERR_NO_WRAPPER;
			return start;
		}
	}
	else{
		char *start = line + index;
		index = 0;
		
		while(start[index] != ARG_SEPARATOR && start[index] != '\0') index ++;
		
		start[index] = '\0';
		*length = index;
		return start;
	}
}


static int
PrepareArguments(char *line, char **argv, unsigned argvSize)
{
    if (line == NULL) return ERR_NULL_LINE;
    if (argv == NULL) return ERR_NULL_ARGV;
    if (argvSize == 0) return ERR_ARGV_ZERO;

    unsigned argCount = 0;
	int parseResult = 0;
	char* currentToken = line;
	
	// Traverse the whole line and replace spaces between arguments by null
    // characters, so as to be able to treat each argument as a standalone
    // string.
    
    while(argCount < argvSize){
		argv[argCount] = ParseToken(currentToken, &parseResult);
		if(parseResult == PARSE_EMPTY)
			break;
		
		if(parseResult == PARSE_ERR_NO_WRAPPER)
			return ERR_BAD_SYNTAX;
		
		currentToken = argv[argCount] + parseResult + 1;
		argCount++; 	
	}
    
    if(argCount == argvSize)
		return ERR_TOO_MANY_ARGS;
		
	argv[argCount] = NULL;
	return argCount;
}

int
main(void)
{
    const OpenFileId INPUT  = CONSOLE_INPUT;
    const OpenFileId OUTPUT = CONSOLE_OUTPUT;
    char             line[MAX_LINE_SIZE];
    char            *argv[MAX_ARG_COUNT];
    int argResult;

    for (;;) {
        WritePrompt(OUTPUT);
        const unsigned lineSize = ReadLine(line, MAX_LINE_SIZE, INPUT);
        if (lineSize == 0)
            continue;

        if ((argResult = PrepareArguments(line, argv, MAX_ARG_COUNT)) <= 0) {
			switch(argResult){
				case ERR_NULL_LINE:
					WriteError("line pointer is null.", OUTPUT);
					break;
				case ERR_NULL_ARGV:
					WriteError("arguments pointer is null.", OUTPUT);
					break;
				case ERR_ARGV_ZERO:
					WriteError("argument size is zero.", OUTPUT);
					break;
				case ERR_BAD_SYNTAX:
					WriteError("syntax error.", OUTPUT);
					break;
				case ERR_TOO_MANY_ARGS:
					WriteError("too many arguments.", OUTPUT);
					break;
			}
			continue;
        }

        if(argv[0][0] == PARALLEL_MARKER){
			argv[0]++;

			// Comment and uncomment according to whether command line arguments
			// are given in the system call or not.
			char debugMsg1[] = "Executing program...\n";
			Write(debugMsg1, sizeof debugMsg1 - 1, OUTPUT);
			
			
			const SpaceId newProc = Exec(line);
			//~ const SpaceId newProc = Exec(line, argv);	
		}
        else{
			// Comment and uncomment according to whether command line arguments
			// are given in the system call or not.
			const SpaceId newProc = Exec(line);
			//~ const SpaceId newProc = Exec(line, argv);
			
			Join(newProc);
		}
        
        

        // TO DO: check for errors when calling `Exec`; this depends on how
        //        errors are reported.

        // TO DO: is it necessary to check for errors after `Join` too, or
        //        can you be sure that, with the implementation of the system
        //        call handler you made, it will never give an error?; what
        //        happens if tomorrow the implementation changes and new
        //        error conditions appear?
    }

    return 0;  // Never reached.
}
