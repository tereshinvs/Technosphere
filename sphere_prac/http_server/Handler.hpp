#ifndef HTTP_SERVER_HANDLER_HPP
#define HTTP_SERVER_HANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"

#include <string>

namespace http_server {
	class Handler {
		private:
			std::string root;

			http_server::Response get(const http_server::Request &request) const;
			http_server::Response post(const http_server::Request &request) const;
			http_server::Response head(const http_server::Request &request) const;

		public:
			Handler(const std::string &_root);

			http_server::Response handle(const http_server::Request &request) const;
	};
};

#endif
