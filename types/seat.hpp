#ifndef KIERKI_TYPES_SEAT_HPP
#define KIERKI_TYPES_SEAT_HPP 1

#include "common.hpp"

#include <type_traits>

namespace kierki {

class Seat {
	enum class Value {
		NORTH, EAST, SOUTH, WEST
	} value;
	using Underlying = std::underlying_type_t<Value>;

public:
	using Value::NORTH, Value::EAST, Value::SOUTH, Value::WEST;
	static const Underlying COUNT = 4;

	constexpr Seat(Value value): value(value) {}

	Seat &operator++() {
		value = Value((Underlying(value) + 1) % COUNT);
		return *this;
	}

	Seat operator++(int) {
		Seat previous = *this;
		operator++();
		return previous;
	}

	operator char const *() const {
		switch (value) {
			case NORTH: return "N";
			case EAST:  return "E";
			case SOUTH: return "S";
			case WEST:  return "W";
			default:    throw std::domain_error("Invalid seat");
		}
	}

	operator std::string() const {
		return operator char const *();
	}

	static Seat rparse(std::istream &stream) {
		switch (stream.get()) {
			case 'N': return Seat::NORTH;
			case 'E': return Seat::EAST;
			case 'S': return Seat::SOUTH;
			case 'W': return Seat::WEST;
			default:  throw ParserError();
		}
	}

	friend std::hash<Seat>;
};

}

namespace std {

	template <>
	struct hash<kierki::Seat> {
		std::size_t operator()(const kierki::Seat &seat) const {
			return std::hash<kierki::Seat::Underlying>()(
				kierki::Seat::Underlying(seat.value)
			);
	        }
	};

}

#endif
