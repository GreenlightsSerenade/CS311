#ifndef _CACHE_H_
#define _CACHE_H_ 

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* cache.h : Declare functions and data necessary for your project*/

extern int miss_penalty; // number of cycles to stall when a cache miss occurs
extern uint32_t ***Cache; // data cache storing data [set][way][byte]

typedef struct Cache_elem { // cache information structure
    uint8_t valid;
    uint8_t dirty;
    uint32_t LRU;
    uint32_t addr;
    uint32_t tag;
} Cachedata;

extern Cachedata **C_data;
extern short hit_flag;  // to check if it is miss or hit

extern int C_cap;   // variables to help calculating tag, index, etc.
extern int C_nw;
extern int C_bs;
extern int nset;
extern int _wpb;

void setupCache(int, int, int);
void setCacheMissPenalty(int);
uint32_t readCache(uint32_t);
void writeCache(uint32_t, uint32_t);
void LRU_update(int, int);
uint32_t LRU_erasecheck(int);

#endif
