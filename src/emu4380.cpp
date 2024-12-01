#include "../include/emu4380.h"
#include <fstream>
#include <cstddef>
#include <iostream>
#include <vector>
#include "../include/cache.h"
#include <filesystem>
using std::string;
using std::cout;
using std::endl;
using std::cin;
using std::vector;

// *****
// cache
// *****


void init_cache(unsigned int cacheType) {
    for (int i = 0; i < CACHE_SIZE; ++i)  {
        cache[i].valid = false;
        cache[i].dirty = false;
    }
    cache_counter = 0;
    
    if (cacheType == 0) {
        // indicates no cache is being used
        cache_set_size = 0;
    }
    else if (cacheType == 1) {
        // direct mapped
        cache_set_size = 1;
    }
    else if (cacheType == 2) {
        // fully associative
        cache_set_size = CACHE_SIZE;
    }
    else if (cacheType == 3) {
        // cache is set to set size of 2
        cache_set_size = 2;
    }
}

// *************
// variable defs
// *************
unsigned char* prog_mem = nullptr;
unsigned int mem_size = 0;
unsigned int reg_file [NUM_REGS] = {0};
unsigned int cntrl_regs[NUM_CNTRL_REGS] = {0};
unsigned int data_regs[NUM_DATA_REGS] = {0};
bool running = true;

// define cache measurement vars
unsigned int mem_cycle_cntr = 0;

// *************
// Memory access
// *************


// get byte from memory
unsigned char readByte(unsigned int address) {
    if (!cache_set_size) {
        unsigned char c;
        c = prog_mem[address];
        mem_cycle_cntr += 8;
        return c;
    }
    else {
        cache_byte cb = get_cache_byte(address);
        mem_cycle_cntr += cb.penalty;
        return cb.byte;
    }
}

// get int  from memory
unsigned int readWord(unsigned int address) {
    if (!cache_set_size) {
        unsigned int i;
        i = *reinterpret_cast<unsigned int*>(prog_mem + address);
        mem_cycle_cntr += 8;
        return i;
    }
    else {
        cache_word cw = get_cache_words(address);
        mem_cycle_cntr += cw.penalty;
        return cw.words[0];
    }
}

// store byte in memory
void writeByte(unsigned int address, unsigned char byte) {
    if (!cache_set_size) {
        unsigned char c;
        prog_mem[address] = byte;
        mem_cycle_cntr += 8;
    }
    else {
        cache_byte cb = write_cache_byte(address, byte);
        mem_cycle_cntr += cb.penalty;
    }
}

// store int in  memory
void writeWord(unsigned int address, unsigned int word) {
    if (!cache_set_size) {
        unsigned int i;
        *reinterpret_cast<unsigned int*>(prog_mem + address) = word;
        mem_cycle_cntr += 8;
    }
    else {
        unsigned int penalty = write_cache_word(address, word);
        mem_cycle_cntr += penalty;
    }
}

// read multiple words at a time (first word increments counter by 8, all others increments counter by 2)
// returns a vector containing num_words values
vector<unsigned int> readWords(unsigned int address, unsigned int num_words) {
    if (!cache_set_size) {
        vector<unsigned int> words;
        for (int i = 0; i < num_words; ++i) {
            words.push_back(*reinterpret_cast<unsigned int*>(prog_mem + address + i * WORD_SIZE));
        }
        mem_cycle_cntr += 8 + 2 * (num_words - 1);
        return words;
    }
    else {
        cache_word cw = get_cache_words(address, num_words);
        mem_cycle_cntr += cw.penalty;
        return cw.words;
    }
}

// ****************
// inititialization
// ****************

// init stack regs
void init_stack(string file) {
    std::filesystem::path p {file};
    reg_file[SB] = mem_size + 1; 
    reg_file[SP] = mem_size + 1;
    reg_file[SL] = std::filesystem::file_size(p) + 1;
    reg_file[HP] = std::filesystem::file_size(p) + 1;
}

// initialize memory array
bool init_mem(unsigned int size) {
    prog_mem = new unsigned char[size]();
    mem_size = size;
    return true;
}

void delete_mem() {
    delete [] prog_mem;
    prog_mem = nullptr;
}

// read bin files contents into memory
unsigned int read_file(string file) {
    std::ifstream ifs (file, std::ios::binary);
    // check if file opened
    if (!ifs) return 3;

    // set end
    ifs.seekg(0, std::ios_base::end);
    auto len = ifs.tellg();
    // check if file is too large
    if (len > mem_size) return 2;

    // read from file
    ifs.seekg(0, std::ios_base::beg);
    ifs.read((char*)prog_mem, len);

    ifs.close();
    return 0;
}


// ***********
// execution
// ***********

// retrieve bytes of instruction
bool fetch() {
    // check if addr is valid 
    if (reg_file[PC] + 7 > mem_size) return false;

    // get instruction from memory (2 words)
    vector<unsigned int> words = readWords(reg_file[PC], 2);

    // move instruction values to correct cntrl_reg position
    for (size_t i = 0; i < WORD_SIZE; ++i) {
        unsigned int word = words.at(0);
        cntrl_regs[i] = *(reinterpret_cast<unsigned char*>(&word) + i); 
    }
    cntrl_regs[IMMEDIATE] = words.at(1);
    
    // increment pc 8 bytes (1 instruction)
    reg_file[PC] += 8;

    return true;
}

bool decode() {
    // verify instructions are valid
    switch (cntrl_regs[OPERATION]) {
        case(JMP):
            break;
        case(MOV):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(MOVI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            break;
        case(LDA):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            break;
        case(STR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]];
            break;
        case(LDR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            break;
        case(STB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = *reinterpret_cast<unsigned char*>(&reg_file[cntrl_regs[OPERAND_1]]);
            break;
        case(LDB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            break;
        case(ADD):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(ADDI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(SUB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(SUBI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(MUL):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(MULI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(DIV):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(SDIV):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(DIVI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(TRP):
            // traps include immediate values 0-4 and 98
            if (cntrl_regs[IMMEDIATE] > RSTR && cntrl_regs[IMMEDIATE] != PREGS) return false;
            if (cntrl_regs[IMMEDIATE] == WINT || cntrl_regs[IMMEDIATE] == WSTR || cntrl_regs[IMMEDIATE] == RSTR) {
                data_regs[REG_VAL_1] = reg_file[R3];
            }
            else if (cntrl_regs[IMMEDIATE] == WCHAR) {
                data_regs[REG_VAL_1] = *reinterpret_cast<unsigned char*>(&reg_file[R3]);
            }
            break;
        case(ISTR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(ILDR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]]; 
            break;
        case(ISTB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(ILDB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]]; 
            break;
        case(JMR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            break;
        case(BNZ):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            break;
        case(BGT):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            break;
        case(BLT):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            break;
        case(BRZ):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            break;
        case(CMP):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]]; 
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]]; 
            break;
        case(CMPI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]]; 
            break;
        case(AND):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(OR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(ALCI):
        case(ALLC):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[HP];
            break;
        case(IALLC):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]]; 
            data_regs[REG_VAL_2] = reg_file[HP]; 
            break;
        case(PSHR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            data_regs[REG_VAL_2] = reg_file[SP]; 
            break;
        case(PSHB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]]; 
            data_regs[REG_VAL_2] = reg_file[SP]; 
            break;
        case(POPR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[SP]; 
            break;
        case(POPB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[SP]; 
            break;
        case(CALL):
            data_regs[REG_VAL_1] = reg_file[SP];
            data_regs[REG_VAL_2] = reg_file[PC];
            break;
        case(RET):
            data_regs[REG_VAL_1] = reg_file[SP];
            break;
        default:
            // invalid instruction
            return false;
    }

    return true;
}

// execute operation based on data regs and cntrl regs
bool execute() {
    string names[] {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "PC", "SL", "SB", "SP", "FP", "HP"};
    switch(cntrl_regs[OPERATION]) {
        case(JMP):
            reg_file[PC] = cntrl_regs[IMMEDIATE];
            break;
        case(MOV):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
            break;
        case(MOVI):
            reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
            break;
        case(LDA):
            reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
            break;
        case(STR):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            writeWord(cntrl_regs[IMMEDIATE], *reinterpret_cast<unsigned int*>(data_regs + REG_VAL_1));
            break;
        case(LDR):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            reg_file[cntrl_regs[OPERAND_1]] = readWord(cntrl_regs[IMMEDIATE]);
            break;
        case(STB):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            writeByte(cntrl_regs[IMMEDIATE], *reinterpret_cast<unsigned char*>(data_regs + REG_VAL_1));
            break;
        case(LDB):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            *reinterpret_cast<unsigned char*>(reg_file + cntrl_regs[OPERAND_1]) = readByte(cntrl_regs[IMMEDIATE]);
            break;
        case(ADD):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] + data_regs[REG_VAL_2];
            break;
        case(ADDI):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] + cntrl_regs[IMMEDIATE];
            break;
        case(SUB):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] - data_regs[REG_VAL_2];
            break;
        case(SUBI):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] - cntrl_regs[IMMEDIATE];
            break;
        case(MUL):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] * data_regs[REG_VAL_2];
            break;
        case(MULI):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] * cntrl_regs[IMMEDIATE];
            break;
        case(DIV):
            if (data_regs[REG_VAL_2] == 0) return false;
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] / data_regs[REG_VAL_2];
            break;
        case(SDIV):
            if (data_regs[REG_VAL_2] == 0) return false;
            reg_file[cntrl_regs[OPERAND_1]] = (int)data_regs[REG_VAL_1] / (int)data_regs[REG_VAL_2];
            break;
        case(DIVI):
            if (cntrl_regs[IMMEDIATE] == 0) return false;
            reg_file[cntrl_regs[OPERAND_1]] = (int)data_regs[REG_VAL_1] / (int)cntrl_regs[IMMEDIATE];
            break;
        case(TRP):
            switch (cntrl_regs[IMMEDIATE]) {
                case(HALT):
                    cout << "Execution completed. Total memory cycles: " << mem_cycle_cntr << "\n";
                    running = false;
                    break;
                case(WINT):
                    cout << data_regs[REG_VAL_1];
                    break;
                case(RINT):
                    cin >> reg_file[R3];
                    break;
                case(WCHAR):
                    cout << *reinterpret_cast<unsigned char*>(&data_regs[REG_VAL_1]);
                    break;
                case(RCHAR):
                    cin >> *reinterpret_cast<unsigned char*>(reg_file + R3);
                    break;
                case(WSTR):
                    for (int i = 1; i < prog_mem[data_regs[REG_VAL_1]] + 1; ++i) {
                        if (prog_mem[data_regs[REG_VAL_1] + i] == 0) break;
                        cout << prog_mem[data_regs[REG_VAL_1] + i];
                    }
                    break;
                case(RSTR): {
                    string str;
                    cin >> str;
                    prog_mem[data_regs[REG_VAL_1]] = str.size();
                    for (int i = 0; i < str.size(); ++i) {
                        prog_mem[data_regs[REG_VAL_1] + i + 1] = str[i];
                    }
                    prog_mem[data_regs[REG_VAL_1] + str.size() + 1] = 0;
                    break;
                }
                case(PREGS):
                    for (size_t i = 0; i < NUM_REGS; ++i) {
                        cout << names[i] << "\t" << reg_file[i] << endl; 
                    }
                    break;
                default:
                    // invalid trap
                    return false;
            }
            break;
        case(JMR):
            reg_file[PC] = data_regs[REG_VAL_1];
            break;
        case(BNZ):
            if (static_cast<int>(data_regs[REG_VAL_1]) != 0)
                reg_file[PC] = cntrl_regs[IMMEDIATE];
            break;
        case(BGT):
            if (static_cast<int>(data_regs[REG_VAL_1]) > 0)
                reg_file[PC] = cntrl_regs[IMMEDIATE];
            break;
        case(BLT):
            if (static_cast<int>(data_regs[REG_VAL_1]) < 0)
                reg_file[PC] = cntrl_regs[IMMEDIATE];
            break;
        case(BRZ):
            if (static_cast<int>(data_regs[REG_VAL_1]) == 0)
                reg_file[PC] = cntrl_regs[IMMEDIATE];
            break;
        case(ISTR):
            writeWord(data_regs[REG_VAL_2], data_regs[REG_VAL_1]);
            break;
        case(ILDR):
            reg_file[cntrl_regs[OPERAND_1]] = readWord(data_regs[REG_VAL_1]);
            break;
        case(ISTB):
            writeByte(data_regs[REG_VAL_2], data_regs[REG_VAL_1]);
            break;
        case(ILDB):
            reg_file[cntrl_regs[OPERAND_1]] = readByte(data_regs[REG_VAL_2]);
            break;
        case(CMP):
            if (static_cast<int>(data_regs[REG_VAL_1]) == static_cast<int>(data_regs[REG_VAL_2])) {
                reg_file[cntrl_regs[OPERAND_1]] = 0;
            }
            else if (static_cast<int>(data_regs[REG_VAL_1]) > static_cast<int>(data_regs[REG_VAL_2])) {
                reg_file[cntrl_regs[OPERAND_1]] = 1;
            }
            else {
                reg_file[cntrl_regs[OPERAND_1]] = -1;
            }
            break;
        case(CMPI):
            if (static_cast<int>(data_regs[REG_VAL_1]) == static_cast<int>(cntrl_regs[IMMEDIATE])) {
                reg_file[cntrl_regs[OPERAND_1]] = 0;
            }
            else if (static_cast<int>(data_regs[REG_VAL_1]) > static_cast<int>(cntrl_regs[IMMEDIATE])) {
                reg_file[cntrl_regs[OPERAND_1]] = 1;
            }
            else {
                reg_file[cntrl_regs[OPERAND_1]] = -1;
            }
            break;
        case(AND):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] && data_regs[REG_VAL_2];
            break;
        case(OR):
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] || data_regs[REG_VAL_2];
            break;
        case(ALCI):
            if ((data_regs[REG_VAL_1] + cntrl_regs[IMMEDIATE]) >= mem_size) return false; 
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
            reg_file[HP] = data_regs[REG_VAL_1] + cntrl_regs[IMMEDIATE];
            break;
        case(ALLC):
            if ((data_regs[REG_VAL_1] + *reinterpret_cast<unsigned int*>(prog_mem + cntrl_regs[IMMEDIATE])) >= mem_size) return false; 
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
            reg_file[HP] = data_regs[REG_VAL_1] + *reinterpret_cast<unsigned int*>(prog_mem + cntrl_regs[IMMEDIATE]);
            break;
        case(IALLC):
            if ((data_regs[REG_VAL_2] + *reinterpret_cast<unsigned int*>(prog_mem + cntrl_regs[IMMEDIATE])) >= mem_size) return false; 
            reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_2];
            reg_file[HP] = data_regs[REG_VAL_2] + *reinterpret_cast<unsigned int*>(prog_mem + data_regs[REG_VAL_1]);
            break;
        case(PSHR):
            reg_file[SP] = data_regs[REG_VAL_2] - 4;
            *reinterpret_cast<unsigned int*>(prog_mem + data_regs[REG_VAL_2] - 4) = data_regs[REG_VAL_1];
            break;
        case(PSHB):
            reg_file[SP] = data_regs[REG_VAL_2] - 1;
            prog_mem[data_regs[REG_VAL_2] - 1] = data_regs[REG_VAL_1];
            break;
        case(POPR):
            reg_file[cntrl_regs[OPERAND_1]] = *reinterpret_cast<unsigned int*>(prog_mem + data_regs[REG_VAL_1]);
            reg_file[SP] = data_regs[REG_VAL_1] + 4;
            break;
        case(POPB):
            reg_file[cntrl_regs[OPERAND_1]] = prog_mem[data_regs[REG_VAL_1]];
            reg_file[SP] = data_regs[REG_VAL_1] + 1;
            break;
        case(CALL):
            reg_file[SP] = data_regs[REG_VAL_1] - 4;
            *reinterpret_cast<unsigned int*>(prog_mem + data_regs[REG_VAL_1] - 4) = data_regs[REG_VAL_2];
            reg_file[PC] = cntrl_regs[IMMEDIATE];
            break;
        case(RET):
            reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem + data_regs[REG_VAL_1]);
            reg_file[SP] = data_regs[REG_VAL_1] + 4;
            break;
        default:
            // invalid instruction
            return false;
    }

    return true;
}
