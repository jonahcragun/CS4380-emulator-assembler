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
