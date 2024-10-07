#ifndef KIERKI_TYPES_ROUND_HPP
#define KIERKI_TYPES_ROUND_HPP 1

#include "common.hpp"
#include "trick.hpp"
#include "ruleset.hpp"
#include "seat.hpp"
#include "hand.hpp"

#include <type_traits>
#include <unordered_map>

namespace kierki {

class Round {
	Ruleset _ruleset;
	Seat _leader;
	std::unordered_map<Seat, Hand> _hands;

public:
	static const auto COUNT = Rank::COUNT * Suit::COUNT;

	Round(
		const Ruleset &ruleset,
		const Seat &leader,
		const Hand &north,
		const Hand &east,
		const Hand &south,
		const Hand &west
	)
	: _ruleset(ruleset)
	, _leader(leader)
	, _hands{
		{Seat::NORTH, north},
		{Seat::EAST, east},
		{Seat::SOUTH, south},
		{Seat::WEST, west}
	} {}

	const auto &ruleset() const { return _ruleset; }
	const auto &leader() const { return _leader; }
	const auto &hands() const { return _hands; }
	const auto tally(const auto &... args) const {
		return ruleset().tally(args...);
	}

	static Round rparse(std::istream &stream) {
		if (stream.get() != '\n') throw ParserError("First LF");
		Hand west = Hand::rparse(stream);
		if (stream.get() != '\n') throw ParserError();
		Hand south = Hand::rparse(stream);
		if (stream.get() != '\n') throw ParserError();
		Hand east = Hand::rparse(stream);
		if (stream.get() != '\n') throw ParserError();
		Hand north = Hand::rparse(stream);
		if (stream.get() != '\n') throw ParserError();
		Seat leader = Seat::rparse(stream);
		Ruleset ruleset = Ruleset::rparse(stream);
		return Round(ruleset, leader, north, east, south, west);
	}
};

}

#endif
