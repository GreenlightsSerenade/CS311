#include "cache.h"
#include "util.h"

/* cache.c : Implement your functions declared in cache.h */

int miss_penalty;
uint32_t ***Cache;
Cachedata **C_data;
short hit_flag;

int C_cap;
int C_nw;
int C_bs;
int nset;
int _wpb;

/***************************************************************/
/*                                                             */
/* Procedure: setupCache                                     */
/*                                                             */
/* Purpose: Allocates memory for your cache                    */
/*                                                             */
/***************************************************************/

void setupCache(int capacity, int num_way, int block_size)
{
/*        code for initializing and setting up your cache    */
/*        You may add additional code if you need to    */

        int i, j, k; //counter
        C_cap = capacity;
        C_nw = num_way;
        C_bs = block_size;

        nset=C_cap/(C_bs * C_nw);
        _wpb = C_bs/BYTES_PER_WORD;

        Cache = (uint32_t ***)malloc(nset * sizeof(uint32_t **));
        C_data = (Cachedata **)malloc(nset * sizeof(Cachedata *));
        
        // set up Cache and C_data
        for (i=0;i<nset;i++) {
            Cache[i] = (uint32_t **)malloc(C_nw * sizeof(uint32_t *));
            C_data[i] = (Cachedata *)malloc(C_nw * sizeof(Cachedata));
        }
        for (i=0; i<nset; i++) {
            for (j=0; j<C_nw; j++)
                Cache[i][j] = (uint32_t *)malloc(_wpb * sizeof(uint32_t));
        }
        
        // initialize
        for (i = 0; i < nset; i++) {
            for (j = 0; j < C_nw; j++) {
                C_data[i][j].valid = 0;
                C_data[i][j].dirty = 0;
                C_data[i][j].LRU = 0;
                C_data[i][j].addr = 0x0;
                C_data[i][j].tag = 0x0;
                for (k = 0; k < _wpb; k++) {
                    Cache[i][j][k] = 0;
                }
            }
        }
}


/***************************************************************/
/*                                                             */
/* Procedure: setCacheMissPenalty                                 */
/*                                                             */
/* Purpose: Sets how many cycles your pipline will stall       */
/*                                                             */
/***************************************************************/

void setCacheMissPenalty(int penalty_cycles)
{
/*        code for setting up miss penaly        	*/
/*        You may add additional code if you need to    */    
        miss_penalty = penalty_cycles;

}

/* Please declare and implement additional functions for your cache */

uint32_t readCache(uint32_t address)
{
    uint32_t idx, tag;
    uint32_t erase_idx;
    int i, j, pos;
    idx = (address / C_bs) % nset;
    tag = address / (nset * C_bs);
    pos = (address % C_bs) / BYTES_PER_WORD;

    short debug = FALSE;

    if (debug) {
        printf("reading cache.../n");
        printf("addr: %x\n", address);
    }

    for (i = 0; i < C_nw; i++) {
        if (C_data[idx][i].valid == 0)
            continue;
        else {          // if it is valid
            if (debug) printf("idx : %d, i : %d, tag : %d | given tag : %d\n", idx, i, C_data[idx][i].tag, tag);
            if (C_data[idx][i].tag == tag) {
		hit_flag = 1;
                LRU_update(idx, i);
                if (debug) printf("hit! : %x\n", Cache[idx][i][pos]);
	        return Cache[idx][i][pos]; // if it is hit, return data
            }
        }
    }
	    
    // miss
    hit_flag = 0;
    erase_idx = LRU_erasecheck(idx);
    if (debug) printf("erase_idx: %d\n", erase_idx);    
    if (C_data[idx][erase_idx].dirty)
        mem_write_block(C_data[idx][erase_idx].addr, Cache[idx][erase_idx]);
    C_data[idx][erase_idx].valid = 1;
    C_data[idx][erase_idx].dirty = 0;
    C_data[idx][erase_idx].LRU = C_nw;
    C_data[idx][erase_idx].addr = (address & (0xffffffff - C_bs + 1));
    //if (debug) printf("address saved: %x\n", (address & (0xffffffff - C_bs + 1)));
    C_data[idx][erase_idx].tag = tag;
    LRU_update(idx, erase_idx);     // update the data using LRU
    mem_read_block(C_data[idx][erase_idx].addr, Cache[idx][erase_idx]);
    if (debug) printf("miss! : %x\n", Cache[idx][erase_idx][pos]);

    return Cache[idx][erase_idx][pos];
}

void writeCache(uint32_t address, uint32_t value)
{
    uint32_t idx, tag;
    uint32_t erase_idx;
    int i, j, pos;
    idx = (address / C_bs) % nset;
    tag = address / (nset * C_bs);
    pos = (address % C_bs) / BYTES_PER_WORD;

    short debug = FALSE;

    if (debug) printf("writing cache ...\naddr : %x, data : %x\n", address, value);

    for (i = 0; i < C_nw; i++) {
        if (C_data[idx][i].valid == 0)
            continue;
        else {      // if it is valid
            if (debug) printf("idx : %d, i : %d, tag : %d | given tag : %d\n", idx, i, C_data[idx][i].tag, tag);
            if (C_data[idx][i].tag == tag) {
		hit_flag = 1;
                LRU_update(idx, i);
                C_data[idx][i].dirty = 1;
                Cache[idx][i][pos] = value;
                if (debug) printf("hit!\n");
                return;         // if it is hit, make the block dirty and return
            }
        }
    }

    // miss
    hit_flag = 0;
    erase_idx = LRU_erasecheck(idx);
    if (debug) printf("erase_idx: %d\n", erase_idx);
    if (C_data[idx][erase_idx].dirty)
        mem_write_block(C_data[idx][erase_idx].addr, Cache[idx][erase_idx]);
    if (debug) printf("writing block -> addr: %x / %d, %d\n", C_data[idx][erase_idx].addr, idx, erase_idx);

    //mem_read_block(C_data[idx][erase_idx].addr, Cache[idx][erase_idx]);
    mem_read_block(address & (0xffffffff - C_bs + 1), Cache[idx][erase_idx]);
    if (debug) printf("test2\n");
    C_data[idx][erase_idx].valid = 1;
    C_data[idx][erase_idx].dirty = 0;
    C_data[idx][erase_idx].LRU = C_nw;
    C_data[idx][erase_idx].addr = (address & (0xffffffff - C_bs + 1));
    C_data[idx][erase_idx].tag = tag;
    Cache[idx][erase_idx][pos] = value;
    LRU_update(idx, erase_idx);     // if it is miss, replace the block using LRU

    if (debug) printf("miss!\n");
}

/* LRU_update : update the block's using frequency */
void LRU_update(int set, int way)
{
    int i;
    uint32_t now = C_data[set][way].LRU;
    for (i = 0; i < C_nw; ++i)
    {
        if (C_data[set][i].valid == 0)
            continue;
        else if (i == way)
            C_data[set][i].LRU = 0;
        else {
	    if (C_data[set][i].LRU < now)
	        ++C_data[set][i].LRU;
        }
    }
}

/* LRU_erasecheck : find the least used block and return the index */
uint32_t LRU_erasecheck(int idx)
{
    int i;
    int maxval = 0, maxidx;
    for (i = 0; i < C_nw; ++i) {
        if (C_data[idx][i].valid == 0)  // if a block is not valid, return that block
            return i;
        if (C_data[idx][i].LRU >= maxval) {
            maxval = C_data[idx][i].LRU;
            maxidx = i;
        }
    }
    return maxidx;
}
