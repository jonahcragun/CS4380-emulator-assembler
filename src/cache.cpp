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
cache_byte get_cache_byte(unsigned int address, bool read_mem, bool wrote_mem) {
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
    if (!wrote_mem) {
        cb.penalty += 8 + 2 * 3;
        cb.wrote_mem = true;
    }
    else {
        cb.penalty += 2 * 4;
    }

    // tag not in line, read line from memory
    read_line:
    for (int i = 0; i < 4; ++i) {
        unsigned int word = *reinterpret_cast<unsigned int*>(prog_mem + (tag << (s_size + block_bits)) + (s << block_bits) + i * WORD_SIZE);
        for (size_t j = 0; j < WORD_SIZE; ++j)
            cache[s * cache_set_size + pos].block[i * WORD_SIZE + j] = *(reinterpret_cast<unsigned char*>(&word) + j); 
    }
    cache[s * cache_set_size + pos].tag = tag;
    if (!read_mem) {
        cb.penalty += 8 + 2 * 3;
        cb.read_mem = true;
    }
    else {
        cb.penalty += 2 * 4;
    }

    // write byte to cache
    get_byte:
    cb.byte = cache[s * cache_set_size + pos].block[block_offset];
    cache[s * cache_set_size + pos].last_used = cache_counter;
    if (read_mem)
        cb.read_mem = true;
    if (wrote_mem)
        cb.wrote_mem = true;

    return cb;
}

// get word from the cache
cache_word get_cache_words(unsigned int address, unsigned int num_words) {
    cache_word cw;
    unsigned char word[4];
    bool read_mem = false;
    bool wrote_mem = false;

    cw.penalty = 1;
    
    for (int j = 0; j < num_words; ++j) {
        for (int i = 0; i < WORD_SIZE; ++i) {
            cache_byte cb = get_cache_byte(address + i + (j * WORD_SIZE), read_mem, wrote_mem);
            cw.penalty += cb.penalty - 1;
            word[i] = cb.byte;
            read_mem = cb.read_mem;
            wrote_mem = cb.wrote_mem;
        }
        cw.words.push_back(*reinterpret_cast<unsigned int*>(word));
    }

    return cw;
}

cache_byte write_cache_byte(unsigned int address, unsigned char byte, bool read_mem, bool wrote_mem) {
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
    if (!wrote_mem) {
        cb.penalty += 8 + 2 * 3;
        cb.wrote_mem = true;
    }
    else {
        cb.penalty += 2 * 4;
    }

    // tag not in line, read line from memory
    read_line:
    for (int i = 0; i < 4; ++i) {
        unsigned int word = *reinterpret_cast<unsigned int*>(prog_mem + (tag << (s_size + block_bits)) + (s << block_bits) + i * WORD_SIZE);
        for (size_t j = 0; j < WORD_SIZE; ++j)
            cache[s * cache_set_size + pos].block[i * WORD_SIZE + j] = *(reinterpret_cast<unsigned char*>(&word) + j); 
    }
    cache[s * cache_set_size + pos].tag = tag;
    if (!read_mem) {
        cb.penalty += 8 + 2 * 3;
        cb.read_mem = true;
    }
    else {
        cb.penalty += 2 * 4;
    }

    // write byte to cache
    get_byte:
    cache[s * cache_set_size + pos].block[block_offset] = byte;
    cache[s * cache_set_size + pos].last_used = cache_counter;
    if (read_mem)
        cb.read_mem = true;
    if (wrote_mem)
        cb.wrote_mem = true;

    return cb;
    
}

unsigned int write_cache_word(unsigned int address, unsigned int word) {
    unsigned int penalty;
    bool read_mem = false;
    bool wrote_mem = false;

    penalty = 1;
    
    for (int i = 0; i < WORD_SIZE; ++i) {
        cache_byte cb = write_cache_byte(address + i, *(reinterpret_cast<unsigned char*>(&word) + i), read_mem, wrote_mem);
        penalty += cb.penalty - 1;
        read_mem = cb.read_mem;
        wrote_mem = cb.wrote_mem;
    }

    return penalty;

}
