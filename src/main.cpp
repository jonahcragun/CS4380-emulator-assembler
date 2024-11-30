#include <iostream>
#include <fstream>
#include "../include/emu4380.h"
#include <exception>
#include <cstring>
using std::string;
using std::cout;
using std::endl;


#define MIN_ARGS 2
#define TAG_POS 2
#define FILE_POS 1

// verify cl arguments are valid
int args_valid(int argc, char** argv) {
    // contains 1-2 arguments
    if (argc < MIN_ARGS) {
        std::cout << "Invalid arguments entered." << std::endl;
        return 1;
    }

    // check file exists
    std::ifstream ifs (argv[FILE_POS]);
    if (ifs.fail()) {
        ifs.close();
        std::cout << "Invalid arguments entered." << std::endl;
        return 1;
    }
    ifs.close();

    // validate flags
    unsigned int mem_size = DEFAULT_MEM_SIZE;
    unsigned int cache_config = 0;
    for (int i = TAG_POS; i < argc; i+=2) {
        if (!strcmp(argv[i], "-m")) {
            // valid memory size
            try {
                if (i+1 >= argc || std::stoul(*(argv + i + 1)) > 4294967295) throw std::range_error("out of bounds");
                mem_size = atoi(*(argv + i + 1));
            }
            catch (...) {
                cout << "Invalid memory configuration. Aborting.\n";
                return 2;
            }
        }
        else if (!strcmp(argv[i], "-c")) {
            try {
                if (i+1 >= argc || std::stoi(*(argv + i + 1)) < 0 || std::stoi(*(argv + i + 1)) > 3) throw std::range_error("out of bounds");
                cache_config = atoi(*(argv + i + 1));
            }
            catch(...) {
                cout << "Invalid cache configuration. Aborting.\n"; 
                return 2;
            }
        }
        else {
            cout << "Invalid arguments entered." << endl;
            return 1;
        }
    }

    // initialize memory and cache
    init_mem(mem_size);
    init_cache(cache_config);

    // is valid
    return 0;
}


int main(int argc, char* argv[]) {
    // validate args
    int ret = args_valid(argc, argv);
    if (ret != 0) {
        return ret;
    }

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

    // init stack regs
    init_stack(argv[FILE_POS]);

    // initialize pc
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);

    // execution loop
    while (running) {
        bool fret = fetch();
        bool dret = decode();
        bool eret = execute();

        if (!fret || !dret || !eret || reg_file[SP] < reg_file[SL] || reg_file[SP] >= reg_file[SB]) break;
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
