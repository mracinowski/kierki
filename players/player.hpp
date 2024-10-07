#ifndef KIERKI_PLAYER_HPP
#define KIERKI_PLAYER_HPP 1

#include "../types/message.hpp"
#include "../engines/engine.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

namespace kierki::engines { class Engine; }

namespace kierki::players {

class Player {
protected:
	static Card taken_card(
		const Seat &seat,
		Seat leader,
		const Trick &cards
	) {
		for (const Card &card: cards) {
			if (leader == seat) return card;
			leader++;
		}

		return cards.front();
	}

public:
	virtual void join(
		const std::shared_ptr<engines::Engine> &engine
	) = 0;

	virtual void busy(
		const Seats &taken
	) = 0;

	virtual void deal(
		const Ruleset &ruleset,
		const Seat &leader,
		const Hand &hand
	) = 0;

	virtual void wrong(
		const Number &number
	) = 0;

	virtual void taken(
		const Number &number,
		const Trick &cards,
		const Seat &taker
	) = 0;

	virtual void score(
		const Scores &scores
	) = 0;

	virtual void total(
		const Scores &scores
	) = 0;

	virtual void trick(
		const Number &number,
		const Cards &cards
	) = 0;

	virtual void close() = 0;
};

}

#endif
