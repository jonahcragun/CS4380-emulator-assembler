#include <iostream>
#include <fstream>
#include "../include/emu4380.h"
using std::string;
using std::cout;
using std::endl;


#define MAX_ARGS 3
#define MIN_ARGS 2
#define MEM_SIZE_POS 2
#define FILE_POS 1

// verify cl arguments are valid
bool args_valid(int argc, char** argv) {
    // contains 1-2 arguments
    if (argc > MAX_ARGS || argc < MIN_ARGS) return false;
    // check file exists
    std::ifstream ifs (argv[FILE_POS]);
    if (ifs.fail()) {
        ifs.close();
        return false;
    }
    ifs.close();
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


int main(int argc, char* argv[]) {
    // validate args
    if (!args_valid(argc, argv)) {
        std::cout << "invalid arguments entered" << std::endl;
        return 1;
    }

    // initialize emulator memory
    unsigned mem_size = 0;
    if (argc == MAX_ARGS) {
        mem_size = std::stoul(argv[MEM_SIZE_POS]);
    }
    else {
        mem_size = DEFAULT_MEM_SIZE;
    }

    init_mem(mem_size);

    // read in file 
    unsigned int file_read = read_file(argv[FILE_POS]);

    if (file_read == 1) {
        delete_mem();
        cout << "INVALID FILE FORMAT" << endl;
        return file_read;
    }
    else if (file_read == 2) {
        delete_mem();
        cout << "INSUFFICIENT MEMORY SPACE\n";
        return file_read;
    }

    // initialize pc
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);

    // execution loop
    while (running) {
        bool fret = fetch();
        bool dret = decode();
        bool eret = execute();

        if (!fret || !dret || !eret) break;
    }
    
    delete_mem();

    if (running) {
        cout << "INVALID INSTRUCTION AT: " << reg_file[PC]-8 << "\n";
        return 1;
    }
    else {
        return 0;
    }
}
