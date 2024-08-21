#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"


struct{
    struct spinlock lock;
    char seed;
}   random;

int randomwrite(int user_src, uint64 src, int n){
    if(n != 1){ //If n is not 1, the function should return -1.
        return -1;
    }
    acquire(&random.lock); 
    if(either_copyin(&random.seed, user_src, src, 1) == -1){ //seed the random number generator with the byte pointed to by src
        release(&random.lock);
        return -1;
    }
    release(&random.lock);  
    return 1;     
}

int randomread(int user_dst, uint64 dst, int n){
    int written = 0;
    acquire(&random.lock);
    for(int i = 0; i < n; i++){
        random.seed = lfsr_char(random.seed);
        if(either_copyout(user_dst, dst, &random.seed, 1) == -1){
            release(&random.lock);
            return written; // On failure,return the amount of bytes it managed to write before the failure.
        }
        written++;
        dst++;  
    }
    release(&random.lock);
    return written;
}

void randominit(void)
{
  initlock(&random.lock, "random");
  random.seed = 0x2A;
  // connect read and write system calls
  // to randomread and randomwrite.
  devsw[RANDOM].read = randomread;
  devsw[RANDOM].write = randomwrite;
}

uint8 lfsr_char(uint8 lfsr)
{
uint8 bit;
bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 0x01;
lfsr = (lfsr >> 1) | (bit << 7);
return lfsr;
}
