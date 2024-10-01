from os.path import exists
import re

# assembler opens a valid input file and converts assembly code to byte code writtento .bin file
class Assembler:
    def __init__(self):
        self.mem = [] # list of prog memory
        self.labels = {} # dict of labels associated with line numbers
        self.file = "" # string of assembly file
        self.bin_file_name = "" # string of file name (with .bin extension)
        self.cur_line = 0 # current line number

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
        pass

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
            "end_line": False,
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
            "alloc_byt": False,
            "add_int": False,
            "add_byte_num": False,
            "add_byte_char": False,
            "error": False,
            "exit": False
        }


        # read through input file a character at a time until error, eof, or instruction is encountered
        label = ""
        while True:
            self.cur_line += 1
            s = list(states.keys())[list(states.values()).index(True)]
            if len(file) == 0:
                s = "error"
            else:
                c = popc()
            
            match s:
                case "start_line":
                    states[s] = False
                    if c == '\n':
                        states["start_line"] = True
                    elif re.match(r'[ \t]', c):
                        states["start_ts"] = True
                    elif re.match(r'[a-zA-z\d]', c):
                        states["label"] = True
                        label += c;
                    elif c == ';':
                        states["comment"] = True
                    else:
                        states["error"] = True
                case "label":
                    states[s] = False
                    if re.match(r'[\w\$]', c):
                        states["label"] = True
                        label += c
                    elif re.match(r'[ \t]', c):
                        states["label_done"]
                        self.labels[label] = self.cur_line
                        label = ""
                    else:
                        states["error"] = True
                case "start_space":
                    state[s] = False
                    if re.match(r'[ \t]', c):
                        states["start_space"] = True
                    elif c == '.':
                        states["directive"] = True
                    elif c == '\n':
                        states["start_line"] = True
                    elif c = ';':
                        states["comment"] = True
                    elif re.match(r'[a-zA-Z\d]', c):
                        states["exit"] = True
                        self.file = c + self.file
                    else:
                        states["error"] = True
                case "comment":
                    state[s] = False
                    if c == '\n':
                        state["start_line"] = True
                    else:
                        state["comment"] = True
                case "end_line":
                    pass
                case "label_done":
                    pass
                case "directive":
                    pass
                case "byt0":
                    pass
                case "byt1":
                    pass
                case "byt2":
                    pass
                case "int0":
                    pass
                case "int1":
                    pass
                case "int2":
                    pass
                case "alloc_int":
                    pass
                case "alloc_byt":
                    pass
                case "add_int":
                    pass
                case "add_byte_num":
                    pass
                case "add_byte_char":
                    pass
                case "error":
                    pass
                case "exit":
                    self.file = c + self.file
                    return 0
                case _:
                    raise ValueError("invalid state found")

    
    # convert instructions to bytes, store in self.mem
    def parse_instr(self) -> int:
        pass

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
