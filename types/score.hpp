#ifndef KIERKI_TYPES_SCORE_HPP
#define KIERKI_TYPES_SCORE_HPP 1

#include "common.hpp"

#include <unordered_set>

namespace kierki {

struct Score {
	unsigned value;

	Score(): value(0) {}
	Score(decltype(value) value): value(value) {}

	Score &operator+=(const Score &other) {
		value += other.value;
		return *this;
	}

	Score operator+(const Score &other) {
		Score result(*this);
		result += other;
		return result;
	}

	operator std::string() const {
		return std::to_string(value);
	}

	static Score rparse(std::istream &stream) {
		std::string string;

		while ('0' <= stream.peek() && stream.peek() <= '9')
			string.push_back(stream.get());

		if (string.empty()) throw ParserError();
		if (string == "0") return Score(0);

		std::reverse(string.begin(), string.end());

		if (string.front() == '0') throw ParserError("Leading zero");

		return Score(std::stoul(string));
	}

};

}

#endif
