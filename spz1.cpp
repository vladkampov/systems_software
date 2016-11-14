#include <iostream>
#include <stdlib.h>
#include <vector>
#include <iomanip>

using namespace std;

#define byte char
#define mem_size 256
#define mem_seg 4
#define mem_chunks 256

struct MemBlock {
  void* seg_pointer;
  int seg_size;

  MemBlock(void* pointer, int size) {
    this -> seg_pointer = pointer;
    this -> seg_size = size;
  }
};

int mem_avaliable = mem_size;
vector<MemBlock> mem_vect;
void* memory = malloc(sizeof (byte) * mem_size);

void mem_init(bool random = false) {
  if (random) {
    for (int i = 0; i < mem_size; i++) {
        ((byte*) memory)[i] = rand() % 256;
    }
  } else {
    for (int i = 0; i < mem_size / 4; i++) {
        ((int*) memory)[i] = 0;
    }
  }
}


void mem_dump() {
  cout << "Total memory: " << mem_size << " Bytes" << endl;
  cout << "Avaliable: " << mem_avaliable << " Bytes" << endl;
  cout << endl;

  cout << "    |";

  for (int i = 0; i < 16; i++) {
    cout << " 0";
    cout << std::hex << std::uppercase << i << " ";
  }

  cout << endl << "----|";

  for (int i = 0; i < 16; i++) {
    cout << "----";
  }
  cout << endl;

  void* mem_ptr = memory;

  for (int i = 0; i < mem_chunks; i += 16) {
    cout << "\033[37m";
    cout << " " << std::hex << std::uppercase << i / 16;
    cout << "0 |";

    for (int j = 0; j < 16; j++) {
      cout << "\033[32m";

      int cur_ptr = *(int*) &mem_ptr + i + j;
      int cur_byte = (int) (byte) (((byte*) mem_ptr)[i + j]);

      for (MemBlock block : mem_vect) {
        int seg_ptr = *(int*) &block.seg_pointer;
        int seg_size = (int) block.seg_size;
        if (cur_ptr >= seg_ptr && cur_ptr < seg_ptr + seg_size) {
            cout << "\033[31m";
        }
      }

      cout << " " << nouppercase << setfill('0') << setw(2) << std::hex << cur_byte << " ";
    }

    cout << endl;
  }
  cout << endl << "\e[0m";
}

void* mem_alloc(size_t size) {
  cout << "Allocate " << size << " bytes." << endl;

  if (size > mem_avaliable || size < 1)
    return NULL;


  cout << size << endl;

  if (size % mem_seg != 0)
    size = ((size / mem_seg) + 1) * mem_seg;

  cout << size << endl;

  return 0;
}

int main(int argc, char **argv) {
  // Initialization
  mem_init();
  mem_dump();

  void* myInt1 = mem_alloc(5);

  mem_dump();
  // *(int*)myInt1 = 0x11223344;
  // mem_dump();

  return 0;
}