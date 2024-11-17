#ifndef CACHE_H
#define CACHE_H

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
    unsigned int word;
};

struct cache_byte {
    unsigned int penalty;
    unsigned char byte;
};

// cache
extern cache_line cache [];
extern unsigned int cache_set_size;
extern unsigned long long cache_counter;
cache_byte get_cache_byte(unsigned int address);
cache_word get_cache_word(unsigned int address);

#endif
