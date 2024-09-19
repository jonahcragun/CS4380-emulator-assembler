#ifndef EMU4380_H
#define EMU4380_H
#include <string>

#define DEFAULT_MEM_SIZE 131072
#define MAX_MEM_SIZE 4294967295
#define NUM_REGS 22
#define NUM_CNTRL_REGS 5
#define NUM_DATA_REGS 2
#define MAX_BYTE 255
#define NUM_INSTR 18

// declare units of emulator
extern unsigned int reg_file [];
extern unsigned char* prog_mem;
extern unsigned int mem_size;
extern unsigned int cntrl_regs[];
extern unsigned int data_regs[];
// true if system is running, false if halt is read
extern bool running;

// enums to access above arrays
enum RegNames {R0=0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, PC, SL, SB, SP, FP, HP};
enum CntrlRegNames {OPERATION=0, OPERAND_1, OPERAND_2, OPERAND_3, IMMEDIATE};
enum DataRegNames {REG_VAL_1=0, REG_VAL_2};

// instructions 
enum Instructions {JMP=1, MOV=7, MOVI, LDA, STR, LDR, STB, LDB, ADD=18, ADDI, SUB, SUBI, MUL, MULI, DIV, SDIV, DIVI, TRP=31};
enum Traps {HALT=0, WINT, RINT, WCHAR, RCHAR, PREGS=98};

// execution
bool fetch();
bool decode();
bool execute();

// initialization
bool init_mem(unsigned int);
unsigned int read_file(std::string);

// deleteor for memory
void delete_mem();

#endif
