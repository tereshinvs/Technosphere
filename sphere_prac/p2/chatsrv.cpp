// OS: Ubuntu Linux 14.04 64-bit

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <ctime>

const unsigned PORT_NUMBER = 3100;
const unsigned MAX_EVENTS = 100;
const unsigned MAX_BUF_SIZE = 256;
const unsigned MAX_MSG_SIZE = 1024;
const unsigned MAX_MSG_BUF_SIZE = 65536;
const time_t TIME_OUT = 50;
const std::string WELCOME = "Welcome\n";
std::ostream &logger = std::cout;

struct Client {
	int fd;
	size_t eolpos = std::string::npos;
	std::string curmsg = "";

	Client(int _fd):
		fd(_fd) {}

	void append(const std::string &str) {
		curmsg += str;
		eolpos = curmsg.find_first_of('\n', eolpos + 1);
	}

	std::string get_current_msg() const {
		return eolpos == std::string::npos ? "" : curmsg.substr(0, eolpos + 1);
	}

	std::string flush_msg() {
		if (eolpos == std::string::npos)
			return "";
		std::string res = curmsg.substr(0, eolpos + 1);
		curmsg = curmsg.substr(eolpos + 1);
		eolpos = curmsg.find_first_of('\n');
		return res;
	}

	bool new_msg_is_ready() const {
		return eolpos != std::string::npos;
	}

	bool operator==(const Client &c) const {
		return fd == c.fd;
	}

	bool operator==(int _fd) const {
		return fd == _fd;
	}
};

static int make_socket_non_blocking(int sfd) {
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1)
		return -1;
	return 0;
}

void drop_client(int client_fd, std::vector<Client> &clients) {
	clients.erase(find(clients.begin(), clients.end(), client_fd));
	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
	logger << "connection terminated: fd = " << client_fd << std::endl;
}

int main() {
	char hbuf[MAX_BUF_SIZE], sbuf[MAX_BUF_SIZE];

	std::vector<Client> clients;
	int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int sockopt = 1;
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) != 0) {
		std::cerr << "setsockopt failed" << std::endl;
		return 0;
	}

	int epoll = epoll_create1(0);
	epoll_event event;
	event.data.fd = master_socket;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epoll, EPOLL_CTL_ADD, master_socket, &event);
	std::vector<epoll_event> events(MAX_EVENTS);

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_NUMBER);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(master_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr)) != 0) {
		std::cerr << "bind failed" << std::endl;
		return 0;
	}

	if (make_socket_non_blocking(master_socket) == -1) {
		std::cerr << "non blocking failed" << std::endl;
		close(master_socket);
		return 0;
	}
	listen(master_socket, SOMAXCONN);

	while (true) {
		int n = epoll_wait(epoll, events.data(), MAX_EVENTS, -1);
		for (int i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				int error = 0;
				socklen_t errlen = sizeof(error);
				if (getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) == 0)
    				std::cerr << "error: " << strerror(error) << std::endl;
				drop_client(events[i].data.fd, clients);
			} else if (events[i].data.fd == master_socket) {
				while (true) {
					sockaddr_in new_addr_in;
					socklen_t new_addr_len = sizeof(sockaddr_in);
					int slave_socket = accept(master_socket,
						reinterpret_cast<sockaddr*>(&new_addr_in), &new_addr_len);
					if (slave_socket != -1) {
						make_socket_non_blocking(slave_socket);
						event.data.fd = slave_socket;
						event.events = EPOLLIN | EPOLLET;
						epoll_ctl(epoll, EPOLL_CTL_ADD, slave_socket, &event);

						clients.push_back(Client(slave_socket));

						if (send(slave_socket, WELCOME.c_str(), WELCOME.size() + 1, 0) < WELCOME.size() + 1) {
							drop_client(slave_socket, clients);
							continue;
						}

						if (getnameinfo(reinterpret_cast<sockaddr*>(&new_addr_in), new_addr_len,
							hbuf, MAX_BUF_SIZE, sbuf,
							MAX_BUF_SIZE, NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
							logger << "accepted connection: fd = " << slave_socket
								<< ", addr = " << hbuf << ", port = " << sbuf << std::endl;
						}
					} else {
						break;
					}
				}
			} else {
				char buf[MAX_MSG_SIZE] = {0};
				int byte_readed
					= recv(events[i].data.fd, buf, MAX_MSG_SIZE - 1, 0);
				if (byte_readed == -1 && errno != EAGAIN) {
					drop_client(events[i].data.fd, clients);
					continue;
				}
				auto curclient = find(clients.begin(), clients.end(), Client(events[i].data.fd));
				curclient->append(buf);
				while (curclient->new_msg_is_ready()) {
					std::string msg = curclient->flush_msg();
					logger << "New message from fd = " << events[i].data.fd << ": " << msg << std::endl;
					for (size_t j = 0; j < msg.length(); j += MAX_MSG_SIZE) {
						std::string part = msg.substr(j, std::min(j + MAX_MSG_SIZE, msg.length()));
						const char *c_part = part.c_str();
						size_t part_len = strlen(c_part);
						for (auto it = clients.begin(); it != clients.end(); ++it)
							if (send(it->fd, c_part, part_len + 1, 0) <= 0)
								drop_client(it->fd, clients);
					}
				}
			}
		}
	}

	shutdown(master_socket, SHUT_RDWR);
	close(master_socket);
	return 0;
}
