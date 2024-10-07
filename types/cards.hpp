#ifndef KIERKI_TYPES_CARDS_HPP
#define KIERKI_TYPES_CARDS_HPP 1

#include "common.hpp"
#include "card.hpp"

#include <algorithm>
#include <unordered_set>

namespace kierki {

struct Cards : public std::vector<Card> {
	using std::vector<Card>::vector;

	operator std::string() const {
		return concat(cbegin(), cend());
	}

	static Cards rparse(std::istream &stream) {
		Cards cards;
		for (int i = 0; i < 3; i++) {
			auto position = stream.tellg();
			try {
				cards.push_back(Card::rparse(stream));
				position = stream.tellg();
			} catch (const ParserError &) {
				stream.seekg(position);
				break;
			}
		}
		std::reverse(cards.begin(), cards.end());
		return cards;
	}

};

}

#endif
