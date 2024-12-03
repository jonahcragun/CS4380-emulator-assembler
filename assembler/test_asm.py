from assembler import Assembler
import pytest

# test file is read correctly
def test_read_file():
    asm = Assembler()
    ret = asm.read_file("test/test.txt")
    assert ret == 1

    ret = asm.read_file("tests/test.asm")
    assert ret == 0
    assert asm.file ==  "num .int #30\n\nmain trp #0\n"
    assert asm.bin_file_name == "tests/test.bin"

    ret = asm.read_file("fake_test.asm")
    assert ret == 1

# test file writes binary correctly
def test_write_bin():
    asm = Assembler()
    asm.bin_file_name = "tests/test.bin"
    asm.mem = [30, 0, 0, 0, 254, 255, 255, 255, 104, 105, 10]
    ret = asm.write_bin()

    with open("tests/test.bin", 'rb') as f:
        b = f.read()

    assert list(b) == [30, 0, 0, 0, 254, 255, 255, 255, 104, 105, 10]


    
# test parse data section 
def test_parse_data_valid():
    asm = Assembler()
    # test file works when passed .int, .byt, different whitespace, escaped chars, with labeled instruction
    asm.file = "num$ .int #30\n2_Num .INt #-2  ;end of line 5%6^ \tj\nchar .byt 'h'\n\t.byt #105\n   .bYT '\n'\n\n;this is the instr section\nmain trap #0"
    ret = asm.parse_data()
    assert asm.mem == [15, 0, 0, 0, 30, 0, 0, 0, 254, 255, 255, 255, 104, 105, 10]
    assert ret == 0
    assert asm.labels == {"num$": 4, "2_Num": 8, "char": 12}
    assert asm.cur_line == 8

    # test file works when passed .int, .byt, different whitespace, escaped chars, with unlabeled instruction
    asm = Assembler()
    asm.file = "num$ .int #30\n2_Num .INt #-2\nchar .byt 'h'\n\t.byt #105\n   .bYT '\n'\n   trap #0"
    ret = asm.parse_data()
    assert ret == 0
    assert asm.mem == [15, 0, 0, 0, 30, 0, 0, 0, 254, 255, 255, 255, 104, 105, 10]
    assert asm.labels == {"num$": 4, "2_Num": 8, "char": 12}
    assert asm.cur_line == 6


def test_parse_data_invalid():
    pass


# test parse instruction section
def test_parse_instr_valid():
    asm = Assembler()
    asm.file = " adD r1, R2, r3 ; add\n jmp Sec_2$ ; jump  \t\nSec_2$ divi r1, r2, #256 \n trp #3\n mov r13, r2 \n movi r14, #20\n stR r12, val"
    asm.mem = [12]
    asm.labels = {'val': 0, 'main': 1}

    ret = asm.parse_instr()

    assert ret == 0
    assert asm.mem == [12, 18, 1, 2, 3, 0, 0, 0, 0, 1, 0, 0, 0, "Sec_2$", 0, 0, 0, 26, 1, 2, 0, 0, 1, 0, 0, 31, 0, 0, 0, 3, 0, 0, 0, 7, 13, 2, 0, 0, 0, 0, 0, 8, 14, 0, 0, 20, 0, 0, 0, 10, 12, 0, 0, "val", 0, 0, 0]
    assert asm.labels == {'val': 0, 'main': 1, 'Sec_2$': 17}
    

# test second pass
def test_second_pass():
    asm = Assembler()
    asm.labels = {'val': 0, 'main': 1}
    asm.mem = [12, 1, 0, 0, 0, 'main', 0, 0, 0, 9, 0, 0, 0, 'val', 0, 0, 0]

    ret = asm.second_pass()
    assert asm.mem == [12, 1, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0]
    assert ret == 0

# **************************************************
# Tests added for resubmission
# **************************************************

def test_parse_more_instr():
    asm = Assembler()
    asm.file = " jmp MAIN\nMAIN movi r0, #1    \n add sP, r3, r4\n addi Hp, SP, #5\n muli R6, R7, #-2\nHELLO div r6, r7, r8"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 0

    asm.file = " jmp MAIN\nMAIN muli r6, r7, #10101000000000000000001010123232001"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 0

    asm.file = " jmp MAIN\nMAIN muli r6, r7, #-10000\n      add hp, sp, sL\n movi R15, #-1\n    divi hp, sp, #-2" 
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    print(asm.mem)
    assert ret == 0
    assert asm.mem == [4, 0, 0, 0, 1, 0, 0, 0, 'MAIN', 0, 0, 0, 23, 6, 7, 0, 240, 216, 255, 255, 18, 21, 19, 17, 0, 0, 0, 0, 8, 15, 0, 0, 255, 255, 255, 255, 26, 21, 19, 0, 254, 255, 255, 255]

def test_invalid_instr():
    asm = Assembler()
    asm.file = " jmp MAIN\nMAIN mov r0, r1 \n LTR r0, LEFT"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2


    asm.file = " jmp MAIN\nMAIN mov r0, r1 \n LDR r0, LEFT \n"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert asm.mem == [4, 0, 0, 0, 1, 0, 0, 0, 'MAIN', 0, 0, 0, 7, 0, 1, 0, 0, 0, 0, 0, 11, 0, 0, 0, 'LEFT', 0, 0, 0]
    assert ret == 0


    asm.file = " jmp MAIN\nMAIN mov r0, r1 \n LDR r0, LEFT\n mulli r0, r15, #12 \n"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test mov invalid param 1
    asm.file = " mov HS, r1"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test mov invalid param 2
    asm.file = " mov r1, HS"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test movi invalid param 1
    asm.file = " movi HS, #1"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test str invalid param 1
    asm.file = " str HS, Label"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test mul invalid param 1
    asm.file = " mul HS, r1, HP"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test mul invalid param 2
    asm.file = " mul r1, HS, r2"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test mul invalid param 3
    asm.file = " mul r1, r2, HS"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test muli invalid param 1
    asm.file = " muli HS, r0, r1"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test muli invalid param 2
    asm.file = " muli r0, r1, HS"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

    # test muli invalid param 3
    asm.file = " muli r1, r2, HS"
    asm.mem = [4, 0, 0, 0]
    ret = asm.parse_instr()
    assert ret == 2

