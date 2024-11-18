#ifndef CACHE_H
#define CACHE_H
#include <vector>

// ***************************
// declare components of cache
// ***************************

#define CACHE_SIZE 64
#define BLOCK_SIZE 16

// cache line
struct cache_line {
    bool valid = 0;
    unsigned int tag;
    unsigned char block[BLOCK_SIZE];
    unsigned int last_used = 0;
};

// cache return values
// use word or byte based on the function calling cache
struct cache_word {
    unsigned int penalty;
    std::vector<unsigned int> words;
};

struct cache_byte {
    unsigned int penalty;
    unsigned char byte;
    bool read_mem = false;
    bool wrote_mem = false;
};

// cache
extern cache_line cache [];
extern unsigned int cache_set_size;
extern unsigned long long cache_counter;
// accessed_mem allows us to determine if memory has already be accessed so when the function is called again we know what the penalty is
cache_byte get_cache_byte(unsigned int address, bool read_mem = false, bool wrote_mem = false);
cache_word get_cache_words(unsigned int address, unsigned int num_words = 1);
cache_byte write_cache_byte(unsigned int address, unsigned char byte, bool read_mem = false, bool wrote_mem = false);
unsigned int write_cache_word(unsigned int address, unsigned int word);

#endif
