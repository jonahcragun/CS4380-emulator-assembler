from assembler import *
import sys

def main():
    asm = Assembler()
    if len(sys.argv) != 2:
        asm.handle_error(1)

    asm.run(sys.argv[1])

if __name__ == "__main__":
    main()
