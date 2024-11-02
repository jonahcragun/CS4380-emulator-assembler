#include "../include/emu4380.h"
#include <fstream>
#include <cstddef>
#include <iostream>
using std::string;
using std::cout;
using std::endl;
using std::cin;

// *************
// variable defs
// *************
unsigned char* prog_mem = nullptr;
unsigned int mem_size = 0;
unsigned int reg_file [NUM_REGS] = {0};
unsigned int cntrl_regs[NUM_CNTRL_REGS] = {0};
unsigned int data_regs[NUM_DATA_REGS] = {0};
bool running = true;

// ****************
// inititialization
// ****************

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
    for (size_t i = OPERATION; i < IMMEDIATE; ++i) {
        size_t addr = reg_file[PC] + i; 
        cntrl_regs[OPERATION + i] = prog_mem[addr]; 
    }
    cntrl_regs[IMMEDIATE] = *reinterpret_cast<unsigned int*>(prog_mem + reg_file[PC] + IMMEDIATE);
    
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
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1) return false;
            break;
        case(LDA):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1) return false;
            break;
        case(STR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]];
            break;
        case(LDR):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1) return false;
            break;
        case(STB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_1]];
            break;
        case(LDB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1) return false;
            break;
        case(ADD):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(ADDI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(SUB):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(SUBI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(MUL):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(MULI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(DIV):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(SDIV):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1 || cntrl_regs[OPERAND_3] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            data_regs[REG_VAL_2] = reg_file[cntrl_regs[OPERAND_3]];
            break;
        case(DIVI):
            if (cntrl_regs[OPERAND_1] > NUM_REGS -1 || cntrl_regs[OPERAND_2] > NUM_REGS - 1) return false;
            data_regs[REG_VAL_1] = reg_file[cntrl_regs[OPERAND_2]];
            break;
        case(TRP):
            // traps include immediate values 0-4 and 98
            if (cntrl_regs[IMMEDIATE] > RCHAR && cntrl_regs[IMMEDIATE] != PREGS) return false;
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
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
            break;
        case(STR):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            *reinterpret_cast<unsigned int*>(prog_mem + cntrl_regs[IMMEDIATE]) = data_regs[REG_VAL_1];
            break;
        case(LDR):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            reg_file[cntrl_regs[OPERAND_1]] = *reinterpret_cast<unsigned int*>(prog_mem + cntrl_regs[IMMEDIATE]);
            break;
        case(STB):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            prog_mem[cntrl_regs[IMMEDIATE]] = *reinterpret_cast<unsigned char*>(data_regs + REG_VAL_1);
            break;
        case(LDB):
            if (cntrl_regs[IMMEDIATE] > mem_size) return false;
            *reinterpret_cast<unsigned char*>(reg_file + cntrl_regs[OPERAND_1]) = prog_mem[cntrl_regs[IMMEDIATE]];
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
                    running = false;
                    break;
                case(WINT):
                    cout << reg_file[R3];
                    break;
                case(RINT):
                    cin >> reg_file[R3];
                    break;
                case(WCHAR):
                    cout << *reinterpret_cast<unsigned char*>(reg_file + R3);
                    break;
                case(RCHAR):
                    cin >> *reinterpret_cast<unsigned char*>(reg_file + R3);
                    break;
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
        default:
            // invalid instruction
            return false;
    }

    return true;
}
