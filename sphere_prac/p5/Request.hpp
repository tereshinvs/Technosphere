#ifndef CACHE_REQUEST_HPP
#define CACHE_REQUEST_HPP

#include <string>
#include <memory>

#include "HashTable.hpp"

class Request {
	private:
		struct RequestBase {
			virtual ~RequestBase();
			virtual std::string perform(HashTable &hash_table) const = 0;
		};

		struct GetRequest: public RequestBase {
			std::string key;
			GetRequest(const std::string &_key);
			virtual std::string perform(HashTable &hash_table) const;
		};

		struct SetRequest: public RequestBase {
			std::string key, value;
			unsigned TTL;
			SetRequest(const std::string &_key,
				const std::string &_value,
				unsigned _TTL);
			virtual std::string perform(HashTable &hash_table) const;
		};

		std::unique_ptr<RequestBase> request;
		bool parse_error = false;

	public:
		enum {
			INCORRECT_REQUEST
		};

		Request();
		Request(const std::string &s);

		std::string perform(HashTable &hash_table) const;
};

#endif