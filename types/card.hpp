#ifndef KIERKI_TYPES_CARD_HPP
#define KIERKI_TYPES_CARD_HPP 1

#include "common.hpp"
#include "rank.hpp"
#include "suit.hpp"

#include <compare>
#include <type_traits>

namespace kierki {

class Card {
	Rank _rank;
	Suit _suit;

public:
	static const auto COUNT = Rank::COUNT * Suit::COUNT;

	constexpr Card(const Rank &rank, const Suit &suit)
	: _rank(rank)
	, _suit(suit) {}

	const Rank &rank() const { return _rank; }
	const Suit &suit() const { return _suit; }

	std::strong_ordering operator<=>(const Card &other) const = default;

	operator std::string() const {
		std::string string(rank());
		string.append(std::string(suit()));
		return string;
	}
		
	static Card rparse(std::istream &stream) {
		Suit suit = Suit::rparse(stream);
		Rank rank = Rank::rparse(stream);
		return Card(rank, suit);
	}
};

}

namespace std {

	template <>
	struct hash<kierki::Card> {
		std::size_t operator()(const kierki::Card &card) const {
			auto rank = std::hash<kierki::Rank>()(card.rank());
			auto suit = std::hash<kierki::Rank>()(card.rank());
			return rank ^ (suit << 1);
	        }
	};

}

#endif
