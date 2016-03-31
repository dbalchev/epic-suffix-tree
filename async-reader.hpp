extern "C" {
#include <aio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
}
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "optimize.h"

namespace balchev
{
	template <int NBuffersBits, typename MemoryAllocator>
	struct async_reader : MemoryAllocator
	{
		const static int NBuffers = 1 << NBuffersBits;
		const static int BUFFER_MASK = NBuffers - 1;
		typedef MemoryAllocator base;
		enum touch_r
		{
			TOUCH_OK = 1,
			TOUCH_EOF = 0,
			TOUCH_ERROR = -1
		};

		int fd;
		off_t file_size;
#ifdef ASYNC
		struct aiocb op_info[NBuffers];
#endif
		off_t next_to_load;
		size_t buffer_len;
		int next_info;
		char *memory;

		async_reader(int fd, int buffer_len = 4 << 10)
		{
			this->fd = fd;
			this->buffer_len = buffer_len;
			struct stat stat_buf;
		        if (fstat(fd, &stat_buf))
				THROW("cannot stat");
			file_size = stat_buf.st_size;
			memory = (char*)base::allocate(file_size);
#ifdef ASYNC
			for (int i = 0; i < NBuffers; ++i)
				start_io(i, i * buffer_len);
#endif
			next_to_load = 0;
			next_info = 0;
		}
		void *get_memory()
		{
			return memory;
		}
		touch_r touch(off_t offset)
		{
	 		if (unlikely(offset >= file_size))
				return TOUCH_EOF;
			while (unlikely(offset >= next_to_load)) {
#ifdef ASYNC
				struct aiocb *aio_ptr = op_info + next_info;
				if (unlikely(aio_suspend(&aio_ptr, 1, 0))) {
					THROW("suspend failed");
				}
				start_io(next_info, next_to_load + NBuffers * buffer_len);
				next_info = (next_info + 1) & BUFFER_MASK;
#else
                                int readed = read(fd, memory + next_to_load, buffer_len);
				if (readed != (int)buffer_len && next_to_load + readed != (int)file_size)
					THROW("read error");
#endif
				next_to_load += buffer_len;
			}
			return TOUCH_OK;
		}
#ifdef ASYNC
		void start_io(int info_index, off_t where)
		{ 
			memset(op_info + info_index, 0, sizeof op_info[0]);
			op_info[info_index].aio_fildes   = fd;
			op_info[info_index].aio_offset   = where;
			op_info[info_index].aio_buf      = memory + where;
			op_info[info_index].aio_nbytes   = buffer_len;
			op_info[info_index].aio_sigevent.sigev_notify = SIGEV_NONE;
			if (unlikely(aio_read(op_info + info_index))) {
				THROW("aio_read failed");
			}
		}	
#endif
		~async_reader()
		{
			if (memory)
				base::release(memory);
		}
	};
	struct memory_allocator
	{
		size_t sz;
		void *alloced;
		memory_allocator()
		{
			alloced = 0;
		}
		void* allocate(size_t sz)
		{
			if (alloced)
				THROW("already alloced");
			this->sz = sz;
			return alloced = MY_ALLOC(sz);
		}
		void release(void* mem)
		{
			if (alloced == mem) {
				MY_FREE(alloced, sz);
				return;
			}
			THROW("cannot free this");
		}
	};
}

#if 0

int main(int argc, char *argv[])
{
	try {
		if (argc != 3) {
			throw "need 2 params";
		}
		int fd = open(argv[1], O_RDONLY);
		if (fd == -1)
			throw "open failed";
		typedef balchev::async_reader<5, memory_allocator> asr_t;
	 	asr_t ar(fd, 16 << 10);
		int lines = 0;
		for (int i = 0; ar.touch(i) == asr_t::TOUCH_OK ; ++i) {
			lines += ((char*)ar.get_memory())[i] == argv[2][0];
		}
		std::cout << "lines = " << lines << std::endl;
	}catch (char *msg) {
		perror(msg);
	}
}
#endif 
