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
const unsigned MAX_MSG_SIZE = 1024;

int main() {
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_NUMBER);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
		std::cerr << "connection failed" << std::endl;
		return 0;
	}

	while (true) {
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		FD_SET(0, &fdset);
		int maxfd = std::max(sock, 0);

		select(maxfd + 1, &fdset, NULL, NULL, NULL);

		if (FD_ISSET(0, &fdset)) {
			std::string msg;
			std::cin >> msg;
			msg += '\n';
			if (send(sock, msg.c_str(), msg.size() + 1, 0) < msg.size() + 1) {
				std::cerr << "send failed" << std::endl;
				break;
			}
		}

		if (FD_ISSET(sock, &fdset)) {
			char buf[MAX_MSG_SIZE] = {0};
			int byte_readed
				= recv(sock, buf, MAX_MSG_SIZE - 1, 0);
			if (byte_readed > 0) {
				std::cout << buf;
				std::cout.flush();
			}
			if (byte_readed < 0)
				break;
		}
	}

	shutdown(sock, SHUT_RDWR);
	close(sock);
	return 0;
}