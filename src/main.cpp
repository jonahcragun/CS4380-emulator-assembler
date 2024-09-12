#include <iostream>
#include "../include/emu4380.h"
using std::string;


#define MAX_ARGS 3
#define MIN_ARGS 2
#define MEM_SIZE_POS 2
#define FILE_POS 1
#define DEFAULT_MEM_SIZE 131072

// verify cl arguments are valid
bool args_valid(int argc, char** argv) {
    // contains 1-2 arguments
    if (argc > MAX_ARGS || argc < MIN_ARGS) return false;
    // valid memory size
    try {
        if (argc == MAX_ARGS && (std::stoul(*(argv + MEM_SIZE_POS)) > 4294967295 || std::stoul(*(argv + MEM_SIZE_POS)) < 0)) return false;
    }
    catch (...) {
        return false;
    }
    // is valid
    return true;
}

void init_sys(string file, unsigned mem_size) {
    return;
}

int main(int argc, char* argv[]) {
    // validate args
    if (!args_valid(argc, argv)) {
        std::cout << "invalid arguments entered" << std::endl;
        return 1;
    }

    // initialize emulator values
    unsigned mem_size = 0;
    if (argc == MAX_ARGS) {
        mem_size = std::stoul(argv[MEM_SIZE_POS]);
    }
    else {
        mem_size = DEFAULT_MEM_SIZE;
    }
    init_sys(argv[FILE_POS], mem_size);

    return 0;
}
