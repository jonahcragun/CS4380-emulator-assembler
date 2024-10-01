from assembler import *

def main():
    asm = Assembler()
    # test file works when passed .int, .byt, different whitespace, escaped chars, with labeled instruction
    asm.file = "num$ .int #30\n2_num .INt #-2  ;end of line 5%6^ \tj\nchar .byt 'h'\n\t.byt #151\n   .bYT '\n'\n\n;this is the instr section\nmain trap #0"
    ret = asm.parse_data()
 
    print(asm.mem)
    print(ret)

if __name__ == "__main__":
    main()
