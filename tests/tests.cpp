#include <gtest/gtest.h>
#include <cstdio>
#include "../include/emu4380.h"
#include <string>
using std::string;

// test init_mem
TEST(init, ValidInput) {
    bool ret;
    ret = init_mem(DEFAULT_MEM_SIZE);
    EXPECT_EQ(ret, true);
    for (int i = 0; i < DEFAULT_MEM_SIZE; ++i) {
        EXPECT_EQ(*(prog_mem + i), 0);
    }
    delete_mem();

    ret = init_mem(MAX_MEM_SIZE);
    EXPECT_EQ(ret, true);
    delete_mem();
}

// test reading a file in to memory

TEST(ReadFile, ValidFile) {
    // set expected memory
    unsigned char expected_mem[DEFAULT_MEM_SIZE] {0};
    unsigned char vals[] = {8, 0, 0, 0, 
                            10, 0, 0, 0, 
                            11, 3, 0, 0, 4, 0, 0, 0,
                            19, 3, 3, 0, 10, 0, 0, 0, 
                            31, 0, 0, 0, 1, 0, 0, 0,
                            31, 0, 0, 0, 0, 0, 0, 0};
    for (size_t i = 0; i < 40; ++i) expected_mem[i] = vals[i];


    init_mem(DEFAULT_MEM_SIZE);
    unsigned int ret = read_file("../tests/test1.bin");
    EXPECT_EQ(ret, 0);
    for (size_t i=0; i<DEFAULT_MEM_SIZE; ++i) {
        EXPECT_EQ((unsigned int)prog_mem[i], expected_mem[i]);
    }
    delete_mem();
} 

TEST(ReadFile, InvalidFile) {
    init_mem(DEFAULT_MEM_SIZE);
    unsigned int ret = read_file("../test/no_exist.bin");
    EXPECT_EQ(ret, 3);
    delete_mem();

    init_mem(5);
    ret = read_file("../tests/test1.bin");
    EXPECT_EQ(ret, 2);
    delete_mem();
}

// testing fetching instruction
TEST(fetch, correctInstruction) {
    init_mem(DEFAULT_MEM_SIZE);
    // set mem values
    prog_mem[0] = 6;
    prog_mem[5] = 4;
    prog_mem[6] = 19;
    prog_mem[7] = 1;
    prog_mem[8] = 2;
    prog_mem[9] = 3;
    prog_mem[10] = 10;

    prog_mem[14] = 21;
    prog_mem[15] = 2;
    prog_mem[16] = 3;
    prog_mem[17] = 1;
    prog_mem[18] = 12;

    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);

    // retrieve instruction
    bool ret = fetch();

    EXPECT_EQ(ret, true);

    // verify value are correct in cntrl_reg
    EXPECT_EQ(cntrl_regs[OPERATION], 19);
    EXPECT_EQ(cntrl_regs[OPERAND_1], 1);
    EXPECT_EQ(cntrl_regs[OPERAND_2], 2);
    EXPECT_EQ(cntrl_regs[OPERAND_3], 3);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 10);

    ret = fetch();

    EXPECT_EQ(ret, true);

    // verify value are correct in cntrl_reg
    EXPECT_EQ(cntrl_regs[OPERATION], 21);
    EXPECT_EQ(cntrl_regs[OPERAND_1], 2);
    EXPECT_EQ(cntrl_regs[OPERAND_2], 3);
    EXPECT_EQ(cntrl_regs[OPERAND_3], 1);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 12);

    delete_mem();
}

TEST(fetch, correctPC) {
    init_mem(DEFAULT_MEM_SIZE);
    prog_mem[0] = 5;
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);
    
    bool ret = fetch();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 13);

    ret = fetch();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 21);   
}

TEST(fetch, outOfBounds) {
    init_mem(6);
    prog_mem[0] = 5;
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);

    bool ret = fetch();

    EXPECT_EQ(ret, false);
}

// testing validate instruction
TEST(decode, validInstructions) {
    init_mem(DEFAULT_MEM_SIZE);
    size_t fret = read_file("../tests/all_instr_test.bin");
    EXPECT_EQ(fret, 0);
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);
    
    for (size_t i = 0; i < 22; ++i) {
        fetch();
        bool ret = decode();
        EXPECT_EQ(ret, true) << "Instr: " << (unsigned int)cntrl_regs[OPERATION] << "; iter: " << i << "; imm: " << cntrl_regs[IMMEDIATE];
    }
    delete_mem();
}

TEST(decode, invalidInstructions) {
    init_mem(DEFAULT_MEM_SIZE);
    size_t fret = read_file("../tests/invalid_instr.bin");
    EXPECT_EQ(fret, 0);
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);
    
    for (size_t i = 0; i < 23; ++i) {
        fetch();
        bool ret = decode();
        EXPECT_EQ(ret, false) << "instr: " << cntrl_regs[OPERATION] << "; iter: " << i;
    }
    delete_mem();
}

TEST(decode, dataRegs) {
    init_mem(DEFAULT_MEM_SIZE);

    prog_mem[0] = 6;
    prog_mem[5] = 4;
    prog_mem[6] = 18;
    prog_mem[7] = 1;
    prog_mem[8] = 2;
    prog_mem[9] = 3;
    prog_mem[10] = 10;
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);
    reg_file[R2] = 10;
    reg_file[R3] = 5;
    fetch();

    bool ret = decode();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 10);
    EXPECT_EQ(data_regs[REG_VAL_2], 5);

    delete_mem();
}

// testing execute instruction
TEST(execute, CorrectExecution) {
    init_mem(DEFAULT_MEM_SIZE);
    size_t fret = read_file("../tests/correct_instr.bin");
    EXPECT_EQ(fret, 0);
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);

    for (size_t i = 0; i < 21; ++i) {
        prog_mem[500] = 15;
        prog_mem[501] = 0;
        reg_file[R1] = 1;
        reg_file[R2] = 4;
        reg_file[R3] = 2;
        fetch();
        decode();
        bool ret = execute();
        EXPECT_EQ(ret, true);

        switch(cntrl_regs[OPERATION]) {
            case(JMP):
                EXPECT_EQ(reg_file[PC], 20);
                break;
            case(MOV):
                EXPECT_EQ(reg_file[R1], 4);
                break;
            case(MOVI):
                EXPECT_EQ(reg_file[R1], 10);
                break;
            case(LDA):
                EXPECT_EQ(reg_file[R1], 500) << "imm: " << cntrl_regs[IMMEDIATE];
                break;
            case(STR):
                EXPECT_EQ((unsigned int)prog_mem[501], 1);
                break;
            case(LDR):
                EXPECT_EQ(reg_file[R1], 15);
                break;
            case(STB):
                EXPECT_EQ(prog_mem[501], (unsigned char)1);
                break;
            case(LDB):
                EXPECT_EQ(reg_file[R1], 15);
                break;
            case(ADD):
                EXPECT_EQ(reg_file[R1], 6);
                break;
            case(ADDI):
                EXPECT_EQ(reg_file[R1], 14);
                break;
            case(SUB):
                EXPECT_EQ(reg_file[R1], 2);
                break;
            case(SUBI):
                EXPECT_EQ(reg_file[R1], 2);
                break;
            case(MUL):
                EXPECT_EQ(reg_file[R1], 8);
                break;
            case(MULI):
                EXPECT_EQ(reg_file[R1], 40);
                break;
            case(DIV):
                EXPECT_EQ(reg_file[R1], 2);
                break;
            case(SDIV):
                EXPECT_EQ(reg_file[R1], 2);
                break;
            case(DIVI):
                EXPECT_EQ(reg_file[R1], 4);
                break;
            case(TRP):
                switch(cntrl_regs[IMMEDIATE]) {
                    case(HALT):
                        EXPECT_EQ(running, false);
                    case(WINT):
                    case(RINT):
                    case(WCHAR):
                    case(RCHAR):
                    case(PREGS):
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    } 
    delete_mem();
}

TEST(trp, writeInt) {
    init_mem(DEFAULT_MEM_SIZE);
    testing::internal::CaptureStdout();
    data_regs[REG_VAL_1] = 42;
    data_regs[REG_VAL_2] = 0;
    cntrl_regs[OPERATION] = TRP;
    cntrl_regs[IMMEDIATE] = 1;
    bool ret = execute();
    EXPECT_EQ(ret, true);
    string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "42");

    testing::internal::CaptureStdout();
    data_regs[REG_VAL_1] = 1024;
    ret = execute();
    EXPECT_EQ(ret, true);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "1024");
        
    // test decode
    cntrl_regs[0] = TRP;
    cntrl_regs[IMMEDIATE] = 1;
    reg_file[R3] = 4138;
    ret = decode();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 4138);

    delete_mem();
}

TEST(trp, writeChar) {
    init_mem(DEFAULT_MEM_SIZE);
    testing::internal::CaptureStdout();
    data_regs[REG_VAL_1] = '*';
    data_regs[REG_VAL_2] = 0;
    cntrl_regs[OPERATION] = TRP;
    cntrl_regs[IMMEDIATE] = 3;
    bool ret = execute();
    EXPECT_EQ(ret, true);
    string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "*");

    // test decode
    cntrl_regs[0] = TRP;
    cntrl_regs[IMMEDIATE] = 3;
    reg_file[R3] = 298;
    ret = decode();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 42);


    delete_mem();
}

TEST(execute, LDA) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = LDA;
    cntrl_regs[OPERAND_1] = R15;
    cntrl_regs[IMMEDIATE] = 16777215;
    bool ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[R15], 16777215);

    cntrl_regs[OPERATION] = LDA;
    cntrl_regs[OPERAND_1] = R15;
    cntrl_regs[IMMEDIATE] = -1;
    ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[R15], -1);

    cntrl_regs[OPERATION] = LDA;
    cntrl_regs[OPERAND_1] = SB;
    cntrl_regs[IMMEDIATE] = -1;
    ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[SB], -1);

    delete_mem();
}

TEST(decode, memAccessInstr) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = STR;
    cntrl_regs[OPERAND_1] = HP;
    cntrl_regs[IMMEDIATE] = 20;
    reg_file[HP] = 1024;
    bool ret = decode();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 1024);

    // test LDR
    cntrl_regs[OPERATION] = LDR;
    cntrl_regs[OPERAND_1] = HP;
    cntrl_regs[IMMEDIATE] = 20;
    reg_file[HP] = 1024;
    ret = decode();
    EXPECT_EQ(ret, true);

    // test STB
    cntrl_regs[OPERATION] = STB;
    cntrl_regs[OPERAND_1] = HP;
    cntrl_regs[IMMEDIATE] = 20;
    reg_file[HP] = 1024;
    ret = decode();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 0);

    // test LDB
    cntrl_regs[OPERATION] = LDB;
    cntrl_regs[OPERAND_1] = HP;
    cntrl_regs[IMMEDIATE] = 20;
    reg_file[HP] = 1024;
    ret = decode();
    EXPECT_EQ(ret, true);

    delete_mem();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
