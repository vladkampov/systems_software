#ifndef LAB2_MEMORY_H
#define LAB2_MEMORY_H
#include <vector>
#include <iostream>
using namespace std;
class Memory {
    // Data structure for location in memory
    struct mem_location {
        int page;
        int block;
        mem_location(int page, int block) : page(page), block(block) { }
    };
    // Data structure for information about block of pages
    struct pages_with_state_3_block {
        uint16_t start_idx;
        uint16_t number_of_pages;
        pages_with_state_3_block(uint16_t start_idx, uint16_t number_of_pages): start_idx(start_idx),
                                                                                number_of_pages(number_of_pages) { }
    };
    struct Page {
        uint16_t page_size;
        uint8_t state = 0;  // 0 - free; 2 - divided to blocks; 3 - whole in use
        uint16_t block_size;
        uint16_t blocks_count;
        // Info about used blocks
        vector<uint16_t> in_use_blocks_info;
        Page(const uint16_t page_size) {
            this->page_size = page_size;
        }
        // Splits page to blocks
        void split_to_blocks(uint16_t block_length);
        // Checks for free block
        bool have_free_block() {
            return (blocks_count != in_use_blocks_info.size());
        }
    };
    class Memory_unit {
        int memory;
    public:
        void operator=(const Memory_unit &unit) {
            this->memory = unit.memory;
        }
    };
    // Pages
    vector<Page> pages;
    vector<pages_with_state_3_block> pages_blocks;
    // Real memory
    vector<Memory_unit> memory_block;
    // Returns size aligned to 4B
    uint32_t size_with_align_4B(uint32_t size);
    // Calculates offset in real memory
    uint16_t offset(uint16_t page, uint16_t block);
public:
    Memory(uint16_t pages_number, uint16_t page_size);
    // Returns addr location in memory (page and block index in page)
    Memory::mem_location what_number_am_i(void *addr);
    // Allocates memory. If cannot returns NULL
    void *mem_alloc(size_t size);
    // Reallocates memory. Firstly tries to extend current block and if unsuccessful to find new
    void *mem_realloc(void *addr, size_t size);
    // Make memory free
    void mem_free(void *addr);
    // Prints memory info. '_' - free byte, '*' - byte in use
    void mem_dump();
};
#endif //LAB2_MEMORY_H
