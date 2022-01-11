// For handling things:
#include <iostream>		// printing messages

// For MMAP:
#include <sys/mman.h>	// MMAP
#include <sys/stat.h>	// Stats of files for MMAP

// For file management:
#include <fcntl.h>
#include <unistd.h>

// Takes in pointer of pages, checks for spaces and newlines.
// Output wordCount, which is the writing between these.
unsigned long wordCounter(char *page){
	unsigned long wordCount = 0;
	for(long i = 1; page[i]; i++){
        if (isspace(page[i]))
			// Check if prev page is a empty space then skip if it is.
            if (!isspace(page[i-1])){
				wordCount++;
			}
		}
	return wordCount;
	}

// Takes in pointer of pages, checks for newlines.
// Output newLine, which is the amount of newlines in the text.
unsigned long newCounter(char *page){
	unsigned long newLine = 0;
	for(long i = 0; page[i]; i++){
		if (page[i] == '\n'){
			newLine++;
		}
	}
	return newLine;
}

int main(int argc, char** argv){
	//Some info for the user
	if (argc != 2) {
		std::cout << "Usage: mmapwc <FILENAME>\n";
		return 1;
	}

	// Used variables
	unsigned long wordCount = 0;
	unsigned long newLine = 0;
	const char *filepath = argv[1];
	size_t length;
	
	// Checking if the file exists
	int fd = open(filepath, O_RDONLY);
	if (fd < 0){
		std::cout << "Error reading file\n";
		return 1;
	}
	
	// File size for mmap:
	struct stat sb;
	if (fstat(fd, &sb) == -1){
		std::cout << "File doesn't have anything in it\n";
		return 1;
	}
	length = sb.st_size;
	
	// Create the wanted pointer to the memoryslot
	char* dataPointer = static_cast<char*>(mmap(NULL,length,PROT_READ,MAP_SHARED,fd,0));
	if (dataPointer == MAP_FAILED){
		std::cout << "0 0 0 " << argv[1] << "\n";
		return 1;
	}
	wordCount += wordCounter(dataPointer);
	newLine += newCounter(dataPointer);
	// Cleanup crew
	close(fd);

	std::cout << "  " << newLine << "  " << wordCount << "  " << length << " " << argv[1] << "\n";
	return 0;
}
