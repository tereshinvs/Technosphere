#ifndef CACHE_COWORKER_HPP
#define CACHE_COWORKER_HPP

#include "HashTable.hpp"

#include <memory>

class Coworker {
	private:
		std::shared_ptr<HashTable> &hash_table;
		int id;
		std::string sock_name;
		pid_t pid;
		int socket, socket_fd;

		void execute() const;

	public:
		Coworker(std::shared_ptr<HashTable> &_hash_table, int _id);

		//~Coworker();

		int run();
		void stop();

		int send_fd(int fd) const;
};

#endif
