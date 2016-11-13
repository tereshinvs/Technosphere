#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

const unsigned PORT_NUMBER = 3100;
const unsigned MAX_EVENTS = 100;
const unsigned MAX_BUF_SIZE = 256;
const std::string WELCOME = "Welcome\n";

int main() {
	char hbuf[MAX_BUF_SIZE], sbuf[MAX_BUF_SIZE];

	std::vector<int> clients;
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
	listen(master_socket, SOMAXCONN);

	while (true) {
		int n = epoll_wait(epoll, events.data(), MAX_EVENTS, -1);
		for (int i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				clients.erase(find(clients.begin(), clients.end(), events[i].data.fd));
				shutdown(events[i].data.fd, SHUT_RDWR);
				close(events[i].data.fd);
			} else if (events[i].data.fd == master_socket) {
				while (true) {
					sockaddr_in new_addr_in;
					socklen_t new_addr_len;
					int slave_socket = accept(master_socket,
						reinterpret_cast<sockaddr*>(&new_addr_in), &new_addr_len);
					if (slave_socket != -1) {
						event.data.fd = slave_socket;
						event.events = EPOLLIN | EPOLLET;
						epoll_ctl(epoll, EPOLL_CTL_ADD, slave_socket, &event);

						clients.push_back(slave_socket);

						send(slave_socket, WELCOME.c_str(), WELCOME.size() + 1, 0);

						if (getnameinfo(reinterpret_cast<sockaddr*>(&new_addr_in), new_addr_len,
							hbuf, MAX_BUF_SIZE, sbuf,
							MAX_BUF_SIZE, NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
							std::cout << "New connection: fd = " << slave_socket
								<< ", addr = " << hbuf << ", port = " << sbuf << std::endl;
						}
					} else {
						break;
					}
				}
			} else {

			}
		}
	}

	close(master_socket);
}
