#ifndef HTTP_SERVER_REQUEST_HPP
#define HTTP_SERVER_REQUEST_HPP

#include <string>
#include <map>

namespace http_server {
	class Request {
		private:
			int type = ERROR;		
			std::string url;
			std::map<std::string, std::string> url_params;
			std::string version;
			std::map<std::string, std::string> headers;
			std::string body;
			std::map<std::string, std::string> body_params;

			struct STATES {
				enum {
					STARTLINE, STARTLINE_INFO, HEADER, HEADER_VALUE, BODY, ERROR, END
				};
			};

			void parse_params(const std::string &s,
				std::string &addr,
				std::map<std::string, std::string> &params,
				bool addr_exist = true);
			//void parse_url(const std::string &url);
			std::string trim(const std::string &s);

		public:
			enum {
				GET, POST, HEAD, ERROR
			};

			Request(int _type,
				const std::string &_url,
				const std::map<std::string, std::string> &_url_params,
				const std::string &_version,
				const std::map<std::string, std::string> &_headers,
				const std::string &_body,
				const std::map<std::string, std::string> &_body_params);

			Request(const std::string &s);

			int get_type() const;
			const std::string &get_url() const;
			const std::map<std::string, std::string> &get_url_params() const;
			const std::string &get_version() const;
			const std::map<std::string, std::string> &get_headers() const;
			const std::string &get_body() const;
			const std::map<std::string, std::string> &get_body_params() const;

			std::string to_http() const;
	};
};

#endif
