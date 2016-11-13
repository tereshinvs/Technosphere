#include "Client.hpp"

void Client::read() {
	async_read(sock, )
}

Client::Client(
	int _id,
	const boost::asio::ip::tcp::socket &_sock,
	std::size_t bufsize,
	std::function<void(const Client &)> &_callback):
	id(_id), sock(_sock), buf(bufsize) callback(_callback) {}

bool Client::operator<(const Client &right) const {
	return id < right.id;
}

int Client::get_id() const {
	return id;
}

ClientBuilder::ClientBuilder(std::size_t _bufsize,
	const std::function<void(const Client &)> &_callback):
	last_id(0), bufsize(_bufsize), callback(_callback) {}

Client ClientBuilder::build_client(boost::asio::ip::tcp::socket &sock) {
	return Client(last_id++, sock, bufsize, callback);
}
