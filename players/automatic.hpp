#ifndef KIERKI_PLAYERS_AUTOMATIC_HPP
#define KIERKI_PLAYERS_AUTOMATIC_HPP 1

#include "player.hpp"

namespace kierki::players {

class Automatic : public Player, public std::enable_shared_from_this<Automatic> {
	const Seat seat;

	std::shared_ptr<engines::Engine> engine;

	Seat leader = Seat::NORTH;
	Hand hand;
	bool got_score = false;
	bool got_total = false;

	Card pick(const Cards &cards) {
		if (!cards.empty()) {
			Suit leading = cards.front().suit();

			for (const Card &card: hand) {
				if (card.suit() == leading) return card;
			}
		}

		return *hand.begin();
	}

public:
	Automatic(
		const Seat &seat
	)
	: seat(seat) {}

	static std::shared_ptr<Automatic> from_seat(
		const Seat seat
	) {
		return std::make_shared<Automatic>(seat);
	}

	void join(
		const std::shared_ptr<engines::Engine> &engine
	) override {
		if (this->engine || !engine) return;

		this->engine = engine;
		this->engine->join(shared_from_this(), seat);
	}

	void busy(
		const Seats &
	) override {
		close();
	}

	void deal(
		const Ruleset &,
		const Seat &leader,
		const Hand &hand
	) override {
		this->leader = leader;
		this->hand = hand;
		this->got_score = false;
	}

	void wrong(
		const Number &
	) override {}

	void taken(
		const Number &,
		const Trick &cards,
		const Seat &taker
	) override {
		this->hand.extract(
			taken_card(this->seat, this->leader, cards)
		);

		this->leader = taker;
	}

	void score(
		const Scores &
	) override {
		this->got_score = true;
	}

	void total(
		const Scores &
	) override {
		this->got_total = true;
	}

	void trick(
		const Number &number,
		const Cards &cards
	) override {
		if (hand.empty()) return;

		Card card = pick(cards);
		hand.extract(card);
		engine->play(shared_from_this(), number, card);
	}

	void close() override {
		if (engine) engine->leave(
			shared_from_this(),
			this->got_score && this->got_total
		);
	}
};

}

#endif
