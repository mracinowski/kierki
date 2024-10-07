#ifndef KIERKI_TYPES_SUIT_HPP
#define KIERKI_TYPES_SUIT_HPP 1

#include "common.hpp"

#include <compare>
#include <type_traits>

namespace kierki {

class Suit {
	enum class Value {
		CLUB, DIAMOND, HEART, SPADE
	} value;
	using Underlying = std::underlying_type_t<Value>;

	constexpr Suit(Underlying value) : value(Value(value)) {}

public:
	using Value::CLUB, Value::DIAMOND, Value::HEART, Value::SPADE;
	static const Underlying COUNT = 4;

	constexpr Suit(Value value): value(value) {}

	std::strong_ordering operator<=>(const Suit &) const = default;

	operator char const *() const {
		switch (value) {
			case CLUB:    return "C";
			case DIAMOND: return "D";
			case HEART:   return "H";
			case SPADE:   return "S";
			default:      throw std::runtime_error("Invalid suit");
		}
	}
		
	static Suit rparse(std::istream &stream) {
		switch (stream.get()) {
			case 'C': return Suit::CLUB;
			case 'D': return Suit::DIAMOND;
			case 'H': return Suit::HEART;
			case 'S': return Suit::SPADE;
			default:  throw ParserError();
		}
	}

	friend std::hash<Suit>;
};

}

namespace std {

	template <>
	struct hash<kierki::Suit> {
		std::size_t operator()(const kierki::Suit &suit) const {
			return std::hash<kierki::Suit::Underlying>()(
				kierki::Suit::Underlying(suit.value)
			);
	        }
	};

}

#endif
