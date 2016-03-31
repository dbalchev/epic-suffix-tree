#include "suffix-tree.hpp"
#include "async-reader.hpp"
#include <iostream>
#include <cstddef>

namespace special
{
	struct memory_pool
	{
		struct block
		{
			block *next_block;
			char memory[1];
		};
		block *first_block;
		block *cu_block;

		char *cu_ptr; /* of the current block */
		char *end_ptr;
		size_t block_size;

		void allocate_next_block()
		{
			allocate_next_block(&cu_block);
		}
		void allocate_next_block(block **p_rez)
		{
			*p_rez = (block*)MY_ALLOC(block_size);
			if (!*p_rez)
				THROW("cannot alloc a block");
			(*p_rez)->next_block = 0;
			cu_ptr = (*p_rez)->memory;
			end_ptr = (char*)(*p_rez) + block_size;
		}
		memory_pool()
		{
			init(4 << 20);
		}

		void init(size_t block_size)
		{
			this->block_size = block_size;
			allocate_next_block(&first_block);
			cu_block = first_block;
		}
		void free()
		{
			block* cu_block = first_block;
			first_block = cu_block = 0;
			while (cu_block) {
        block* prev_block = cu_block;
				cu_block = cu_block->next_block;
				MY_FREE(prev_block, block_size);
			}
		}

		template <typename T>
			T* alloc()
			{
				if (unlikely(cu_ptr + sizeof(T) > end_ptr)) {
					if (unlikely(sizeof(T) + offsetof(block, memory) > block_size))
						THROW("block size too small");
					allocate_next_block();
				}
				T* rez = (T*)cu_ptr;
				cu_ptr += sizeof(T);
				return rez;
			}
	};
};

int main(int argc, char *argv[])
{
  try {
		if (argc != 2) {
			THROW("need exatly 1 arg");
		}
		remapper<256> rmp(1, 6);
		rmp.map('\n', 0);
		int fd = open(argv[1], O_RDONLY);
		if (fd == -1)
			throw "open failed";
		typedef balchev::async_reader<5, balchev::memory_allocator> areader_t;
	 	areader_t areader(fd, 16 << 10);
    suffix_tree::suffix_tree<6, char, const char*, special::memory_pool> st;
		char* prev = (char*)areader.get_memory(), *cu;
		off_t file_offset;
		areader_t::touch_r rez;
		for (cu = prev, file_offset = 0; (rez = areader.touch(file_offset)) == areader_t::TOUCH_OK ; ++cu, ++file_offset) {
			*cu = rmp.remap(*cu);
			if (*cu == 0) {
				st.add_word(prev, cu + 1);
				prev = cu + 1;
			}
		}
#ifdef PRINT_TREE
    std::cout << "digraph ST {" << std::endl;
    st.print();
    std::cout << "}" << std::endl;
		std::cerr << st.get_num_nodes() << std::endl;
#else
		std::cout << st.get_num_nodes() << std::endl;
#endif
  } catch(const char *str) {
    std::cout << "exception: " << str << std::endl;
  }

  return 0;
}
