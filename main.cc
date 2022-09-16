// Playing with mmap and madvise.
// mmaps, a file then uses madvise to remove pages from it and confirms
// that those pages are zero filled as prescribed by the man pages.

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <limits>

// The makefile generates this file by filtering alphabetic characters from
// dev/urandom.
constexpr char kFileName[] = "64k-random.txt";

// Print format
// 0xNN 0xNN 0xNN 0xNN 0xNN 0xNN 0xNN 0xNN
// ...
// 0xNN 0xNN 0xNN 0xNN 0xNN 0xNN 0xNN 0xNN
void hexprint(void* p, size_t len) {
    char* c = static_cast<char*>(p);
    for (size_t i = 1; i <= len; i++) {
        printf("0x%02x ", *c);
        // Newline after printing 8 bytes.
        if (i % 8 == 0) {
            printf("\n");
        }
        c++;
    }
}

int main(int argc, char* argv[]) {
    // madvise works in granularity of pages, passing args not in pages
    // returns an error.
    const int page_size = getpagesize();
    std::cout << "Page size is " << page_size << std::endl;

    std::cout << "Opening " << kFileName << std::endl;

    int fd = open(kFileName, O_RDWR);
    if (fd < 0) {
        std::cerr << "Failed to open file" << std::endl;
        return fd;
    }

    const off_t length = lseek(fd, 0, SEEK_END);
    std::cout << "File length: " << length << "b\n" << std::endl;

    // Mapping must be shared + writable in order to madvise remove
    void* mapping = mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        std::cerr << "Mapping failed: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    void* removed_page = static_cast<void*>(static_cast<char*>(mapping) + page_size);
    int result = madvise(removed_page, page_size, MADV_REMOVE);
    if (result < 0) {
        std::cerr << "Failed to madvise: " << strerror(errno) << std::endl;
        close(fd);
        return result;
    }

    // The pages that get MADVISE removed should be zeroes whereas the
    // rest of the mmaped region should still be accessible.
    std::cout << "mmaped file:" << std::endl;
    hexprint(mapping, 16);

    std::cout << "\nMAD_REMOVEd pages" << std::endl;
    hexprint(removed_page, 16);

    munmap(mapping, length);
    close(fd);
    return 0;
}