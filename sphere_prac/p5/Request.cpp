#include "Request.hpp"

#include <sstream>
#include <ios>

Request::RequestBase::~RequestBase() {}

Request::GetRequest::GetRequest(const std::string &_key):
	key(_key) {}

std::string Request::GetRequest::perform(HashTable &hash_table) const {
	return "ok " + key + " " + hash_table.get(key);
}

Request::SetRequest::SetRequest(const std::string &_key,
	const std::string &_value,
	unsigned _TTL):
	key(_key), value(_value), TTL(_TTL) {}

std::string Request::SetRequest::perform(HashTable &hash_table) const {
	hash_table.set(key, value, TTL);
	return "ok " + key + " " + value;
}

Request::Request():
	request() {}

Request::Request(const std::string &s):
	request() {
	std::istringstream stream(s);
	std::string type, key;
	stream >> type;
	if (type == "get") {
		stream >> key;
		request.reset(new GetRequest(key));
	} else if (type == "set") {
		unsigned TTL;
		std::string value;
		stream >> TTL >> key >> std::ws;
		std::getline(stream, value);
		value = value.substr(0, value.size() - 1);
		request.reset(new SetRequest(key, value, TTL));
	} else
		parse_error = true;
	if (stream.fail())
		parse_error = true;
}

std::string Request::perform(HashTable &hash_table) const {
	if (parse_error)
		return "error parse_error";
	try {
		return request->perform(hash_table);
	} catch (unsigned code) {
		return "error " + std::to_string(code);
	}
}
