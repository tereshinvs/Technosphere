#ifndef CACHE_SOCK_OPS_H
#define CACHE_SOCK_OPS_H

int get_master_socket_and_listen(int port);
int get_slave_socket(int master_socket);

int get_internal_server_socket(const char *sock_name);
int connect_to_internal_socket(const char *sock_name);

int accept_client(int socket);

static int make_socket_non_blocking(int sfd);
int close_socket(int socket);

int send_fd(int socket, int fd_to_send);
int recv_fd(int socket);

#endif
