#ifndef HTTP_SERVER_RESPONSE_HPP
#define HTTP_SERVER_RESPONSE_HPP

#include <string>
#include <map>

namespace http_server {
	class Response {
		private:
			std::string version;
			int status;
			std::string reason;
			std::map<std::string, std::string> headers;
			std::string body;

		public:
			Response();
			Response(const std::string &_version,
				int _status,
				const std::string &_reason,
				const std::map<std::string, std::string> &_headers,
				const std::string &_body);

			Response &set_version(const std::string &_version);
			Response &set_status(int _status);
			Response &set_reason(const std::string &reason);
			Response &set_headers(const std::map<std::string, std::string> &_headers);
			Response &set_body(const std::string &_body);

			Response &add_header(const std::string &name, const std::string &value);
			Response &add_body(const std::string &_body);

			std::string to_http() const;
	};
};

#endif
