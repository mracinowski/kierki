#ifndef KIERKI_TYPES_COMMON_HPP
#define KIERKI_TYPES_COMMON_HPP 1

#include <istream>

namespace kierki {
	class ParserError : public std::exception {
		const char *message;

	public:
		ParserError() : message("Parser error") {}
		ParserError(const char *message) : message(message) {}

		const char* what() const noexcept override {
			return message;
		}
		
	};

	std::string concat(const auto &begin, const auto &end) {
		std::string result;

		for (auto i = begin; i != end; i++)
			result.append(std::string(*i));

		return result;
	}
};

#endif
