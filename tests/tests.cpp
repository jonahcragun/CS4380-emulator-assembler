#include <gtest/gtest.h>
#include <cstdio>
#include "../include/emu4380.h"
#include "../include/cache.h"

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
    init_cache(0);
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

TEST(execute, illegalSTR) {
    init_mem(DEFAULT_MEM_SIZE);
    EXPECT_EQ(0,0);
}

// Test JMR
TEST(JMR, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = JMR;
    cntrl_regs[OPERAND_1] = R1;
    reg_file[PC] = 0;
    reg_file[R1] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    ret = decode();
    EXPECT_EQ(ret, false);
    
    
    delete_mem();
}

TEST(JMR, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);
    cntrl_regs[OPERATION] = JMR;
    data_regs[REG_VAL_1] = 5;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 5);

    delete_mem();
}


// Test BNZ
TEST(BNZ, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = BNZ;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 10;
    reg_file[PC] = 0;
    reg_file[R1] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 10);

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    ret = decode();
    EXPECT_EQ(ret, false);
    
    delete_mem();

}

TEST(BNZ, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[PC] = 0;
    cntrl_regs[OPERATION] = BNZ;
    data_regs[REG_VAL_1] = 5;
    cntrl_regs[IMMEDIATE] = 10;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 10);

    cntrl_regs[OPERATION] = BNZ;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = 0;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);


    cntrl_regs[OPERATION] = BNZ;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = -1;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 10);

    delete_mem();

}

// Test BGT
TEST(BGT, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = BGT;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 10;
    reg_file[PC] = 0;
    reg_file[R1] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 10);

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    ret = decode();
    EXPECT_EQ(ret, false);
    
    
    delete_mem();

}

TEST(BGT, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[PC] = 0;
    cntrl_regs[OPERATION] = BGT;
    data_regs[REG_VAL_1] = 5;
    cntrl_regs[IMMEDIATE] = 10;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 10);

    cntrl_regs[OPERATION] = BGT;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = 0;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);


    cntrl_regs[OPERATION] = BGT;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = -1;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);

    delete_mem();

}

// Test BLT
TEST(BLT, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = BLT;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 10;
    reg_file[PC] = 0;
    reg_file[R1] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 10);

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    ret = decode();
    EXPECT_EQ(ret, false);
    
    
    delete_mem();

}

TEST(BLT, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[PC] = 0;
    cntrl_regs[OPERATION] = BLT;
    data_regs[REG_VAL_1] = 5;
    cntrl_regs[IMMEDIATE] = 10;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);

    cntrl_regs[OPERATION] = BLT;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = 0;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);


    cntrl_regs[OPERATION] = BLT;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = -1;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 10);

    delete_mem();

}


// Test BRZ
TEST(BRZ, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = BRZ;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 10;
    reg_file[PC] = 0;
    reg_file[R1] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 10);

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    ret = decode();
    EXPECT_EQ(ret, false);
    
    
    delete_mem();

}

TEST(BRZ, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[PC] = 0;
    cntrl_regs[OPERATION] = BRZ;
    data_regs[REG_VAL_1] = 5;
    cntrl_regs[IMMEDIATE] = 10;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);

    cntrl_regs[OPERATION] = BRZ;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = 0;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 10);


    cntrl_regs[OPERATION] = BRZ;
    reg_file[PC] = 0;
    data_regs[REG_VAL_1] = -1;
    cntrl_regs[IMMEDIATE] = 10;
    ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[PC], 0);

    delete_mem();

}


// Test ISTR
TEST(ISTR, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[R1] = 5;
    reg_file[R2] = 10;
    cntrl_regs[OPERATION] = ISTR;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(data_regs[REG_VAL_2], 10);
    
    delete_mem();

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    cntrl_regs[OPERAND_2] = 22;
    ret = decode();

    EXPECT_EQ(ret, false);

}

TEST(ISTR, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = ISTR;
    data_regs[REG_VAL_1] = 500;
    data_regs[REG_VAL_2] = 10;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(*reinterpret_cast<unsigned int*>(prog_mem + 10), 500);

    delete_mem();

}

// Test ILDR
TEST(ILDR, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[R1] = 5;
    reg_file[R2] = 500;
    cntrl_regs[OPERATION] = ILDR;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 500);
    
    delete_mem();

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    cntrl_regs[OPERAND_2] = 22;
    ret = decode();

    EXPECT_EQ(ret, false);

}

TEST(ILDR, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = ILDR;
    prog_mem[10] = 244;
    prog_mem[10] = 1;
    data_regs[REG_VAL_1] = 10;
    cntrl_regs[OPERAND_1] = R1;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[R1], 1);

    delete_mem();

}

// Test ISTB
TEST(ISTB, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[R1] = 5;
    reg_file[R2] = 10;
    cntrl_regs[OPERATION] = ISTB;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(data_regs[REG_VAL_2], 10);
    
    delete_mem();

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    cntrl_regs[OPERAND_2] = 22;
    ret = decode();

    EXPECT_EQ(ret, false);

}

TEST(ISTB, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = ISTB;
    data_regs[REG_VAL_1] = 5;
    data_regs[REG_VAL_2] = 10;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(prog_mem[10], 5);

    delete_mem();

}

// Test ILDB
TEST(ILDB, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    reg_file[R1] = 5;
    reg_file[R2] = 10;
    cntrl_regs[OPERATION] = ILDB;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 10);
    
    delete_mem();

    // expect fail
    cntrl_regs[OPERAND_1] = 22;
    cntrl_regs[OPERAND_2] = 22;
    ret = decode();

    EXPECT_EQ(ret, false);

}

TEST(ILDB, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = ILDB;
    prog_mem[10] = 5;
    data_regs[REG_VAL_1] = 10;
    cntrl_regs[OPERAND_1] = R1;
    bool ret = execute();

    EXPECT_EQ(ret, true);
    EXPECT_EQ(reg_file[R1], 5);

    delete_mem();

}


// Test CMP
TEST(CMP, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = CMP;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;
    reg_file[R1] = 5;
    reg_file[R2] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    EXPECT_EQ(data_regs[REG_VAL_2], 5);
    
    delete_mem();

}

TEST(CMP, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = CMP;
    cntrl_regs[OPERAND_1] = R1;
    data_regs[REG_VAL_1] = 5;
    data_regs[REG_VAL_2] = 5;
    bool ret = execute();

    // test eq
    EXPECT_EQ(ret, true);
    EXPECT_EQ(static_cast<int>(reg_file[R1]), 0);

    // test gt 
    data_regs[REG_VAL_2] = -1;
    ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(static_cast<int>(reg_file[R1]), 1);

    // test lt
    data_regs[REG_VAL_2] = 9;
    ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(static_cast<int>(reg_file[R1]), -1);

    delete_mem();

}

// Test CMPI
TEST(CMPI, DECODE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = CMP;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;
    reg_file[R1] = 5;
    reg_file[R2] = 5;
    bool ret = decode(); 

    EXPECT_EQ(ret, true);
    EXPECT_EQ(data_regs[REG_VAL_1], 5);
    
    delete_mem();

}

TEST(CMPI, EXECUTE) {
    init_mem(DEFAULT_MEM_SIZE);
    cntrl_regs[OPERATION] = CMPI;
    cntrl_regs[OPERAND_1] = R1;
    data_regs[REG_VAL_1] = 5;
    cntrl_regs[IMMEDIATE] = 5;
    bool ret = execute();

    // test eq
    EXPECT_EQ(ret, true);
    EXPECT_EQ(static_cast<int>(reg_file[R1]), 0);

    // test gt 
    cntrl_regs[IMMEDIATE] = -1;
    ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(static_cast<int>(reg_file[R1]), 1);

    // test lt
    cntrl_regs[IMMEDIATE] = 9;
    ret = execute();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(static_cast<int>(reg_file[R1]), -1);

    delete_mem();

}

TEST(_CACHE, GET_CACHE_BYTE) {
    // test on fully associative
    for (int i = 0; i < CACHE_SIZE; ++i) 
        cache[i].valid = false;

    init_mem(DEFAULT_MEM_SIZE);
    mem_cycle_cntr = 0;
    prog_mem[100] = 1;
    prog_mem[101] = 2;
    prog_mem[102] = 3;
    prog_mem[103] = 4;
    prog_mem[111] = 5;
    prog_mem[1021] = 7;
    prog_mem[1120] = 6;

    // init direct mapped cache
    init_cache(1);

    cache_byte cb = get_cache_byte(100);
    EXPECT_EQ(cb.byte, 1);
    EXPECT_EQ(cb.penalty, 15);

    cb = get_cache_byte(101);
    EXPECT_EQ(cb.byte, 2);
    EXPECT_EQ(cb.penalty, 1);

    cb = get_cache_byte(111);
    EXPECT_EQ(cb.byte, 5);
    EXPECT_EQ(cb.penalty, 1);

    cb = get_cache_byte(1021);
    EXPECT_EQ(cb.byte, 7);
    EXPECT_EQ(cb.penalty, 15);

    // change value in cache line before overwitting it
    cache[6*cache_set_size].block[4] = 10;
    cache[6*cache_set_size].block[7] = 200;

    cb = get_cache_byte(1120);
    EXPECT_EQ(cb.byte, 6);
    EXPECT_EQ(cb.penalty, 29);

    // check value was written to memory correctly
    EXPECT_EQ(prog_mem[100], 10);
    EXPECT_EQ(prog_mem[101], 2);
    EXPECT_EQ(prog_mem[102], 3);
    EXPECT_EQ(prog_mem[103], 200);
    EXPECT_EQ(prog_mem[111], 5);

    // ******************************

    // test on fully associative
    for (int i = 0; i < CACHE_SIZE; ++i) 
        cache[i].valid = false;
    init_cache(3);

    mem_cycle_cntr = 0;
    prog_mem[100] = 1;
    prog_mem[101] = 2;
    prog_mem[102] = 3;
    prog_mem[103] = 4;
    prog_mem[111] = 5;
    prog_mem[1021] = 7;
    prog_mem[1120] = 6;


    cb = get_cache_byte(100);
    EXPECT_EQ(cb.byte, 1);
    EXPECT_EQ(cb.penalty, 15);

    cb = get_cache_byte(101);
    EXPECT_EQ(cb.byte, 2);
    EXPECT_EQ(cb.penalty, 1);

    cb = get_cache_byte(111);
    EXPECT_EQ(cb.byte, 5);
    EXPECT_EQ(cb.penalty, 1);

    cb = get_cache_byte(1021);
    EXPECT_EQ(cb.byte, 7);
    EXPECT_EQ(cb.penalty, 15);

    // change value in cache line before overwitting it
    cache[6*cache_set_size].block[4] = 10;
    cache[6*cache_set_size].block[7] = 200;

    cb = get_cache_byte(1120);
    EXPECT_EQ(cb.byte, 6);
    EXPECT_EQ(cb.penalty, 15);

    prog_mem[2144] = 15;
    cb = get_cache_byte(2144);
    EXPECT_EQ(cb.byte, 15);
    EXPECT_EQ(cb.penalty, 29);


    // check value was written to memory correctly
    EXPECT_EQ(prog_mem[100], 10);
    EXPECT_EQ(prog_mem[101], 2);
    EXPECT_EQ(prog_mem[102], 3);
    EXPECT_EQ(prog_mem[103], 200);
    EXPECT_EQ(prog_mem[111], 5);



    delete_mem();
}

TEST(_CACHE, GET_CACHE_WORDS) {
    // test on fully associative
    for (int i = 0; i < CACHE_SIZE; ++i) 
        cache[i].valid = false;

    init_mem(DEFAULT_MEM_SIZE);
    init_cache(1);

    mem_cycle_cntr = 0;
    prog_mem[100] = 1;
    prog_mem[104] = 2;
    prog_mem[105] = 3;
    prog_mem[106] = 4;
    prog_mem[111] = 5;
    prog_mem[112] = 1;
    prog_mem[1021] = 1;
    prog_mem[1133] = 6;
    prog_mem[1137] = 7;

    cache_word cw = get_cache_words(100);
    EXPECT_EQ(cw.words.at(0), 1);
    EXPECT_EQ(cw.penalty, 15);
    
    cw = get_cache_words(111);
    EXPECT_EQ(cw.words.at(0), 261);
    EXPECT_EQ(cw.penalty, 15);

    cw = get_cache_words(111);
    EXPECT_EQ(cw.words.at(0), 261);
    EXPECT_EQ(cw.penalty, 1);

    cw = get_cache_words(1133);
    EXPECT_EQ(cw.words.at(0), 6);
    EXPECT_EQ(cw.penalty, 45);

    cw = get_cache_words(1133, 2);
    EXPECT_EQ(cw.words.at(0), 6);
    EXPECT_EQ(cw.words.at(1), 7);
    EXPECT_EQ(cw.penalty, 1);

    cw = get_cache_words(111);
    EXPECT_EQ(cw.words.at(0), 261);
    EXPECT_EQ(cw.penalty, 45);

    
    delete_mem();
}

TEST(_CACHE, WRITE_CACHE_BYTE) {
    // test on fully associative
    for (int i = 0; i < CACHE_SIZE; ++i) 
        cache[i].valid = false;

    init_mem(DEFAULT_MEM_SIZE);
    init_cache(1);

    cache_byte cb = write_cache_byte(100, 1);
    EXPECT_EQ(cb.penalty, 15);
    EXPECT_EQ(cache[6*cache_set_size].block[4], 1);

    cb = write_cache_byte(105, 3);
    EXPECT_EQ(cb.penalty, 1);
    EXPECT_EQ(cache[6*cache_set_size].block[9], 3);

    cb = write_cache_byte(1120, 10);
    EXPECT_EQ(cb.penalty, 29);
    EXPECT_EQ(cache[6*cache_set_size].block[0], 10);

    EXPECT_EQ(prog_mem[100], 1);
    EXPECT_EQ(prog_mem[105], 3);

    delete_mem();
}

TEST(_CACHE, WRITE_CACHE_WORD) {
    // test on fully associative
    for (int i = 0; i < CACHE_SIZE; ++i) 
        cache[i].valid = false;
    
    init_mem(DEFAULT_MEM_SIZE);
    init_cache(1);

    unsigned int pen = write_cache_word(100, 197121);
    EXPECT_EQ(pen, 15);
    EXPECT_EQ(cache[6].block[4], 1);
    EXPECT_EQ(cache[6].block[5], 2);
    EXPECT_EQ(cache[6].block[6], 3);

    pen = write_cache_word(112, 197121);
    EXPECT_EQ(pen, 15);
    EXPECT_EQ(cache[7].block[0], 1);
    EXPECT_EQ(cache[7].block[1], 2);
    EXPECT_EQ(cache[7].block[2], 3);
    EXPECT_EQ(cache[7].block[3], 0);

    pen = write_cache_word(116, 256);
    EXPECT_EQ(pen, 1);
    EXPECT_EQ(cache[7].block[0], 1);
    EXPECT_EQ(cache[7].block[1], 2);
    EXPECT_EQ(cache[7].block[2], 3);
    EXPECT_EQ(cache[7].block[3], 0);
    EXPECT_EQ(cache[7].block[4], 0);
    EXPECT_EQ(cache[7].block[5], 1);
    EXPECT_EQ(cache[7].block[6], 0);

    pen = write_cache_word(1133, 16974337);
    EXPECT_EQ(pen, 45);
    EXPECT_EQ(cache[6].block[12], 0);
    EXPECT_EQ(cache[6].block[13], 1);
    EXPECT_EQ(cache[6].block[14], 2);
    EXPECT_EQ(cache[6].block[15], 3);
    EXPECT_EQ(cache[7].block[0], 1);
    EXPECT_EQ(cache[7].block[1], 0);
    EXPECT_EQ(cache[7].block[2], 0);

    delete_mem();
}

TEST(mem_cycle_cntr, readByte) {
    unsigned char b;
    init_mem(DEFAULT_MEM_SIZE);
    prog_mem[100] = 1;
    prog_mem[101] = 2;
    prog_mem[102] = 3;
    prog_mem[103] = 4;
    prog_mem[104] = 5;
    prog_mem[105] = 6;
    prog_mem[106] = 7;
    prog_mem[107] = 8;
    prog_mem[108] = 9;
    prog_mem[109] = 10;
    prog_mem[110] = 11;
    prog_mem[111] = 12;
    prog_mem[112] = 13;
    prog_mem[113] = 14;
    prog_mem[114] = 15;
    prog_mem[115] = 16;
    prog_mem[116] = 17;
    prog_mem[117] = 18;
    prog_mem[118] = 19;
    prog_mem[1120] = 20;
    prog_mem[1121] = 21;
    prog_mem[1122] = 22;
    prog_mem[1123] = 23;

    init_cache(0);
    mem_cycle_cntr = 0;
    
    b = readByte(100);
    EXPECT_EQ(b, 1);
    EXPECT_EQ(mem_cycle_cntr, 8);
    b = readByte(101);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(mem_cycle_cntr, 16);
    b = readByte(106);
    EXPECT_EQ(b, 7);
    EXPECT_EQ(mem_cycle_cntr, 24);
    b = readByte(115);
    EXPECT_EQ(b, 16);
    EXPECT_EQ(mem_cycle_cntr, 32);
    b = readByte(116);
    EXPECT_EQ(b, 17);
    EXPECT_EQ(mem_cycle_cntr, 40);

    // test direct mapped cache
    init_cache(1);
    mem_cycle_cntr = 0;
    
    b = readByte(100);
    EXPECT_EQ(b, 1);
    EXPECT_EQ(mem_cycle_cntr, 15);
    b = readByte(101);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(mem_cycle_cntr, 16);
    b = readByte(106);
    EXPECT_EQ(b, 7);
    EXPECT_EQ(mem_cycle_cntr, 17);
    b = readByte(1120);
    EXPECT_EQ(b, 20);
    EXPECT_EQ(mem_cycle_cntr, 46);
    b = readByte(1121);
    EXPECT_EQ(b, 21);
    EXPECT_EQ(mem_cycle_cntr, 47);


    // test fully associative cache
    init_cache(2);
    mem_cycle_cntr = 0;
    
    b = readByte(100);
    EXPECT_EQ(b, 1);
    EXPECT_EQ(mem_cycle_cntr, 15);
    b = readByte(101);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(mem_cycle_cntr, 16);
    b = readByte(106);
    EXPECT_EQ(b, 7);
    EXPECT_EQ(mem_cycle_cntr, 17);
    b = readByte(1120);
    EXPECT_EQ(b, 20);
    EXPECT_EQ(mem_cycle_cntr, 32);
    b = readByte(1121);
    EXPECT_EQ(b, 21);
    EXPECT_EQ(mem_cycle_cntr, 33);
    
    // test direct mapped on read word cache
    unsigned int w;
    init_cache(1);
    mem_cycle_cntr = 0;
    
    w = readWord(111);
    EXPECT_EQ(w, 1);
    EXPECT_EQ(mem_cycle_cntr, 23);
    w = readWord(112);
    EXPECT_EQ(w, 2);
    EXPECT_EQ(mem_cycle_cntr, 24);
    w = readWord(117);
    EXPECT_EQ(w, 7);
    EXPECT_EQ(mem_cycle_cntr, 25);
    w = readWord(1133);
    EXPECT_EQ(w, 20);
    EXPECT_EQ(mem_cycle_cntr, 70);
    w = readWord(1134);
    EXPECT_EQ(w, 21);
    EXPECT_EQ(mem_cycle_cntr, 71);


    delete_mem();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


