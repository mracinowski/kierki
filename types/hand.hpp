#ifndef KIERKI_TYPES_HAND_HPP
#define KIERKI_TYPES_HAND_HPP 1

#include "common.hpp"
#include "card.hpp"

#include <numeric>
#include <set>

namespace kierki {

struct Hand : public std::multiset<Card> {
	using std::multiset<Card>::multiset;

	operator std::string() const {
		return concat(cbegin(), cend());
	}

	static Hand rparse(std::istream &stream) {
		return {
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream),
			Card::rparse(stream)
		};
	}

};

}

#endif
