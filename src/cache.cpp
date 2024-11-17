#include "../include/cache.h"
#include "../include/emu4380.h"
#include <cmath>
#include <iostream>

// *****
// cache
// *****
cache_line cache[CACHE_SIZE] = {0};
unsigned int cache_set_size = 0;
unsigned long long cache_counter = 0;

// get byte from the cache
cache_byte get_cache_byte(unsigned int address) {
    // increment cache counter
    cache_counter++;

    cache_byte cb;
    // cache penalty of 1 added
    cb.penalty = 1;

    unsigned int s_size = (int)log2(CACHE_SIZE/cache_set_size);
    unsigned int block_bits = (int)log2(BLOCK_SIZE);
    unsigned int addr_bits = 32;
    unsigned int mask = 1;
    unsigned long long lru;
    cache_line old_line;

    // divide address into tag, s, and block offset
    mask = (mask << s_size) - 1;
    unsigned int s = address & (mask << block_bits);
    s = s >> block_bits;

    mask = 1;
    unsigned int block_offset = address & ((mask << block_bits) - 1);

    mask = 1;
    unsigned int tag = address >> (block_bits + s_size);

    // check if tag is in cache
    int pos;
    for (pos = 0; pos < cache_set_size; ++pos) {
        cache_line cl = cache[s * cache_set_size + pos];
        if (cl.valid && tag == cl.tag)
            goto get_byte;
    }

    // get least recently used line in set
    pos = 0;
    lru = cache_counter;
    for (int i = 0; i < cache_set_size; ++i) {
        if (!cache[s * cache_set_size + i].valid) {
            cache[s * cache_set_size + i].valid = true;
            pos = i;
            goto read_line; 
        }
        if (cache[s * cache_set_size + i].last_used < lru) {
            pos = i;
            lru = cache[s * cache_set_size + i].last_used;
        }
    }

    // save value in cache to mem before overwrite
    old_line = cache[s * cache_set_size + pos];
    for (int i = 0; i < 4; ++i) {
        unsigned int* word = reinterpret_cast<unsigned int*>(&old_line.block[i * WORD_SIZE]);
        *reinterpret_cast<unsigned int*>(prog_mem + (old_line.tag << (s_size + block_bits)) + (s << block_bits) + i * WORD_SIZE) = *word;
    }
    cb.penalty += 8 + 2 * 3;

    // tag not in line, read line from memory
    read_line:
    for (int i = 0; i < 4; ++i) {
        unsigned int word = *reinterpret_cast<unsigned int*>(prog_mem + (tag << (s_size + block_bits)) + (s << block_bits) + i * WORD_SIZE);
        for (size_t j = 0; j < WORD_SIZE; ++j)
            cache[s * cache_set_size + pos].block[i * WORD_SIZE + j] = *(reinterpret_cast<unsigned char*>(&word) + j); 
    }
    cb.penalty += 8 + 2 * 3;

    // get byte from cache
    get_byte:
    cb.byte = cache[s * cache_set_size + pos].block[block_offset];
    cache[s * cache_set_size + pos].last_used = cache_counter;

//    for (int i = 0; i < 16; ++i) {
//        std::cout << (int)cache[s*cache_set_size+pos].block[i] << " : " << (tag << (s_size + block_bits)) + (s << block_bits) + i << std::endl;
//    }
//    std::cout << std::endl;

    return cb;
}

// get word from the cache
cache_word get_cache_word(unsigned int address) {
    cache_word cw;

    // tag not in line, read line from mem
    for (int i = 0; i < 4; ++i) {
        unsigned int word = *reinterpret_cast<unsigned int*>(prog_mem + address + i * WORD_SIZE);
        for (size_t i = 0; i < WORD_SIZE; ++i) 
            cache[0].block[0] = *(reinterpret_cast<unsigned char*>(&word) + i); 
    }

    return cw;
}

