from os.path import exists
import re
from enum import Enum

# assembler opens a valid input file and converts assembly code to byte code writtento .bin file
class Assembler:
    def __init__(self):
        self.mem = [] # list of prog memory
        self.labels = {} # dict of labels associated with line numbers
        self.file = "" # string of assembly file
        self.bin_file_name = "" # string of file name (with .bin extension)
        self.cur_line = 0 # current line number
        self.MAX_INT = 2147483647 # max int allowed stored in memory
        self.MIN_INT = -2147483648

    # read contents of file int self.fileo
    # saves file name (with .bin extension to) self.bin_file_name
    # return 1 if file is not valid
    # return 0 if successful
    def read_file(self, file_name: str) -> int:
        fns = file_name.split('.')
        
        # check file exists and is .asm
        if (not exists(file_name) or fns[-1] != "asm"):
            return 1

        # save file name with .bin extension
        self.bin_file_name = ''.join(fns[:-1]) + '.bin'

        # read file to string
        with open(file_name, 'r') as f:
            self.file = f.read()

        return 0

    # write contents of self.mem to <self.file_name>.bin
    def write_bin(self) -> int:
        with open(self.bin_file_name, 'wb') as f:
            f.write(bytearray(self.mem))

    # convert data section to bytes, store in self.mem
    # return 2 if invalid line is encountered (or eof)
    # return 0 if instruction is encountered
    def parse_data(self) -> int:
        # func pops next char from self.file
        def popc():
            c = self.file[0]
            self.file = self.file[1:]
            return c
       
        # all valid states and which is currently active
        states = {
            "start_line": True,
            "start_space": False,
            "comment": False,
            "label": False,
            "label_done": False,
            "directive": False,
            "byt0": False,
            "byt1": False,
            "byt2": False,
            "int0": False,
            "int1": False,
            "int2": False,
            "alloc_int": False,
            "alloc_byt_num": False,
            "get_byt_char": False,
            "alloc_byt_char": False,
            "end_line": False,
            "error": False,
            "exit": False
        }


        # read through input file a character at a time until error, eof, or instruction is encountered
        label = ""
        value = ""
        while True:
            s = list(states.keys())[list(states.values()).index(True)]
            if len(self.file) == 0:
                s = "error"
            else:
                c = popc()
            
            match s:
                # at beginning of line
                case "start_line":
                    self.cur_line += 1
                    states[s] = False
                    if c == '\n':
                        states["start_line"] = True
                    elif re.match(r'[ \t]', c):
                        states["start_space"] = True
                    elif re.match(r'[a-zA-z\d]', c):
                        states["label"] = True
                        label += c;
                    elif c == ';':
                        states["comment"] = True
                    else:
                        states["error"] = True
                # possible label found, save label name and memory index in self.labels
                case "label":
                    states[s] = False
                    if re.match(r'[\w\$]', c):
                        states["label"] = True
                        label += c
                    elif re.match(r'[ \t]', c):
                        states["label_done"] = True
                        self.labels[label] = len(self.mem)
                        label = ""
                    else:
                        states["error"] = True
                # line starts with space
                case "start_space":
                    states[s] = False
                    if re.match(r'[ \t]', c):
                        states["start_space"] = True
                    elif c == '.':
                        states["directive"] = True
                    elif c == '\n':
                        states["start_line"] = True
                    elif c == ';':
                        states["comment"] = True
                    elif re.match(r'[a-zA-Z\d]', c):
                        states["exit"] = True
                        self.file = c + self.file
                    else:
                        states["error"] = True
                # comment found
                case "comment":
                    states[s] = False
                    if c == '\n':
                        states["start_line"] = True
                    else:
                        states["comment"] = True
                # label has been read and saved to self.labels
                case "label_done":
                    states[s] = False
                    if re.match(r'[ \t]', c):
                        states[s] = True
                    elif re.match(r'[a-zA-Z\d]', c):
                        states["exit"] = True
                        self.file = c + self.file
                    elif c == '.':
                        states["directive"] = True
                    else:
                        states["error"] = True
                # directive found
                case "directive":
                    states[s] = False
                    if c.lower() == 'i':
                        states["int0"] = True
                    elif c.lower() == 'b':
                        states["byt0"] = True
                    else:
                        states["error"] = True
                # reading chars for byt directive
                case "byt0":
                    states[s] = False
                    if c.lower() == 'y':
                        states["byt1"] = True
                    else:
                        states["error"] == True
                # reading chars for byt directive
                case "byt1":
                    states[s] = False
                    if c.lower() == 't':
                        states["byt2"] = True
                    else:
                        states["error"] = True
                # reading chars for byt directive
                case "byt2":
                    states[s] = False
                    if re.match(r'[ \t]', c):
                        states["byt2"] = True
                    elif c == '#':
                        states["alloc_byt_num"] = True
                    elif c == "'":
                        states["get_byt_char"] = True
                    else:
                        states["error"] = True
                # reading chars for int directive
                case "int0":
                    states[s] = False
                    if c.lower() == 'n':
                        states["int1"] = True
                    else:
                        states["error"] = True
                # reading chars for int directive
                case "int1":
                    states[s] = False
                    if c.lower() == 't':
                        states["int2"] = True
                    else:
                        states["error"] = True
                # reading chars for int directive
                case "int2":
                    states[s] = False
                    if re.match(r'[ \t]', c):
                        states["int2"] = True
                    elif c == '#':
                        states["alloc_int"] = True
                    else:
                        states["error"] = True
                # read int value from self.file, store in self.mem as 4 bytes
                case "alloc_int":
                    states[s] = False
                    if re.match(r'[\d-]', c):
                        states[s] = True
                        value += c
                    elif re.match(r'[\s;]', c):
                        # add 4 byte value to self.mem
                        if int(value) > self.MAX_INT or int(value) < self.MIN_INT:
                            states["error"] = True
                            continue
                        else:
                            b1, b2, b3, b4 = (int(value) & 0xFFFFFFFF).to_bytes(4, 'little')
                            self.mem.extend([b1, b2, b3, b4])
                            value = ""

                        if re.match(r'[ \t]', c):
                            states["end_line"] = True
                        elif c == ';':
                            states["comment"] = True
                        else:
                            states["start_line"] = True
                    else:
                        states["error"] = True
                # read char value as numeric value from self.file, store as 1 byte number in self.mem
                case "alloc_byt_num":
                    states[s] = False
                    if re.match(r'[\d]', c):
                        states[s] = True
                        value += c
                    elif re.match(r'[\s;]', c):
                        # add 1 byte value to self.mem
                        if int(value) > 255: # max val in 1 byte
                            states["error"] = True
                            continue
                        else:
                            self.mem.append(int(value))
                            value = ""

                        if re.match(r'[ \t]', c):
                            states["end_line"] = True
                        elif c == ';':
                            states["comment"] = True
                        else:
                            states["start_line"] = True
                    else:
                        states["error"] = True     
                # get char from inside quotes
                case "get_byt_char":
                    states[s] = False
                    value = c
                    states["alloc_byt_char"] = True
                # read char value as char from self.file, convert to int, store as 1 byte in self.mem
                case "alloc_byt_char":
                    states[s] = False
                    if c == "'":
                        states["end_line"] = True
                        self.mem.append(ord(value))
                        value = ""
                    else:
                        states["error"] = True
                # end of line, eat ws, handle comments, move to start line after '\n'
                case "end_line":
                    states[s] = False
                    if c == '\n':
                        states["start_line"] = True
                    elif c == ';':
                        states["comment"] = True
                    elif re.match(r'[ \t]', c):
                        states[s] = True
                    else:
                        states["error"] = True
                # handle any syntax errors encountered, return with value of 2
                case "error":
                    return 2
                # possible instruction encountered, return with val of 0
                case "exit":
                    self.file = c + self.file
                    return 0
                # invalid state found (should not ever happen)
                case _:
                    raise ValueError(f"invalid state found {s}")

    
    # convert instructions to bytes, store in self.mem
    def parse_instr(self) -> int:
        # func pops next char from self.file
        def popc():
            c = self.file[0]
            self.file = self.file[1:]
            return c

        # enum type for states
        class State(Enum):
            START_LINE = 0
            START_SPACE = 1
            COMMENT = 2
            LABEL = 3
            LABEL_DONE = 4
            OPERATOR = 5
            OPERATOR_DONE = 6
            OPERANDS = 7
            PROC_INSTR = 8
            ERROR = 9
            EXIT = 10

        # enum for instrunctions
        class Instr(Enum):
            JMP = 1
        
        # loop to read chars from self.file
        operator = ""
        operand = ""
        operands = []
        label = ""
        s = State.START_LINE
        while True:
            if s != State.EXIT:
                if len(self.file) > 0:
                    c = popc()
                else:
                    s = State.ERROR

        match s:
            case State.START_LINE:
                self.cur_line += 1
                if re.match(r'[ \t]', c):
                    s = State.START_SPACE
                elif c == '\n':
                    s = State.START_LINE
                elif c == ';':
                    s = State.COMMENT
                elif re.match(r'[a-zA-Z\d]', c):
                    s = State.LABEL
                    label += c
                else:
                    s = State.ERROR
            case State.START_SPACE:
                if re.match(r'[ \t]', c):
                    s = State.START_SPACE
                elif c == '\n':
                    s = State.START_LINE
                elif c == ';':
                    s = State.COMMENT
                elif re.match(r'[a-zA-Z]', c):
                    s = State.OPERATOR
                    operator += c.lower()
                else:
                    s = State.ERROR
            case State.COMMENT:
                if c == '/n':
                    s = State.START_LINE
                else:
                    s = State.COMMENT
            case State.LABEL:
                if re.match(r'[\w\$]', c):
                    s = State.LABEL
                    label += c
                elif re.mach(r'[ \t]', c):
                    s = State.LABEL_DONE
                    self.labels[label] = len(self.mem)
                    label = ""
                else:
                    s = State.ERROR
            case State.LABEL_DONE:
                if re.match(r'[ \n]', c):
                    s = State.LABEL_DONE
                elif re.match(r'[a-zA-Z]', c):
                    s = State.OPERATOR
                    operator += c.lower()
                else:
                    s = State.ERROR
            case State.OPERATOR:
                if re.match(r'[a-zA-Z]', c):
                    s = State.OPERATOR
                    operator += c.lower()
                elif re.match(r'[ \t]', c):
                    s = State.OPERATOR_DONE
                else:
                    s = State.ERROR
            case State.OPERATOR_DONE:
                if re.match(r'[a-zA-Z]', c):
                    s = State.OPERATOR_DONE
                elif re.match(r'[a-zA-Z]', c):
                    s = State.OPERANDS
                    operands += c.lower()
                else:
                    s = State.ERROR
            case State.OPERANDS:
                pass
            case State.PROC_INSTR:
                pass
            case State.ERROR:
                return 2
            case State.EXIT:
                return 0


    # replace labels in self.mem with addr from self.labels
    def second_pass(self) -> int:
        pass

    # print error message and exit program with proper return value
    def handle_error(self, ret: int) -> None:
        pass

    # run all parts of assembler
    def run(self) -> None:
        self.file = " \ts9A$_(\n\r"
        self.parse_data()
