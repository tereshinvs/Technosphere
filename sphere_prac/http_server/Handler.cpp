#include "Handler.hpp"

//#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>

http_server::Handler::Handler(const std::string &_root):
	root(_root) {}

http_server::Response http_server::Handler::handle(
	const http_server::Request &request) const {
	switch (request.get_type()) {
		case Request::GET: {
			return get(request);
			break;
		}
		case Request::POST: {
			return post(request);
			break;
		}
		case Request::HEAD: {
			return head(request);
			break;
		}
		default: {
			break;
		}
	}
}

http_server::Response http_server::Handler::get(
	const http_server::Request &request) const {
	http_server::Response response;
	response.set_version("HTTP/1.1");

	std::ifstream ifs;
	//std::cout << "[" << root + request.get_url() << "]" << std::endl;
	ifs.open(root + request.get_url(), std::ifstream::in);
	if (!ifs.is_open()) {
		response.set_status(404).set_reason("File error").set_body("File error");
		return response;
	}
	response.set_status(200).set_reason("OK");

	std::stringstream buf;
	buf << ifs.rdbuf();
	response.set_body(buf.str());
	return response;
}

http_server::Response http_server::Handler::post(
	const http_server::Request &request) const {
	http_server::Response response;
	response.set_version("HTTP/1.1");
	std::ofstream ofs;
	ofs.open(root + "/data.html", std::ofstream::out | std::ofstream::app);
	if (!ofs.is_open()) {
		response.set_status(404).set_reason("File error").set_body("Post failed");
		return response;
	}
	const std::map<std::string, std::string> &body_params = request.get_body_params();
//	ofs << request.get_body();
//	for (const auto &p: body_params)
//		ofs << p.first << " " << p.second << std::endl;
	ofs << body_params.at("name") << " " << body_params.at("phone") << std::endl;
	ofs.close();
	response.set_status(200).set_reason("OK")
		.set_body("Post finished. Try to load data.html\n" + request.get_body());
	return response;
}

http_server::Response http_server::Handler::head(
	const http_server::Request &request) const {
	http_server::Response response;
	response.set_version("HTTP/1.1");

	std::ifstream ifs;
	ifs.open(root + request.get_url(), std::ifstream::in);
	if (!ifs.is_open()) {
		response.set_status(404).set_reason("File error");
		return response;
	}
	response.set_status(200).set_reason("OK");
	return response;
}
