#include "sock_ops.h"

#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <limits.h>

int get_master_socket_and_listen(int port) {
	int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int sockopt = 1;
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) != 0)
		return -1;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(master_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr)) != 0)
		return -1;

/*	if (make_socket_non_blocking(master_socket) == -1) {
		close(master_socket);
		return -1;
	}*/
	listen(master_socket, SOMAXCONN);
	return master_socket;
}

int get_slave_socket(int master_socket) {
	struct sockaddr_in new_addr_in;
	socklen_t new_addr_len = sizeof(sockaddr_in);
	int slave = accept(master_socket,
		(sockaddr*)(&new_addr_in), &new_addr_len);
	return slave;
}

int get_internal_server_socket(const char *sock_name) {
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;
	pid_t child;
 
	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
		return 1;

	unlink(sock_name);

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, PATH_MAX, "%s", sock_name);

	if (bind(socket_fd, 
		(struct sockaddr *) &address, 
		sizeof(struct sockaddr_un)) != 0)
		return 1;

	if (listen(socket_fd, 5) != 0)
		return 1;
	return socket_fd;
}

int connect_to_internal_socket(const char *sock_name) {
	struct sockaddr_un address;
 	int socket_fd;

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0) {
	  return -1;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));
 
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, PATH_MAX, "%s", sock_name);

	if (connect(socket_fd, 
		(struct sockaddr *) &address, 
            sizeof(struct sockaddr_un)) != 0) {
		return -1;
	}

	return socket_fd;
}

int accept_client(int socket) {
	struct sockaddr_un address;
	socklen_t address_length;
	return accept(socket, (struct sockaddr *) &address, &address_length);
}

int make_socket_non_blocking(int sfd) {
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

int close_socket(int socket) {
	return close(socket);
}

int send_fd(int socket, int fd_to_send) {
	struct msghdr socket_message;
	struct iovec io_vector[1];
	struct cmsghdr *control_message = NULL;
	char message_buffer[1];
	/* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
	char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
	int available_ancillary_element_buffer_space;

	/* at least one vector of one byte must be sent */
	message_buffer[0] = 'F';
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;

	/* initialize socket message */
	memset(&socket_message, 0, sizeof(struct msghdr));
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;

	/* provide space for the ancillary data */
	available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
	memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
	socket_message.msg_control = ancillary_element_buffer;
	socket_message.msg_controllen = available_ancillary_element_buffer_space;

	/* initialize a single ancillary data element for fd passing */
	control_message = CMSG_FIRSTHDR(&socket_message);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));
	*((int *) CMSG_DATA(control_message)) = fd_to_send;

	return sendmsg(socket, &socket_message, 0);
}

int recv_fd(int socket) {
	int sent_fd, available_ancillary_element_buffer_space;
	struct msghdr socket_message;
	struct iovec io_vector[1];
	struct cmsghdr *control_message = NULL;
	char message_buffer[1];
	char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

	/* start clean */
	memset(&socket_message, 0, sizeof(struct msghdr));
	memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

	/* setup a place to fill in message contents */
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;

	/* provide space for the ancillary data */
	socket_message.msg_control = ancillary_element_buffer;
	socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

	if(recvmsg(socket, &socket_message, MSG_CMSG_CLOEXEC) < 0)
		return -1;

	if(message_buffer[0] != 'F') {
		/* this did not originate from the above function */
		return -1;
	}

	if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC) {
		/* we did not provide enough space for the ancillary element array */
		return -1;
	}

	/* iterate ancillary elements */
	for(control_message = CMSG_FIRSTHDR(&socket_message);
		control_message != NULL;
		control_message = CMSG_NXTHDR(&socket_message, control_message)) {
		if((control_message->cmsg_level == SOL_SOCKET) &&
			(control_message->cmsg_type == SCM_RIGHTS)) {
			sent_fd = *((int *) CMSG_DATA(control_message));
			return sent_fd;
		}
	}

	return -1;
}
