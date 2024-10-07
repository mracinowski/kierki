#ifndef KIERKI_TYPES_TRICK_HPP
#define KIERKI_TYPES_TRICK_HPP 1

#include "common.hpp"
#include "card.hpp"

#include <array>

namespace kierki {

struct Trick : public std::array<Card, 4> {

	operator std::string() const {
		return concat(cbegin(), cend());
	}

	static Trick rparse(std::istream &stream) {
		Trick result{
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream)
		};
		std::reverse(result.begin(), result.end());
		return result;
	}

};

}

#endif
