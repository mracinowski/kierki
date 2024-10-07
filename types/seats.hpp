#ifndef KIERKI_TYPES_SEATS_HPP
#define KIERKI_TYPES_SEATS_HPP 1

#include "common.hpp"
#include "seat.hpp"

#include <numeric>
#include <unordered_set>

namespace kierki {

struct Seats : public std::unordered_set<Seat> {
	using std::unordered_set<Seat>::unordered_set;

	operator std::string() const {
		return concat(cbegin(), cend());
	}

	static Seats rparse(std::istream &stream) {
		Seats seats;
		for (int i = 0; i < 4; i++) {
			auto position = stream.tellg();
			try {
				if (!seats.insert(Seat::rparse(stream)).second)
					throw ParserError("Duplicate seat");
				position = stream.tellg();
			} catch (const ParserError &) {
				stream.seekg(position);
				break;
			}
		}
		return seats;
	}

};

}

#endif
