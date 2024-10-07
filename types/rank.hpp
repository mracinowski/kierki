#ifndef KIERKI_TYPES_RANK_HPP
#define KIERKI_TYPES_RANK_HPP 1

#include "common.hpp"

#include <compare>
#include <type_traits>

namespace kierki {

class Rank {
	enum class Value {
		TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
		JACK, QUEEN, KING, ACE,
	} value;
	using Underlying = std::underlying_type_t<Value>;

	constexpr Rank(Underlying value) : value(Value(value)) {}

public:
 	using Value::TWO, Value::THREE, Value::FOUR, Value::FIVE, Value::SIX,
		Value::SEVEN, Value::EIGHT, Value::NINE, Value::TEN,
		Value::JACK, Value::QUEEN, Value::KING, Value::ACE;
	static const Underlying COUNT = 13;

	constexpr Rank(Value value): value(value) {}

	std::strong_ordering operator<=>(const Rank &) const = default;

	operator char const *() const {
		switch (value) {
			case TWO:   return "2";
			case THREE: return "3";
			case FOUR:  return "4";
			case FIVE:  return "5";
			case SIX:   return "6";
			case SEVEN: return "7";
			case EIGHT: return "8";
			case NINE:  return "9";
			case TEN:   return "10";
			case JACK:  return "J";
			case QUEEN: return "Q";
			case KING:  return "K";
			case ACE:   return "A";
			default:    throw std::runtime_error("Invalid rank");
		}
	}
		
	static Rank rparse(std::istream &stream) {
		switch (stream.get()) {
			case '2': return Rank::TWO;
			case '3': return Rank::THREE;
			case '4': return Rank::FOUR;
			case '5': return Rank::FIVE;
			case '6': return Rank::SIX;
			case '7': return Rank::SEVEN;
			case '8': return Rank::EIGHT;
			case '9': return Rank::NINE;
			case 'J': return Rank::JACK;
			case 'Q': return Rank::QUEEN;
			case 'K': return Rank::KING;
			case 'A': return Rank::ACE;
			case '0':
				if (stream.get() == '1') return Rank::TEN;
				[[fallthrough]];

			default:  throw ParserError();
		}
	}

	friend std::hash<Rank>;
};

}

namespace std {

	template <>
	struct hash<kierki::Rank> {
		std::size_t operator()(const kierki::Rank &rank) const {
			return std::hash<kierki::Rank::Underlying>()(
				kierki::Rank::Underlying(rank.value)
			);
	        }
	};

}

#endif
