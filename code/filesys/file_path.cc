#include "file_path.hh"


// Creates a file path from a string that describes it, 
// or sets it to an empty path if NULL is given.
FilePath::FilePath(char *pathString){
	if(pathString == nullptr)
		first = last = nullptr;
	else{
		first = new PathNode;
		first -> name[0] = '\0';
		first -> prev = nullptr;
		
		PathNode *previousNode = first;
		
		for(unsigned idx = 1; pathString[idx] != '\0'; idx ++){
			char buffer[FILE_NAME_MAX_LEN + 2];
			unsigned bufferIdx = 0;
			while(pathString[idx] != '/' and pathString[idx] != '\0'){
				buffer[bufferIdx] = pathString[idx];
				bufferIdx ++;
				idx ++;
			}
			
			buffer[bufferIdx] = '\0';
			previousNode -> next = new PathNode;
			previousNode -> next -> prev = previousNode;
			strcpy(previousNode -> next -> name, buffer);
			previousNode = previousNode -> next;
			
			if(pathString[idx] == '\0')
				idx --;
			
		}
		
		last = previousNode;
		last -> next = nullptr;		
	}
}


FilePath::~FilePath(){
	while(first != nullptr){
		first = first -> next;
		free(first -> prev);
	}
}

// Given a string for a root, appends the current path after it.
void
FilePath::Prepend(char *root){
	PathNode *aux = new PathNode;
	strcpy(aux -> name, root);
	aux -> next = first;
	aux -> prev = nullptr;
	
	if(IsEmpty())
		first = last = aux;
	else{
		first -> prev = aux;
		first = aux;	
	}
}

// Appends a given path to the current one.
void
FilePath::Append(char *leaf){
	PathNode *aux = new PathNode;
	strcpy(aux -> name, leaf);
	aux -> prev = last;
	aux -> next = nullptr;
	
	if(IsEmpty())
		first = last = aux;
	else{
		last -> next = aux;
		last = aux;
	}
}

// Returns a string describing the current path.
char *
FilePath::ToString(){
	if(IsEmpty())
		return nullptr;
	
	PathNode *aux = first;
	unsigned counter = 0;
	
	while(aux != nullptr){
		aux = aux -> next;
		counter ++;
	}
	
	char *result = new char[FILE_NAME_MAX_LEN * counter + counter];
	result[0] = '\0';
	
	aux = first;
	while(aux != nullptr){
		strcat(result, aux -> name);
		strcat(result, "/");
		aux = aux -> next;
	}
	
	unsigned length = strlen(result);
	if(length > 1){
		result[length - 1] = '\0';
	}
	
	return result;
}

// Returns whether the current path is a terminal node.
bool 
FilePath::IsBottomLevel(){
	if(IsEmpty())
		return false;
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

// Given a string describing a path, appends it
// to the current path.
void 
FilePath::Merge(char *pathString){}
