#ifndef CACHE_SEMAPHORE_HPP
#define CACHE_SEMAPHORE_HPP

#include <string>

#include <sys/types.h>
#include <sys/sem.h>

class Semaphore {
	private:
		std::size_t n;
		short max_value;
		key_t semkey;
		int semid;

		int update(std::size_t start, std::size_t m, short val, short flag) const;

	public:
		Semaphore(std::size_t _n, unsigned short _max_value,
			const std::string &file_name, int proj_id);
		~Semaphore();

/*		int lock(unsigned short start, unsigned short m, short val) const;
		int lock_nowait(unsigned short start, unsigned short m, short val) const;
		int release(unsigned short start, unsigned short m, short val) const;*/
		int lock(std::size_t start, std::size_t m = 1) const;
		int full_lock(std::size_t start, std::size_t m = 1) const;
		int lock_nowait(std::size_t start, std::size_t m = 1) const;
		int full_release(std::size_t start, std::size_t m = 1) const;
		int release(std::size_t start, std::size_t m = 1) const;
};

#endif
