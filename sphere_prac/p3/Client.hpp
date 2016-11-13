#ifndef PROXY_CLIENT_HPP
#define PROXY_CLIENT_HPP

#include <boost/asio.hpp>

#include <array>

class Client {
	private:
		int id;
		boost::asio::ip::tcp::socket sock;
		std::array<char> buf;
		std::function<void(const Client &)> &callback;

		void read();
		

	public:
		Client(int _id,
			const boost::asio::ip::tcp::socket &_sock,
			std::size_t bufsize,
			std::function<void(const Client &)> &_callback);

		bool operator<(const Client &right) const;

		int get_id() const;
};

class ClientBuilder {
	private:
		int last_id;
		std::size_t bufsize;
		std::function<void(const Client &)> callback;

	public:
		ClientBuilder(std::size_t _bufsize,
			const std::function<void(const Client &)> &_callback);

		Client build_client(boost::asio::ip::tcp::socket &sock);
};

#endif
