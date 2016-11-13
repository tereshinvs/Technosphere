#ifndef CACHE_HASHTABLE_HPP
#define CACHE_HASHTABLE_HPP

#include "Semaphore.hpp"

#include <string>
#include <functional>

class HashTable {
	private:
		mutable char *data;
		std::size_t data_size;
		int n_records;
		std::size_t key_size;
		std::size_t value_size;
		std::function<std::size_t(const std::string&, std::size_t)> hash;
		Semaphore semaphore;
		int gc_pid;

		enum {
			FREE, ALIVE, REMOVED
		};

		struct Entry {
			const char *key;
			const char *value;
			unsigned TTL;
			char status;
			Entry(const char *_key, const char *_value, unsigned _TTL, char _status);
		};

		Entry get_entry(char *from) const;
		Entry get_entry(std::size_t k) const;
		Entry get_entry(const std::string &key) const;

		void set_entry(const Entry &src, Entry &dst);
		//void set_TTL(std::size_t num, unsigned TTL);
		//void set_status(std::size_t, char status);

		Entry find_place(const std::string &key) const;

		void execute();

	public:
		enum {
			OK, NO_SUCH_KEY, INVALID_KEY, INVALID_VALUE, INVALID_PLACE, NO_FREE_SPACE
		};

		HashTable(char *_data, const Semaphore &_semaphore, std::size_t _data_size,
			int _n_records, std::size_t _key_size, std::size_t _value_size,
			const std::function<std::size_t(const std::string&, std::size_t)> &_hash);

		HashTable(const HashTable &_hash_table);

		HashTable &operator=(const HashTable &_hash_table);

		int run_garbage_collector();

		std::string get(const std::string &key) const;
		void set(const std::string &key,
			const std::string &value,
			const unsigned TTL);
};

#endif
