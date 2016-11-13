#include "HashTable.hpp"

#include <cstring>

#include <unistd.h>

HashTable::Entry::Entry(
	const char *_key, const char *_value, unsigned _TTL, char _status):
	key(_key), value(_value), TTL(_TTL), status(_status) {}

HashTable::Entry HashTable::get_entry(char *from) const {
	if (from < data || from >= data + data_size ||
		(from - data) % (key_size + value_size + sizeof(unsigned) + sizeof(char)) != 0)
		throw unsigned(INVALID_PLACE);
	return Entry(
		from,
		from + key_size,
		*reinterpret_cast<const unsigned *>(from + key_size + value_size),
		*reinterpret_cast<const char *>(from + key_size + value_size + sizeof(unsigned))
	);
}

HashTable::Entry HashTable::get_entry(std::size_t k) const {
	return get_entry(data + k * (key_size + value_size + sizeof(unsigned) + sizeof(char)));
}

HashTable::Entry HashTable::get_entry(const std::string &key) const {
	for (std::size_t i = 0; i < n_records; ++i) {
		std::size_t pos = hash(key, i);
		Entry entry = get_entry(pos);
		if (entry.status == ALIVE &&
			std::strncmp(entry.key, key.data(), key_size) == 0)
			return entry;
		if (entry.status == FREE)
			break;
	}
	throw unsigned(NO_SUCH_KEY);
}

void HashTable::set_entry(const Entry &src, Entry &dst) {
	char *to = const_cast<char*>(dst.key);
	if (src.key != nullptr) {
		std::size_t len = std::strlen(src.key);
		std::memcpy(to, src.key, len);
		std::memset(to + len, 0, key_size - len);
	}
	to += key_size;
	if (src.value != nullptr) {
		std::size_t len = std::strlen(src.value);
		std::memcpy(to, src.value, len);
		std::memset(to + len, 0, value_size - len);
	}
	to += value_size;
	std::memcpy(to, &src.TTL, sizeof(unsigned));
	to += sizeof(unsigned);
	std::memcpy(to, &src.status, sizeof(char));
}

HashTable::Entry HashTable::find_place(const std::string &key) const {
	for (std::size_t i = 0; i < n_records; ++i) {
		std::size_t pos = hash(key, i);
		Entry entry = get_entry(pos);
		if (entry.status == FREE ||
			entry.status == REMOVED ||
			entry.status == ALIVE &&
			std::strncmp(entry.key, key.data(), key_size) == 0)
			return entry;
	}
	throw unsigned(NO_FREE_SPACE);
}

void HashTable::execute() {
	while (true) {
		for (std::size_t i = 0; i < n_records; ++i) {
			semaphore.full_lock(i);
			Entry entry = get_entry(i);
			if (entry.status == ALIVE)
				set_entry(entry.TTL == 0 ?
					Entry(nullptr, nullptr, 0, REMOVED) :
					Entry(nullptr, nullptr, entry.TTL - 1, ALIVE),
					entry);
			semaphore.full_release(i);
		}
		sleep(1);
	}
}

HashTable::HashTable(
	char *_data,
	const Semaphore &_semaphore,
 	std::size_t _data_size,
	int _n_records,
	std::size_t _key_size,
	std::size_t _value_size,
	const std::function<std::size_t(const std::string&, std::size_t)> &_hash):
	data(_data),
	data_size(_data_size),
	n_records(_n_records),
	key_size(_key_size),
	value_size(_value_size),
	hash(_hash),
	semaphore(_semaphore),
	gc_pid(0) {
	if (data_size < 
		n_records * (key_size + value_size + sizeof(unsigned) + sizeof(char)))
		throw unsigned(NO_FREE_SPACE);
}

HashTable::HashTable(const HashTable &_hash_table):
	data(_hash_table.data),
	data_size(_hash_table.data_size),
	n_records(_hash_table.n_records),
	key_size(_hash_table.key_size),
	value_size(_hash_table.value_size),
	hash(_hash_table.hash),
	semaphore(_hash_table.semaphore),
	gc_pid(_hash_table.gc_pid) {}

HashTable &HashTable::operator=(const HashTable &_hash_table) {
	data = _hash_table.data;
	data_size = _hash_table.data_size;
	n_records = _hash_table.n_records;
	key_size = _hash_table.key_size;
	value_size = _hash_table.value_size;
	hash = _hash_table.hash;
	semaphore = _hash_table.semaphore;
	gc_pid = _hash_table.gc_pid;
	return *this;
}

int HashTable::run_garbage_collector() {
	if ((gc_pid = fork()) == 0) {
		execute();
		exit(0);
	}
	return gc_pid;
}

std::string HashTable::get(const std::string &key) const {
//	if (key.size() != key_size)
//		throw unsigned(INVALID_KEY);
//	return std::string(get_entry(key).value, value_size);
	if (key.size() + 1 > key_size)
		throw unsigned(INVALID_KEY);
	for (std::size_t i = 0; i < n_records; ++i) {
		std::size_t pos = hash(key, i);
		semaphore.lock(i);
		Entry entry = get_entry(pos);
		if (entry.status == ALIVE &&
			std::strncmp(entry.key, key.data(), key_size) == 0) {
			std::string res = std::string(entry.value, value_size);
			semaphore.release(i);
			return res;
		}
		semaphore.release(i);
		if (entry.status == FREE)
			break;
	}
	throw unsigned(NO_SUCH_KEY);
}

void HashTable::set(const std::string &key,
	const std::string &value,
	const unsigned TTL) {
/*	if (key.size() != key_size)
		throw unsigned(INVALID_KEY);
	if (value.size() != value_size)
		throw unsigned(INVALID_VALUE);
	Entry dst = find_place(key);
	Entry src(key.data(), value.data(), TTL, ALIVE);
	set_entry(src, dst);*/
	if (key.size() + 1 > key_size)
		throw unsigned(INVALID_KEY);
	if (value.size() + 1 > value_size)
		throw unsigned(INVALID_VALUE);
	for (std::size_t i = 0; i < n_records; ++i) {
		std::size_t pos = hash(key, i);
		semaphore.full_lock(i);
		Entry entry = get_entry(pos);
		if (entry.status == FREE ||
			entry.status == REMOVED ||
			entry.status == ALIVE &&
			std::strncmp(entry.key, key.data(), key_size) == 0) {
			Entry src(key.data(), value.data(), TTL, ALIVE);
			set_entry(src, entry);
			semaphore.full_release(i);
			return;
		}
		semaphore.full_release(i);
	}
	throw unsigned(NO_FREE_SPACE);
}
