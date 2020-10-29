#include <assert.h> // needed for asserts
#include <math.h> // needed for log2
#include <unistd.h> // needed for sbrk etc.
#include <sys/mman.h> // needed for mmap
#include <stdio.h>  // needed for size_t etc.
#include "dmm.h"

/* 
 * The lab handout and code guide you to a solution with a single free list containing all free
 * blocks (and only the blocks that are free) sorted by starting address.  Every block (allocated
 * or free) has a header (type metadata_t) with list pointers, but no footers are defined.
 * That solution is "simple" but inefficient.  You can improve it using the concepts from the
 * reading.
 */

/* 
 *size_t is the return type of the sizeof operator.   size_t type is large enough to represent
 * the size of the largest possible object (equivalently, the maximum virtual address).
 */

typedef struct metadata {
  bool is_free;
  size_t size; // size of data
  struct metadata* next;
  struct metadata* prev;
} metadata_t;

// footer is used for coalesce adjacent free blocks.
typedef struct footer {
  bool is_free;
  size_t size; // size of data
} footer_t;

// Start address and end address of the pre-allocated memory. Valid addressed
// should be within [start_addr, end_addr).
static void* start_addr = NULL;
static void* end_addr = NULL;

// Array to store the header of linked list of various size.
bool has_initialized = false;
static metadata_t* arr[ENTRY_SIZE];

// Get the index of the array according to the block size.
// eg: size = 1 -> 0
//     size = 8 -> 3
size_t get_index(size_t size) {
  assert(size > 0);
  size_t idx = 0;
  size_t val = 1;
  while (size > val) {
    ++idx;
    val <<= 1;
  }
  return idx;
}

// Assert a free block is the header of an entry.
bool check_free_block(metadata_t* entry) {
  size_t size = entry->size;
  size_t index = get_index(size);
  if (arr[index] == NULL) {
    return false;
  }
  return arr[index] == entry;
}

// Check whether there could be previous address.
bool has_previous_address(metadata_t* ptr) {
  assert(ptr != NULL);
  assert(start_addr != NULL);
  return ptr != start_addr;
}

// Check whether there could be next address.
bool has_next_address(metadata_t* ptr) {
  void* cur_end_addr = (void*)ptr + METADATA_T_ALIGNED + ptr->size + 
    FOOTER_T_ALIGNED;
  assert(ptr != NULL);
  assert(cur_end_addr != NULL);
  return cur_end_addr != end_addr;
}

// Called when dmalloc() finds an available free block.
void set_occupied(metadata_t* ptr, size_t size) {
  ptr->is_free = false;
  footer_t* footer = (footer_t*)((void*)ptr + ptr->size + METADATA_T_ALIGNED);
  footer->is_free = false;
  footer->size = size;
}

// Remove a node from a free list. Called at
// (1) dmalloc() when an available free block has been found.
// (2) coalesce() when node can be coalesced with prev or next node.
void remove_from_list(metadata_t* ptr) {
  assert(ptr != NULL);

  // Get corresponding entry.
  size_t index = get_index(ptr->size);

  // Corner case: ptr is head of the current free list, update entry.
  if (ptr->prev == NULL) {
    // Assert ptr is the header.
    assert(arr[index] == ptr);

    arr[index] = ptr->next;
  }

  // Update relation with prev block, only update prev.
  if (ptr->prev != NULL) {
    ptr->prev->next = ptr->next;
  } 

  // Update relation with next block, only update next.
  if (ptr->next != NULL) {
    ptr->next->prev = ptr->prev;
  } 

  // Update current ptr.
  ptr->prev = NULL;
  ptr->next = NULL;
}

// Add block to the current free list.
void add_to_free_list(metadata_t* ptr) {
  assert(ptr != NULL);

  size_t index = get_index(ptr->size);
  metadata_t* head = arr[index];
  arr[index] = ptr;

  // Corner case: if there's no free block inside of the free list.
  if (head == NULL) {
    return;
  }

  // Update pointer related to this two free blocks.
  ptr->next = head;
  ptr->prev = NULL;
  head->prev = ptr;
}

// Split a large free block into two; add left block back to the free list,
// return the allocated one. Called at dmalloc().
metadata_t* split_free_block(metadata_t* large_block, size_t total_size) {
  assert(large_block != NULL);

  // Make sure large_block can be splitted validly.
  assert(large_block->size >= total_size + ALIGNMENT);
  size_t left_size = large_block->size - total_size;

  // Split large_block into two, return the first part, add second part into
  // array. Initialize the allocated_block, including nullify prev/next, size
  // at header; and is_free, size at footer.
  metadata_t* allocated_block = large_block;
  allocated_block->is_free = false;
  allocated_block->size = total_size - METADATA_T_ALIGNED - FOOTER_T_ALIGNED;
  allocated_block->prev = NULL;
  allocated_block->next = NULL;
  footer_t* allocated_block_footer = (footer_t*)((void*)allocated_block + 
    METADATA_T_ALIGNED + allocated_block->size);
  allocated_block_footer->is_free = false;
  allocated_block_footer->size = allocated_block_footer->size;

  // Split left block.
  metadata_t* left_block = (metadata_t*)((void*)large_block + total_size);
  left_block->is_free = true;
  left_block->size = left_size;
  left_block->prev = NULL;
  left_block->next = NULL;
  footer_t* left_block_footer = (footer_t*)((void*)left_block + 
    METADATA_T_ALIGNED + left_block->size);
  left_block_footer->is_free = true;
  left_block_footer->size = left_size;

  // Append the left block into the array.
  add_to_free_list(left_block);

  return allocated_block;
}

// Helper function for coalesce(), coalesce with previous block if possible.
metadata_t* coalesce_with_prev(metadata_t* ptr) {
  assert(ptr != NULL);

  // Check whether possible to coalesce with previous block.
  bool has_prev = has_previous_address(ptr);
  if (!has_prev) {
    return ptr;
  }

  // Assert.
  assert((void*)ptr > start_addr);

  // Get footer of previous block, and check if it's free.
  footer_t* prev_footer = (footer_t*)((void*)ptr - FOOTER_T_ALIGNED);
  footer_t* cur_footer = (footer_t*)((void*)ptr + METADATA_T_ALIGNED + 
    ptr->size);
  if (!prev_footer->is_free) {
    // If cannot merge, merely set is_free to true.
    ptr->is_free = true;
    cur_footer->is_free = true;
    return ptr;
  }

  // Remove previous free block out of free list.
  metadata_t* prev_header = (metadata_t*)((void*)prev_footer - 
    prev_footer->size - METADATA_T_ALIGNED);
  remove_from_list(prev_header);

  // Coalesce with previous free block, update header.
  metadata_t* new_block_header = prev_header;
  size_t new_block_size = prev_header->size + FOOTER_T_ALIGNED + 
    METADATA_T_ALIGNED + ptr->size;
  new_block_header->is_free = true;
  new_block_header->size = new_block_size;
  new_block_header->prev = NULL;
  new_block_header->next = NULL;

  // Update current footer.
  cur_footer->size = new_block_size;
  cur_footer->is_free = true;
  
  return new_block_header;
}

// Helper function for coalesce(), coalesce with next block if possible.
metadata_t* coalesce_with_next(metadata_t* ptr) {
  assert(ptr != NULL);

  // Check whether possible to coalesce with next block.
  bool has_next = has_next_address(ptr);
  if (!has_next) {
    return ptr;
  }

  // Assert.
  void* cur_end_addr = (void*)ptr + METADATA_T_ALIGNED + ptr->size + 
    FOOTER_T_ALIGNED;
  assert(cur_end_addr < end_addr);

  // Get header of next block, and check if it's free.
  metadata_t* next_header = (metadata_t*)((void*)ptr + METADATA_T_ALIGNED + 
    ptr->size + FOOTER_T_ALIGNED);
  if (!next_header->is_free) {
    // If cannot merge with next block, merely set is_free to true.
    footer_t* cur_footer = (footer_t*)((void*)ptr + METADATA_T_ALIGNED + 
      ptr->size);
    ptr->is_free = true;
    cur_footer->is_free = true;
    return ptr;
  }

  // Remove next free block out of free list.
  remove_from_list(next_header);

  // Coalesce with next free block, update data size in header and footer.
  // Update current header(size, is_free);
  metadata_t* new_block = ptr;
  size_t new_block_size = ptr->size + FOOTER_T_ALIGNED + METADATA_T_ALIGNED +
    next_header->size;
  new_block->is_free = true;
  new_block->size = new_block_size;
  new_block->prev = NULL;
  new_block->next = NULL;

  // Update current footer.
  footer_t* next_footer = (footer_t*)((void*)next_header + METADATA_T_ALIGNED +
    next_header->size);
  next_footer->size = new_block_size;
  next_footer->is_free = true;

  return new_block;
}

// Try to coalesce with adjacent free blocks.
// (1) Check whether there's prev and next block.
// (2) Try to merge with prev and next blocks respectively, remove prev/next
// block out of the free list.
// (3) Return merged ptr.
metadata_t* coalesce(metadata_t* ptr) {
  assert(ptr != NULL);
  ptr = coalesce_with_prev(ptr);
  ptr = coalesce_with_next(ptr);

  // Assert coalesced block.
  assert(ptr != NULL);
  assert(ptr->is_free == true);
  assert(ptr->prev == NULL);
  assert(ptr->next == NULL);
  footer_t* footer = (footer_t*)((void*)ptr + METADATA_T_ALIGNED + ptr->size);
  assert(footer->is_free == true);
  assert(footer->size == ptr->size);

  return ptr;
}

// (1) Calculate total consumed memory(data, header, footer).
// (2) Get the smallest available block:
// 1/ There's already such free block in free list.
// 2/ Otherwise, fetch and split larger block. Add left block to the free list;
// If no avaible free block, return NULL to user.
// (3) Update state: prev, next, is_free, size(in header/footer).
void* dmalloc(size_t numbytes) {
  // Allocate the initial memory.
  if (!has_initialized) {
    if (!dmalloc_init()) {
      return NULL;
    }
  }

  assert(numbytes > 0);

  // Iterate through the free list array, find the first available free block.
  size_t total_size = METADATA_T_ALIGNED + FOOTER_T_ALIGNED + ALIGN(numbytes);
  metadata_t* allocated_block = NULL;
  size_t start_index = get_index(ALIGN(numbytes));
  for (size_t ii = start_index; ii < ENTRY_SIZE; ++ii) {
    // No available free block for this entry.
    if (arr[ii] == NULL) {
      continue;
    }
    
    for (metadata_t* block = arr[ii]; block != NULL; block = block->next) {
      if (block->size < ALIGN(numbytes)) {
        continue;
      }

      // Remove the first available block from free list array.
      metadata_t* available_block = block;
      remove_from_list(available_block);
      
      // Check if block can splitted.
      size_t actual_size = 0; // actual size allocated for the required block
      if (available_block->size >= total_size + ALIGNMENT) {
        allocated_block = split_free_block(available_block, total_size);
        actual_size = ALIGN(numbytes);

        // Assert left block.
        metadata_t* left_block = (metadata_t*)((void*)allocated_block + 
          METADATA_T_ALIGNED + allocated_block->size + FOOTER_T_ALIGNED);
        footer_t* left_block_footer = (footer_t*)((void*)left_block + 
          METADATA_T_ALIGNED + left_block->size);
        assert(check_free_block(left_block));
        assert(left_block->is_free == true);
        assert(left_block->prev == NULL);
        assert(left_block_footer->is_free == true);
        assert(left_block_footer->size == left_block->size);

      } else {
        allocated_block = available_block;
        actual_size = available_block->size;
      }

      // Block block splitting.
      allocated_block = available_block;
      actual_size = available_block->size;

      set_occupied(allocated_block, actual_size);
      break;
    }

    // If available block has been found, break.
    if (allocated_block != NULL) {
      break;
    }
  }

  // No available free block.
  if (allocated_block == NULL) {
    return NULL;
  }

  // Address assertion.
  assert(start_addr <= (void*)allocated_block);
  assert((void*)allocated_block + METADATA_T_ALIGNED + allocated_block->size 
    + FOOTER_T_ALIGNED <= end_addr);

  // Pointer assertion.
  assert(allocated_block->prev == NULL);
  assert(allocated_block->next == NULL);
  assert(allocated_block->is_free == false);
  footer_t* footer = (footer_t*)((void*)allocated_block + METADATA_T_ALIGNED + 
    allocated_block->size);
  assert(footer->is_free == false);
  assert(allocated_block->size == footer->size);

  void* data_ptr = (void*)allocated_block + METADATA_T_ALIGNED;
  return data_ptr;
}

// (1) Get header of the current block, try to coalesce with prev and next
// block, get the possibly merged free block.
// (2) Append free block back to corresponding free block.
void dfree(void* ptr) {
  metadata_t* head = (metadata_t*)(ptr - METADATA_T_ALIGNED);
  assert(head->prev == NULL);
  assert(head->next == NULL);
  assert(head->is_free == false);
  metadata_t* merged_block = coalesce(head);
  add_to_free_list(merged_block);
}

// Allocate heap_region slab with a suitable syscall.
bool dmalloc_init() {
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  // Get a slab with mmap, and put it on the freelist as one large block, 
  // starting with an empty header.
  metadata_t* freelist = (metadata_t*)
     mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
     -1, 0);
  has_initialized = true;

  if (freelist == (void *)-1) {
    perror("dmalloc_init: mmap failed");
    return false;
  }

  // Initialize the start and end address of the pre-allocated memory.
  start_addr = freelist;
  end_addr = start_addr + max_bytes;

  // Initialize only block.
  size_t size = max_bytes - METADATA_T_ALIGNED - FOOTER_T_ALIGNED;
  freelist->is_free = true;
  freelist->prev = NULL;
  freelist->next = NULL;
  freelist->size = size;
  footer_t* footer = (footer_t*)((void*)freelist + METADATA_T_ALIGNED + size);
  footer->size = size;
  footer->is_free = true;

  // Initialize the free-list array;
  for (int ii = 0; ii < ENTRY_SIZE; ++ii) {
    arr[ii] = NULL;
  }
  size_t index = get_index(size);
  arr[index] = freelist;

  return true;
}