DirectoryLockNode::DirectoryLockNode(int _sector)
{
    sector = _sector;
    next = nullptr;
    lock = new ReaderWriter();
    waiting = 0;
}

DirectoryLockNode::~DirectoryLockNode()
{
    free lock;
}

DirectoryLockManager::DirectoryLockManager()
{
    first = last = nullptr;
    managerLock = new Lock("Directory Manager");
}


DirectoryLockManager::~DirectoryLockManager()
{
    delete managerLock;
}


void
DirectoryLockManager::AcquireRead(int sector)
{
    Acquire(sector, false);
}


void
DirectoryLockManager::AcquireWrite(int sector)
{
    Acquire(sector, true);
}

// If writePermission is set to true, it acquires the writer lock. Else,
// it acquires the reader lock.
void
DirectoryLockManager::Acquire(int sector, bool writePermission)
{
    managerLock -> Acquire();
    DirectoryLockNode *node = FindNode(sector);
    if(node == nullptr){
        // If the directory isn't in the list, it is added with the
        // corresponding permission taken.
        AddNode(sector, writePermission);
        managerLock -> Release();
    }else{
        node -> waiting++;
        managerLock -> Release();

        if(writePermission)
            node -> AcquireWrite();
        else
            node -> AcquireRead();

        managerLock -> Acquire();
        node -> waiting--;
        managerLock -> Release();
    }
}

void
DirectoryLockManager::ReleaseRead(int sector)
{
    Release(sector, false);
}

void
DirectoryLockManager::ReleaseRead(int sector)
{
    Release(sector, true);
}

// If writePermission is set to true, it releases the writer lock. Else,
// it releases the reader lock.
void
DirectoryLockManager::Release(int sector, bool writePermission)
{
    managerLock -> Acquire();
    DirectoryLockNode *node = FindNode(sector);

    // The lock to be released should always be in the list.
    ASSERT(node != nullptr);

    if(node -> waiting == 0)
        DeleteNode(node);
    else{
        if(writePermission)
            node -> lock -> ReleaseWrite();
        else
            node -> lock -> ReleaseRead();
    }

    managerLock -> Release();
}


// Returns the node corresponding to the sector. If there is none,
// it returns nullptr instead.
DirectoryLockNode*
DirectoryLockManager::FindNode(int sector)
{
    for(DirectoryLockNode *current = first; current != nullptr;
        current = current -> next)
        if(current -> sector == sector)
            return current;

    return nullptr;
}


// Adds a node to the list with the given sector. If writePermission is
// set to true, it acquires the writer lock. Else, it acquires the reader
// lock.
void
DirectoryLockManager::AddNode(int sector, bool writePermission)
{
    DirectoryLockNode *newNode = new DirectoryLockNode(sector);

    if(writePermission)
        newNode -> lock -> AcquireWrite();
    else
        newNode -> lock -> AcquireRead();

    last -> next = newNode;
    last = newNode;
}


// Removes a node from the list.
void
DirectoryLockManager::DeleteNode(DirectoryLockNode *target)
{
    //If the first item is to be deleted, advance the first pointer.
    if(first == target)
        first = first -> next;


    DirectoryLockNode* previous = nullptr;
    else{
        for(previous = first; previous -> next != target;
            previous = previous -> next);

        previous -> next = target -> next;
    }

    //If the last item is to be deleted, bring the last pointer one item back.
    if(last == target)
        last = previous;

    delete target;
}
