#include "file_path.hh"

// Creates a file path from a string that describes it,
// or sets it to an empty path if NULL is given.
FilePath::FilePath(char *pathString){
	first = last = nullptr;
	if(pathString != nullptr and *pathString != '\0' and strcmp(pathString, "/"))
		MergeString(pathString);
}

FilePath::~FilePath(){
	Clear();
}

// Returns a string describing the current path.
char *
FilePath::ToString(){
	if(IsEmpty()){
		char * result = new char[2];
		strcpy(result, "/");
		return result;
	}

	PathNode *aux = first;
	unsigned counter = 0;

	while(aux != nullptr){
		aux = aux -> next;
		counter ++;
	}

	char *result = new char[FILE_NAME_MAX_LEN * counter + counter + 1];
	result[0] = '\0';

	aux = first;
	while(aux != nullptr){
		strcat(result, "/");
		strcat(result, aux -> name);
		aux = aux -> next;
	}

	return result;
}

// Returns whether the current path is a terminal node.
bool
FilePath::IsBottomLevel(){
	if(IsEmpty())
		return true;
	return first -> next == nullptr;
}

// Returns whether the path is empty.
bool
FilePath::IsEmpty(){
	return first == nullptr;
}

// Splits the root node, returning it as a string
// and removing it from the current path.
char *
FilePath::SplitBottomLevel(){
	if(IsEmpty())
		return nullptr;

	char *result = new char[FILE_NAME_MAX_LEN + 1];

	strcpy(result, first -> name);

	PathNode *aux = first;
	if(first == last)
		first = last = nullptr;
	else
		first = first -> next;

	delete aux;
	return result;
}

// Given a string describing a path,
// - if the string represents a global path, sets the current path to it.
// - if not, appends the path described by the string to the current path.
void
FilePath::Merge(char *pathString){
	if(*pathString == '/')
		Clear();

	MergeString(pathString);
}

// Resets the path to a null path.
void
FilePath::Clear(){
	PathNode *aux;
	while(first != nullptr){
		aux = first;
		first = first -> next;
		delete aux;
	}
	first = last = nullptr;
}

// Appends the path described by the given string to the current path.
void
FilePath::MergeString(char *pathString){
	if(*pathString == '/')
		pathString ++;

	PathNode *previousNode = last;

	for(unsigned idx = 0; pathString[idx] != '\0'; idx ++){
		char buffer[FILE_NAME_MAX_LEN + 1];
		unsigned bufferIdx = 0;
		while(pathString[idx] != '/' and pathString[idx] != '\0'){
			buffer[bufferIdx] = pathString[idx];
			bufferIdx ++;
			idx ++;
		}
		buffer[bufferIdx] = '\0';

		if(strcmp(buffer, ".")){
			if(strcmp(buffer, "..")){
				if(previousNode == nullptr){
					first = new PathNode;
					first -> prev = nullptr;
					strcpy(first -> name, buffer);
					previousNode = first;
				}
				else{
					previousNode -> next = new PathNode;
					previousNode -> next -> prev = previousNode;
					strcpy(previousNode -> next -> name, buffer);
					previousNode = previousNode -> next;
				}
			}
			else{
				if(first != nullptr){
					if(previousNode == first){
						delete first;
						previousNode = first = nullptr;
					}
					else{
						previousNode = previousNode -> prev;
						delete previousNode -> next;
					}
				}
			}

		}

		if(pathString[idx] == '\0')
			idx --;
	}

	last = previousNode;
	if(last != nullptr)
		last -> next = nullptr;

}
