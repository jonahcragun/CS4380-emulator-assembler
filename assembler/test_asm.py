from assembler import Assembler
import pytest

# test file is read correctly
def test_read_file():
    asm = Assembler()
    ret = asm.read_file("test/test.txt")
    assert ret == 1

    ret = asm.read_file("tests/test.asm")
    assert ret == 0
    assert asm.file ==  "num .int #30\n\nmain trap #0\n"
    assert asm.bin_file_name == "tests/test.bin"

    ret = asm.read_file("fake_test.asm")
    assert ret == 1
    
# test parse data section 
def test_parse_data_valid():
    # test file works when passed .int, .byt, different whitespace, escaped chars, with labeled instruction
    file = "num$ .int #30\n2_num .INt #-2  ;end of line 5%6^ \tj\nchar .byt 'h'\n\t.byt #151\n   .bYT '\n'\n\n;this is the instr section\nmain trap #0"
    asm = Assembler()
    ret = asm.parse_data()
    assert ret == 0
    assert asm.mem == [30, 0, 0, 0, -2, 0, 0, 0, 150, 151, 10]

    # test file works when passed .int, .byt, different whitespace, escaped chars, with unlabeled instruction
    file = "num .int #30\n2num .INt #-2\nchar .byt 'h'\n\t.byt #151\n   .bYT '\n'\n   trap #0"
    ret = asm.parse_data()
    assert ret == 0
    assert mem == [30, 0, 0, 0, -2, 0, 0, 0, 150, 151, 10]


def test_parse_data_invalid():
    pass
