void TestSimpleManyFiles(){
	char file1[] = "Test 1";
	char file2[] = "Test 2";
	
	unsigned count = 100;
	char contents[] = "42069";
	unsigned size = sizeof contents - 1 * count;
	
	if(not fileSystem -> Create(file1, size)){
		printf("Cannot create test file %s\n", file1);
		return;
	}
	OpenFile *openFile1 = fileSystem->Open(file1);
    if(openFile1 == nullptr) {
        printf("Unable to open test file %s\n", file1);
        return;
    }
    
	
	if(not fileSystem -> Create(file2, size)){
		printf("Cannot create test file %s\n", file2);
		return;
	}
	OpenFile *openFile2 = fileSystem->Open(file2);
    if(openFile2 == nullptr) {
        printf("Unable to open test file %s\n", file2);
        return;
    }

	unsigned i;
	for (i = 0; i < count; i++) {
		unsigned numBytes = openFile1->Write(contents, size);
		if (numBytes < size) {
			printf("Unable to write on test file %s on iteration %d\n", file1, i);
			break;
		}
		numBytes = openFile2->Write(contents, size);
		if (numBytes < size) {
			printf("Unable to write on test file %s on iteration %d\n", file2, i);
			break;
		}
	}

	delete openFile1;
	delete openFile2;
	
	if(i == count)
		printf("-- TestSimpleMultipleFiles successful!\n\n\n");
	else
		printf("!!!! TestSimpleMultipleFiles unsuccessful: Writers failed to write correctly.\n\n\n");
}
