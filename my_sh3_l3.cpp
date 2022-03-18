// Must run as CPP file because <std::unordered_map> is used

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <string>
#include <unordered_map>


#define MEM_CAPACITY 1024
#define SLOT_SIZE 4
#define NUM_MEM_SLOTS MEM_CAPACITY/SLOT_SIZE
#define LARGEST_BLOCK 100

#define MAX_SLOTS_PER_BLOCK LARGEST_BLOCK/SLOT_SIZE
#define MIN_SLOTS_PER_BLOCK 1

#define EMPTY_SLOT_VALUE 0



// Used to construct the overall memory region.
typedef struct AllocatableMemBlock{
  int val;
  int firstSlot;
  int lastSlot;
}; 
AllocatableMemBlock MEM[NUM_MEM_SLOTS]; // MAIN MEMORY
AllocatableMemBlock _nullBlock;         // predefined null-block used for unallocated slots
int NUM_PROCESSES = 0;

/*Track postitions of MemBlocks to easily release them in `releasePercent`. 
  k: first-slot of an allocated block
  v: the AllocatedMemBlock "object".*/
std::unordered_map<int,AllocatableMemBlock> ALLOCATED;



void allocate(AllocatableMemBlock block){
  for(int i = block.firstSlot; i <= block.lastSlot; i++)
    MEM[i] = block;
  ALLOCATED[block.firstSlot] = block; // tracks where blocks are allocated
  NUM_PROCESSES++;
}


void release(AllocatableMemBlock block){
  ALLOCATED.erase(block.firstSlot);   // stop tracking released blocks
  for(int i = block.firstSlot; i <= block.lastSlot; i++)
    MEM[i] = _nullBlock;
  printf("Block with val <%d> removed from from <%d-%d\n>", 
    block.val, block.firstSlot, block.lastSlot);
  NUM_PROCESSES--;
}

void releasePercent(float percent){
  int numToRelease = NUM_PROCESSES * percent;
  for(int i=0; i<numToRelease; i++){
    // get random block in ALLOCATED map
    auto it = ALLOCATED.begin();
    std::advance(it, rand() % ALLOCATED.size());
    int firstSlot = it->first;
    // release block from MEM
    release(ALLOCATED[firstSlot]);
    // update map after we remove the block
    ALLOCATED.erase(firstSlot);
  }
}

void compact(){
  int fullSlotCount = 0;
  int nullSlotCount = 0;
  for(int i = 0; i < NUM_MEM_SLOTS; i++){
    if(MEM[i].val != EMPTY_SLOT_VALUE){
      // We are moving MemBlock, so erase the current entry.
      ALLOCATED.erase(MEM[i].firstSlot);
      // Copy to new location in MEM
      int newFirstSlot = MEM[i].firstSlot - nullSlotCount;
      int newLastSlot = MEM[i].lastSlot - nullSlotCount;
      MEM[fullSlotCount].val = newFirstSlot; 
      MEM[fullSlotCount].firstSlot = newFirstSlot;
      MEM[fullSlotCount].lastSlot = newLastSlot;
      // Update map
      ALLOCATED[newFirstSlot] = MEM[fullSlotCount];
      fullSlotCount++;
    }      
    else{
      nullSlotCount++;
    }
  }
  // After compaction, the old residual slots are at the end.
  while(fullSlotCount < NUM_MEM_SLOTS){
    MEM[fullSlotCount] = _nullBlock;
    fullSlotCount++;
  }  
}


void fillEmptyMemory(){ 
  int currFirstVacantSlot = 0;
  while(MEM[NUM_MEM_SLOTS-1].val == EMPTY_SLOT_VALUE){
    int rand_memblock_size = (rand() % (MAX_SLOTS_PER_BLOCK - MIN_SLOTS_PER_BLOCK + 1)) + MIN_SLOTS_PER_BLOCK;;
    int _blockLastSlot = currFirstVacantSlot + rand_memblock_size - 1;
    if(_blockLastSlot < NUM_MEM_SLOTS){
      AllocatableMemBlock _block;
      _block.val=5;
      _block.firstSlot = currFirstVacantSlot;
      _block.lastSlot = _blockLastSlot;
      allocate((_block));
      currFirstVacantSlot = _block.lastSlot + 1;    
    }
  }
}


void status(){
  for(int i = 0; i < 64; i++)
    printf("index %d: %d\t index %d: %d\t index %d: %d\t index %d: %d\t\n", i, MEM[i].val, i+64, MEM[i+64].val, i+128, MEM[i+128].val, i+192, MEM[i+192].val);
}


int main(void) {
  // predefined null-block used for unallocated slots
  _nullBlock.val = EMPTY_SLOT_VALUE;
  _nullBlock.firstSlot = 0;
  _nullBlock.lastSlot = 0;

  fillEmptyMemory();
  int kill = 100;
  status();
  printf("%d NUM PROCESSES: ", NUM_PROCESSES);
  printf("\n\n");
  releasePercent(0.5);
  compact();
  status();
  printf("%d NUM PROCESSES: ", NUM_PROCESSES);
}