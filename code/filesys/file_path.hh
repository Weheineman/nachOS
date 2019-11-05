#ifndef NACHOS_FILESYS_FILE_PATH__HH
#define NACHOS_FILESYS_FILE_PATH__HH

#include "open_file.hh"


struct PathNode{
	char name[FILENAME_NAME_MAX_LEN];
	PathNode *next, *prev;
};


class FilePath{
public:
	// Creates a file path from a string that describes it, 
	// or sets it to an empty path if NULL is given.
	FilePath(char *pathString);
	~FilePath();
	// Given a string for a root, appends the current path after it.
	void Prepend(char *root);
	// Appends a given path to the current one.
	void Append(char *leaf);
	// Returns a string describing the current path.
	char *ToString();
	// Returns whether the current path is a terminal node.
	bool IsBottomLevel();
	// Returns whether the path is empty.
	bool IsEmpty();
	// Splits the root node, returning it as a string
	// and removing it from the current path.
	char *SplitBottomLevel();
	// Given a string describing a path, appends it
	// to the current path.
	void Merge(char *pathString);

private:
	PathNode *first, *last;
};
#endif
