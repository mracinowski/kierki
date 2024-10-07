#ifndef KIERKI_TYPES_SCORES_HPP
#define KIERKI_TYPES_SCORES_HPP 1

#include "common.hpp"
#include "seat.hpp"
#include "score.hpp"

#include <unordered_set>

namespace kierki {

struct Scores : public std::unordered_map<Seat, Score> {
	using std::unordered_map<Seat, Score>::unordered_map;

	Scores() : std::unordered_map<Seat, Score>::unordered_map{
		{Seat::NORTH, 0},
		{Seat::EAST, 0},
		{Seat::SOUTH, 0},
		{Seat::WEST, 0},
	} {}

	operator std::string() const {
		std::string result;
		for (const auto &[seat, score]: *this)
			result += std::string(seat) + std::string(score);
		return result;
	}

	static Scores rparse(std::istream &stream) {
		Scores scores;
		std::unordered_set<Seat> seats;
		for (int i = 0; i < 4; i++) {
			Score score = Score::rparse(stream);
			Seat seat = Seat::rparse(stream);

			if (seats.contains(seat)) throw ParserError();
			scores.at(seat) = score;
		}
		return scores;
	}

};

}

#endif
