#include "Response.hpp"

#include <algorithm>

http_server::Response::Response():
	version(), status(0), reason(), headers(), body() {}

http_server::Response::Response(
	const std::string &_version,
	int _status,
	const std::string &_reason,
	const std::map<std::string, std::string> &_headers,
	const std::string &_body):
	version(_version),
	status(_status),
	reason(_reason),
	headers(_headers),
	body(_body) {}

http_server::Response &http_server::Response::set_version(const std::string &_version) {
	version = _version;
	return *this;
}

http_server::Response &http_server::Response::set_status(int _status) {
	status = _status;
	return *this;
}

http_server::Response &http_server::Response::set_reason(const std::string &_reason) {
	reason = _reason;
	return *this;
}

http_server::Response &http_server::Response::set_headers(
	const std::map<std::string, std::string> &_headers) {
	headers = _headers;
	return *this;
}

http_server::Response &http_server::Response::set_body(
	const std::string &_body) {
	body = _body;
	return *this;
}

http_server::Response &http_server::Response::add_header(
	const std::string &name, const std::string &value) {
	std::string tmp = name;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
	headers[tmp] = value;
	return *this;
}

http_server::Response &http_server::Response::add_body(const std::string &_body) {
	body += _body;
	return *this;
}

std::string http_server::Response::to_http() const {
	std::string res = version + " " + std::to_string(status) + " " + reason + "\n";
	for (const auto &p: headers)
		res += p.first + ": " + p.second + "\n";
	res += "\n" + body;
	return res;
}
