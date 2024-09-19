#include "../include/emu4380.h"
#include <iostream>
using namespace std;

int main() {
    
    init_mem(50);
    unsigned int ret = read_file("test1.bin");
    cout << ret << " " << ((unsigned int)prog_mem[0] == 8) << endl;
    for (size_t i = 0; i < mem_size; ++i) {
        cout << (unsigned int)prog_mem[i] << endl;
    }
    delete_mem();
}
