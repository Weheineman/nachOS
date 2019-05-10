#include "syscall.h"

#define NULL  ((void *) 0)

static inline unsigned
strlen(const char *s)
{
    if(s == NULL) return 0;
    
    unsigned i;
    for (i = 0; s[i] != '\0'; i++);
    return i;
}

int
main(int argc, char **argv)
{
	char prompt[] = "Echoing: ";
	char middle[] = " ";
	char end[] = "\n";
	int i;
	
    Write(prompt, strlen(prompt), CONSOLE_OUTPUT);
    for(i = 1; i < argc; i++){
		Write(argv[i], strlen(argv[i]), CONSOLE_OUTPUT);
		Write(middle, strlen(middle), CONSOLE_OUTPUT);
	}   
    Write(end, strlen(end), CONSOLE_OUTPUT);
    
    Exit(0);

}
