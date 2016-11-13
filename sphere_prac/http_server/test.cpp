#include "Request.hpp"
#include "Response.hpp"
#include "Handler.hpp"

#include <iostream>
#include <string>
#include <map>

void test_parser() {
	std::string s = std::string("GET /path/resource?param1=value1&param2=value2 HTTP/1.1\n") +
		std::string("Host: ru.wikipedia.org\n") +
		std::string("Info: Dasha\n") +
		std::string("\n") +
		std::string("I love Dasha\n") +
		std::string("Very much.\n");
	std::cout << s << std::endl;

	http_server::Request result(s);

	int type = result.get_type();
	const std::string &url = result.get_url();
	const std::map<std::string, std::string> &url_params = result.get_url_params();
	const std::string &version = result.get_version();
	const std::map<std::string, std::string> &headers = result.get_headers();
	const std::string &body = result.get_body();

	std::cout << "Type:" << std::endl;
	std::cout << type << std::endl;
	std::cout << url << std::endl;
	std::cout << "url_params:" << std::endl;
	for (const auto &p: url_params)
		std::cout << p.first << " = " << p.second << std::endl;
	std::cout << version << std::endl;
	std::cout << "headers:" << std::endl;
	for (const auto &p: headers)
		std::cout << p.first << " = " << p.second << std::endl;
	std::cout << "body:" << std::endl;
	std::cout << body << std::endl;
	std::cout << "end body" << std::endl;	
}

void test_handler() {
	std::string s = std::string("GET /data/index.html?param1=value1&param2=value2 HTTP/1.1\n") +
		std::string("Host: ru.wikipedia.org\n") +
		std::string("Info: Dasha\n") +
		std::string("\n") +
		std::string("I love Dasha\n") +
		std::string("Very much.\n");
	std::cout << s << std::endl;

	http_server::Request result(s);
	http_server::Handler handler(".");
	http_server::Response response = handler.handle(result);
	std::cout << response.to_http() << std::endl;
}
