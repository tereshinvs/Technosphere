#include "Request.hpp"

#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include <algorithm>

void http_server::Request::parse_params(const std::string &s,
	std::string &addr,
	std::map<std::string, std::string> &params,
	bool addr_exist) {
	size_t pst = 0;
	params.clear();
	if (addr_exist) {
		pst = s.find('?');
		addr = s.substr(0, pst);
		if (pst == std::string::npos)
			return;
		pst++;
	}
	for (int i = pst; i < s.size(); ) {
		pst = s.find('=', i);
		std::string name = s.substr(i, pst - i);
		i = pst + 1;
		pst = s.find('&', i);
		std::string value = s.substr(i, pst - i);
		params[name] = value;
		i = pst != std::string::npos ? pst + 1 : s.size();
	}
}

std::string http_server::Request::trim(const std::string &s) {
	if (s.empty())
		return "";
	auto c1 = s.begin();
	auto c2 = s.end();
	for (; c1 != s.end() && isspace(*c1); ++c1);
	for (; c2 != c1 && isspace(*(c2 - 1)); --c2);
	return std::string(c1, c2);
}

http_server::Request::Request(int _type,
	const std::string &_url,
	const std::map<std::string, std::string> &_url_params,
	const std::string &_version,
	const std::map<std::string, std::string> &_headers,
	const std::string &_body,
	const std::map<std::string, std::string> &_body_params):

	url(_url),
	url_params(_url_params),
	version(_version),
	headers(_headers),
	body(_body),
	body_params(_body_params) {
	if (_type != GET &&	_type != POST && _type != HEAD)
		type = ERROR;
	else
		type = _type;
}

http_server::Request::Request(const std::string &s):
	url(), url_params(), version(), headers(), body(), body_params() {
	std::istringstream str(s), startlinestr;
	int state = STATES::STARTLINE;
	std::string token;

	std::string full_url, tmp;

	while (!str.eof() && state != STATES::END &&
		state != STATES::ERROR && state != STATES::END) {
		switch (state) {
			case STATES::STARTLINE: {
				std::getline(str, tmp);
				startlinestr.str(tmp);
				startlinestr >> token;
				if (token == "GET")
					type = GET;
				else if (token == "POST")
					type = POST;
				else if (token == "HEAD")
					type = HEAD;
				else {
					type = ERROR;
					state = STATES::ERROR;
					break;
				}
				state = STATES::STARTLINE_INFO;
				break;
			}
			case STATES::STARTLINE_INFO: {
				startlinestr >> full_url >> version;
				if (version.substr(0, 5) != "HTTP/") {
					type = ERROR;
					state = STATES::ERROR;
				}
				state = STATES::HEADER;
				break;
			}
			case STATES::HEADER: {
				std::getline(str, tmp);
				if (std::all_of(tmp.begin(), tmp.end(), ::isspace)) {
					state = STATES::BODY;
					break;
				}
				std::string header_name, header_value;
				size_t pst = tmp.find(':');
				if (pst == std::string::npos) {
					type == ERROR;
					state = STATES::ERROR;
					break;
				}
				header_name = trim(tmp.substr(0, pst));
				header_value = trim(tmp.substr(pst + 1, std::string::npos));
				std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::tolower);

				headers[header_name] = header_value;
				break;
			}
			case STATES::BODY: {
				while (!str.eof()) {
					std::getline(str, tmp);
					body += tmp + '\n';
				}
				state = STATES::END;
				break;
			}
			case STATES::ERROR: {
				break;
			}
			default:
				break;
		}
	}

	if (type != ERROR) {
		parse_params(full_url, url, url_params);
		parse_params(body, tmp, body_params, false);
	}
}

int http_server::Request::get_type() const {
	return type;
}

const std::string &http_server::Request::get_url() const {
	return url;
}

const std::map<std::string, std::string> &http_server::Request::get_url_params() const {
	return url_params;
}

const std::string &http_server::Request::get_version() const {
	return version;
}

const std::map<std::string, std::string> &http_server::Request::get_headers() const {
	return headers;
}

const std::string &http_server::Request::get_body() const {
	return body;
}

const std::map<std::string, std::string> &http_server::Request::get_body_params() const {
	return body_params;
}

std::string http_server::Request::to_http() const {
	std::string res;
	switch (type) {
		case GET: {
			res += "GET";
			break;
		}
		case POST: {
			res += "POST";
			break;
		}
		case HEAD: {
			res += "HEAD";
			break;
		}
		default: {
			break;
		}
	}
	res += " " + url;
	if (!url_params.empty()) {
		res += "?";
		for (const auto &p: url_params)
			res += p.first + "=" + p.second + "&";
		res = res.substr(0, res.size() - 1);
	}
	res += " " + version + "\n";
	for (const auto &p: headers)
		res += p.first + ": " + p.second + "\n";
	res += "\n" + body;
	return res;	
}
