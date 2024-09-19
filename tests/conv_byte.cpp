#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char** argv) {
    if (argv[1][0] == '1') {
        vector<unsigned char> v;
        ifstream ifs (argv[2]);
        if (!ifs) return -1;

        // read file
        unsigned int val;
        while (ifs >> val) {
            v.push_back((unsigned char)val);
        }
        ifs.close();

        // for (auto i : v) {
        //     cout << (unsigned int) i << endl;
        // }
        
        // write file to binary
        ofstream ofs (argv[3], ios::binary);
        ofs.write((char *) v.data(), v.size()*sizeof(unsigned char));
        ofs.close();
    }
    else if (argv[1][0] == '0'){
        size_t mem_size = 300;
        unsigned char mem[mem_size] {0};
        std::ifstream ifs (argv[2], ios::binary);
        if (!ifs) return 3;
        // set end
        ifs.seekg(0, ios_base::end);
        auto len = ifs.tellg();
        
        ifs.seekg(0, ios_base::beg);
        ifs.read((char *)mem, len); 

        ifs.close();

        for (int i = 0; i < mem_size; i++) {
            cout << *(unsigned int*)(mem + i) << " ";
            if (i % 8 == 0) cout << endl;
        }
    }
    else {
        cout << "invalid params" << endl;
    }
}
