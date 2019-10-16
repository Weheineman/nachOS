#include "open_file_list.hh"

OpenFileList::OpenFileList()
{
    listLock = new Lock;
    first = last = nullptr;
}

OpenFileList::~OpenFileList()
{
    FileMetadataNode* aux;

    while(first != nullptr){
		aux = first -> next;
		delete first -> name;
		delete first -> lock;
		delete first;
		first = aux;
	}
    delete listLock;
}

// Adds the file to the open file list.
// If the file is already open:
//      and is pending removal, it does nothing.
//      else it increases openInstances by 1.
ReaderWriter*
OpenFileList::AddOpenFile(const char *fileName){
    listLock -> Acquire();

	ReaderWriter fileRW = nullptr;

	FileMetadataNode* node = FindOpenFile(fileName);
	if(node != nullptr){
		if(not node -> pendingRemove){
			node -> openInstances++;
            fileRW = node -> lock;
        }

	}else{
		if(IsEmpty())
			first = last = CreateNode(fileName);
		else{
			last -> next = CreateNode(fileName);
			last = last -> next;
		}

        fileRW = last -> node;
	}

    listLock -> Release();
	return fileRW;
}


void
OpenFileList::CloseOpenFile(const char *fileName){
    listLock -> Acquire();

	FileMetadataNode* node = FindOpenFile(fileName);
	if(node != nullptr){
		if(node -> openInstances > 1)
			node -> openInstances--;
		else
			DeleteNode(node);
	}

    listLock -> Release();
}


// Returns true if the file is currently open, in which case
// SetUpRemoval sets pendingRemove to true atomically.
// If the file is not open, it just returns false.
bool SetUpRemoval(const char *fileName){
    listLock -> Acquire();

    bool fileIsOpen;
    FileMetadataNode* node = FindOpenFile(fileName);
	if(node != nullptr){
        node -> pendingRemove = true;
        fileIsOpen = true;
   }else
        fileIsOpen = false;

    listLock -> Release();
    return fileIsOpen;
}

FileMetadataNode*
OpenFileList::FindOpenFile(const char *fileName){
	FileMetadataNode* aux;
	for(aux = first;
	    aux != nullptr and not strncmp(aux -> name, fileName, FILE_NAME_MAX_LEN);
	    aux = aux -> next);

	return aux;
}

bool
OpenFileList::IsEmpty(){
	return first == nullptr;
}

FileMetadataNode*
OpenFileList::CreateNode(const char* fileName){
	FileMetadataNode* node = new FileMetadataNode;
	node -> name = new char [FILE_NAME_MAX_LEN];
	strcpy(node -> name, fileName);
	node -> lock = new ReaderWriter();
	node -> openInstances = 1;
	node -> pendingRemove = false;

	return node;
}


void
OpenFileList::DeleteNode(FileMetadataNode* target){
	FileMetadataNode* aux = nullptr;
	//If the first item is to be deleted, advance the first pointer.
	if(first == target)
		first = first -> next;

	else{
		for(aux = first; aux -> next != target; aux = aux -> next);

		aux -> next = aux -> next -> next;
	}

	//If the last item is to be deleted, bring the last pointer one item back.
	if(last == target)
		last = aux;


    if(target -> pendingRemove)
        fileSystem -> DeleteFromDisk(target -> name);
    

	delete [] target -> name;
	delete target -> lock;
	delete -> target;
}
