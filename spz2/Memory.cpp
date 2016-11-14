#include "Memory.h"
#include <algorithm>
#include <cmath>

Memory::Memory(uint16_t pages_number, uint16_t page_size) {
    // Page size must be 2 in some power
    long power_of_two = 2;
    while (power_of_two < pow(2, 20)) {
        power_of_two *= 2;
        if (power_of_two < page_size && page_size <= power_of_two * 2) {
            page_size = (uint16_t) (power_of_two * 2);
            break;
        }
    }
    page_size = (uint16_t) size_with_align_4B(page_size);
    pages = vector<Page>(pages_number, Page(page_size));
    pages_blocks = vector<pages_with_state_3_block>();
    memory_block = vector<Memory_unit>((unsigned long) (pages_number * page_size / 4));
}
void *Memory::mem_alloc(size_t size) {
    size = size_with_align_4B((uint32_t) size);
    // If need more than half of the page
    if (pages[0].page_size / 2 + 1 <= size) {
        // How many
        size_t how_many = size / pages[0].page_size;
        if (how_many * pages[0].page_size < size)
            how_many++;
        // Looking for free pages
        uint16_t current_length = 0;
        int start_idx = -1;
        for (uint16_t i = 0; i < pages.size(); i++) {
            if (current_length == 0 && pages[i].state == 0) {
                current_length++;
                start_idx = i;
            }
            else if (current_length != 0 && pages[i].state == 0) {
                current_length++;
                if (current_length == how_many)
                    break;
            }
            else
                current_length = 0;
        }
        if (current_length < how_many)
            return NULL;
        for (uint16_t i = 0; i < how_many; i++)
            pages[start_idx + i].state = 3;
        pages_blocks.push_back(pages_with_state_3_block((uint16_t) start_idx, (uint16_t) how_many));
        return &memory_block[offset((uint16_t) start_idx, 0)];
    }
    // Trying find already divided page
    for (uint16_t i = 0; i < pages.size(); i++)
        if (pages[i].state == 2 && pages[i].block_size / 2 < size && size <= pages[i].block_size &&
            pages[i].have_free_block())
            for (uint16_t j = 0; j < pages[i].blocks_count; j++)
                if (find(pages[i].in_use_blocks_info.begin(), pages[i].in_use_blocks_info.end(), j) ==
                    pages[i].in_use_blocks_info.end()) {
                    pages[i].in_use_blocks_info.push_back(j);
                    return &memory_block[offset(i, j)];
                }
    // Trying divide free page
    for (uint16_t i = 0; i < pages.size(); i++)
        if (pages[i].state == 0) {
            pages[i].split_to_blocks((uint16_t) size);
            pages[i].in_use_blocks_info.push_back(0);
            return &memory_block[offset(i, 0)];
        }
    // Again trying to get part of divided page (even if it divided to bigger parts)
    int min_size = -1;
    int min_page = -1;
    for (uint16_t i = 0; i < pages.size(); i++)
        if (pages[i].state == 2 && size <= pages[i].block_size && pages[i].have_free_block()) {
            min_size = pages[i].block_size;
            min_page = i;
            break;
        }
    if (min_size == -1)
        return NULL;
    // Looking for part which is nearer to our size
    for (uint16_t i = 0; i < pages.size(); i++)
        if (pages[i].state == 2 && size <= pages[i].block_size && pages[i].have_free_block() &&
            pages[i].block_size < min_size) {
            min_size = pages[i].block_size;
            min_page = i;
        }
    for (uint16_t j = 0; j < pages[min_page].blocks_count; j++)
        if (find(pages[min_page].in_use_blocks_info.begin(), pages[min_page].in_use_blocks_info.end(), j) ==
            pages[min_page].in_use_blocks_info.end()) {
            pages[min_page].in_use_blocks_info.push_back(j);
            return &memory_block[offset((uint16_t) min_page, j)];
        }
    return NULL;
}
void *Memory::mem_realloc(void *addr, size_t size) {
    size = size_with_align_4B((uint32_t) size);
    if (addr == NULL)
        return mem_alloc(size);
    mem_location idx = what_number_am_i(addr);
    // If old memory is one or more pages
    if (pages[idx.page].state == 3) {
        for (int i = 0; i < pages_blocks.size(); i++)
            if (pages_blocks[i].start_idx == idx.page) {
                if (pages_blocks[i].number_of_pages * pages[0].page_size < size) {
                    size_t old_size = pages_blocks[i].number_of_pages * pages[0].page_size;
                    mem_free(addr);
                    void *new_ref = mem_alloc(size);
                    if (new_ref == NULL)
                        return mem_alloc(old_size);
                }
                else return addr;
            }
    }
    // If old memory is part of some page
    if (pages[idx.page].state == 2) {
        if (pages[idx.page].block_size / 2 < size && size <= pages[idx.page].block_size)
            return addr;
        size_t old_size = pages[idx.page].block_size;
        mem_free(addr);
        void *new_ref = mem_alloc(size);
        if (new_ref == NULL)
            return mem_alloc(old_size);
    }
    return NULL;
}
void Memory::mem_free(void *addr) {
    mem_location idx = what_number_am_i(addr);
    if (idx.page == -1)
        return;
    // If delete block of pages
    if (pages[idx.page].state == 0)
        for (uint16_t i = 0; i < pages_blocks.size(); i++)
            if (pages_blocks[i].start_idx == idx.page) {
                for (uint16_t j = 0; j < pages_blocks[i].number_of_pages; j++)
                    pages[pages_blocks[i].start_idx + j].state = 0;
                pages_blocks.erase(pages_blocks.begin() + i);
                return;
            }
    // If delete part of page
    if (pages[idx.page].state == 2) {
        if (pages[idx.page].in_use_blocks_info.size() == 1 && pages[idx.page].in_use_blocks_info[0] == idx.block) {
            pages[idx.page].state = 0;
            pages[idx.page].in_use_blocks_info = vector<uint16_t>();
            return;
        }
        else if (
                find(pages[idx.page].in_use_blocks_info.begin(), pages[idx.page].in_use_blocks_info.end(), idx.block) !=
                pages[idx.page].in_use_blocks_info.end()) {
            pages[idx.page].in_use_blocks_info.erase(pages[idx.page].in_use_blocks_info.begin() + idx.block);
            return;
        }
    }
}
void Memory::mem_dump() {
    cout << "------------------------------------------------------\n";
    cout << "Pages states: " << endl;
    for (int16_t i = 0; i < pages.size(); i++)
        cout << (int) pages[i].state << " ";
    cout << endl;
    cout << endl;
    for (int16_t i = 0; i < pages.size(); i++)
        if (pages[i].state == 2) {
            cout << "Page #" << i << endl;
            cout << "Block size - " << pages[i].block_size << endl;
            cout << "Blocks count - " << pages[i].blocks_count << endl;
            for (int16_t j = 0; j < pages[i].blocks_count; j++) {
                if (find(pages[i].in_use_blocks_info.begin(), pages[i].in_use_blocks_info.end(), j) ==
                    pages[i].in_use_blocks_info.end())
                    cout << "_";
                else
                    cout << "*";
            }
            cout << endl << endl;
        }
    cout << "------------------------------------------------------\n\n";
}
Memory::mem_location Memory::what_number_am_i(void *addr) {
    if (addr == NULL)
        return mem_location(-1, -1);
    size_t page = ((long) addr - (long) &memory_block[0]) / pages[0].page_size / sizeof(Memory_unit);
    if (pages[page].block_size == 0)
        return mem_location((int) page, -1);
    size_t block = (((long) addr - (long) &memory_block[pages[0].page_size * page])) / pages[page].block_size /
                   sizeof(Memory_unit);
    return mem_location((int) page, (int) block);
}
uint32_t Memory::size_with_align_4B(uint32_t size) {
    if (size % 4 == 0)
        return size;
    return size - size % 4 + 4;
}
uint16_t Memory::offset(uint16_t page, uint16_t block) {
    return pages[page].page_size * page + pages[page].block_size * block;
}
void Memory::Page::split_to_blocks(uint16_t block_length) {
    uint16_t page_size_copy = page_size;
    // Page must be divided to equal parts
    while (2 <= page_size_copy) {
        if (page_size_copy < block_length) {
            page_size_copy *= 2;
            break;
        }
        page_size_copy /= 2;
    }
    this->block_size = page_size_copy;
    blocks_count = page_size / page_size_copy;
    in_use_blocks_info = vector<uint16_t>();
    state = 2;
}
