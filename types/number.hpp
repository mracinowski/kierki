#ifndef KIERKI_TYPES_NUMBER_HPP
#define KIERKI_TYPES_NUMBER_HPP 1

#include "common.hpp"

#include <compare>
#include <type_traits>

namespace kierki {

class Number {
	enum class Value {
		FIRST = 1, SECOND = 2, THIRD = 3, FOURTH = 4, FIFTH = 5,
		SIXTH = 6, SEVENTH = 7, EIGHTH = 8, NINTH = 9, TENTH = 10,
		ELEVENTH = 11, TWELFTH = 12, THIRTEENTH = 13
	} value;
	using Underlying = std::underlying_type_t<Value>;

	constexpr Number(Underlying value) : value(Value(value)) {}
public:
	using Value::FIRST, Value::SECOND, Value::THIRD, Value::FOURTH,
		Value::FIFTH, Value::SIXTH, Value::SEVENTH, Value::EIGHTH,
		Value::NINTH, Value::TENTH, Value::ELEVENTH, Value::TWELFTH,
		Value::THIRTEENTH;
	static const Value LAST = Value::THIRTEENTH;

	static const Underlying COUNT = 13;

	constexpr Number(): value(FIRST) {}
	constexpr Number(Value value): value(value) {}

	std::strong_ordering operator<=>(const Number &) const = default;

	Number &operator++() {
		if (value == LAST) return *this;

		value = Value(Underlying(value) + 1);
		return *this;
	}

	Number operator++(int) {
		Number previous = *this;
		operator++();
		return previous;
	}

	operator char const *() const {
		switch (value) {
			case FIRST:      return "1";
			case SECOND:     return "2";
			case THIRD:      return "3";
			case FOURTH:     return "4";
			case FIFTH:      return "5";
			case SIXTH:      return "6";
			case SEVENTH:    return "7";
			case EIGHTH:     return "8";
			case NINTH:      return "9";
			case TENTH:      return "10";
			case ELEVENTH:   return "11";
			case TWELFTH:    return "12";
			case THIRTEENTH: return "13";
			default:     throw std::domain_error("Invalid number");
		}
	}

	operator std::string() const {
		return std::to_string(Underlying(value));
	}

	static Number rparse(std::istream &stream) {
		switch (char digit = stream.get()) {
			case '0':
				if (stream.get() == '1') return Number(10);
				else throw ParserError();

			case '1':
			case '2':
			case '3':
				if (stream.peek() == '1') {
					stream.get();
					return Number(10 + digit - '0');
				}
				[[fallthrough]];

			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': return Number(digit - '0');
			default:  throw ParserError();
		}
	}
};

}

#endif
