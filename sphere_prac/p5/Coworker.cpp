#include "Coworker.hpp"

#include "Request.hpp"
#include "sock_ops.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

const std::size_t MAX_BUF_SIZE = 65536;

void Coworker::execute() const {
	int fd;
	char buf[MAX_BUF_SIZE] = {0};
	while ((fd = recv_fd(socket_fd)) != -1) {
		recv(fd, buf, MAX_BUF_SIZE, 0);
		std::string strreq(buf);

		std::string response = Request(strreq).perform(*hash_table) + "\n";
		send(fd, response.data(), response.size() + 1, 0);
		close(fd);
	}
	close(socket_fd);
	close(socket);
	hash_table.reset();
}

Coworker::Coworker(std::shared_ptr<HashTable> &_hash_table, int _id):
	hash_table(_hash_table),
	id(_id),
	sock_name("./coworker_socket_" + std::to_string(id) + ".sock"),
	pid(0),
	socket(get_internal_server_socket(sock_name.data())),
	socket_fd(-1) {}
/*
Coworker::~Coworker() {
	close(socket_fd);
	close(socket);
	waitpid(pid, NULL, 0);
}
*/
int Coworker::run() {
	if ((pid = fork()) == 0) {
		socket_fd = connect_to_internal_socket(sock_name.data());
		execute();
		exit(0);
	}
	socket_fd = accept_client(socket);
	return pid;
}

void Coworker::stop() {
	close(socket_fd);
	close(socket);
	waitpid(pid, NULL, 0);
	hash_table.reset();
}

int Coworker::send_fd(int fd) const {
	return ::send_fd(socket_fd, fd);
}
