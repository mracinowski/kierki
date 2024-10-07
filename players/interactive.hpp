#ifndef KIERKI_PLAYERS_INTERACTIVE_HPP
#define KIERKI_PLAYERS_INTERACTIVE_HPP 1

#include "player.hpp"
#include "../io/input.hpp"
#include "../io/output.hpp"

#include <optional>

#define INVALID_COMMAND "Invalid command"
#define NOT_YOUR_TURN "Not your turn"
#define YOUR_HAND_IS "Your current cards are:"

namespace kierki::players {

class Interactive : public Player
, public std::enable_shared_from_this<Interactive> {
	const Seat seat;
	const std::shared_ptr<io::Input> input;
	const std::shared_ptr<io::Output> output;

	std::shared_ptr<engines::Engine> engine;

	Seat leader = Seat::NORTH;
	Number number = Number::FIRST;
	Hand hand = {};
	std::vector<Trick> history;
	bool got_score = false;
	bool got_total = false;

	std::string scoreboard(const Scores &scores) {
		std::string separator, result;

		for (const auto &[seat, score]: scores) {
			result += separator;
			result += std::string(seat);
			result += " | ";
			result += std::string(score);

			separator = "\n";
		}

		return result;
	}

	std::string format(const auto &r) {
		std::string separator, result;

		for (const auto &i: r) {
			result += separator + std::string(i);

			separator = ", ";
		}

		return result;
	}

	void send(const auto &... parts) {
		output->write((std::string(parts) + ...));
	}

	bool play(const std::string &command) {
		if (!command.starts_with("!")) return false;

		try {
			std::istringstream stream(std::string(
				command.crbegin(), command.crend() - 1
			));

			Card card = Card::rparse(stream);

			if (std::string(card).length() + 1 != command.length())
				throw ParserError();

			engine->play(
				shared_from_this(),
				number,
				card
			);

			return true;
		} catch (const ParserError &) {
			return false;
		}
	}

	bool cards(const std::string &command) {
		if (command != "cards") return false;

		send(format(hand));
		return true;
	}

	bool tricks(const std::string &command) {
		if (command != "tricks") return false;

		std::string formatted;

		for (const Trick &trick: history) {
			formatted.append(format(trick));
			formatted.append("\n");
		}

		if (!formatted.empty()) formatted.pop_back();

		send(formatted);

		return true;
	}


	void receive(const std::string &command) {
		if (play(command)) return;
		if (cards(command)) return;
		if (tricks(command)) return;
		send(INVALID_COMMAND, " \"", command, "\"");
	}

public:
	Interactive(
		const Seat &seat,
		const std::shared_ptr<io::Input> &input,
		const std::shared_ptr<io::Output> &output
	)
	: seat(seat)
	, input(input)
	, output(output) {}

	static std::shared_ptr<Interactive> from_io(
		const Seat seat,
		const std::shared_ptr<io::Input> input,
		const std::shared_ptr<io::Output> output
	) {
		auto result = std::make_shared<Interactive>(
			seat, input, output
		);

		input->read(
			std::bind(
				&Interactive::receive,
				result,
				std::placeholders::_1
			)
		);

		return result;
	}

	void join(
		const std::shared_ptr<engines::Engine> &engine
	) override {
		if (this->engine || !engine) return;

		this->engine = engine;
		this->engine->join(shared_from_this(), seat);
	}

	void busy(
		const Seats &seats
	) override {
		send(
			"Place busy, list of busy places received: ",
			format(seats),
			"."
		);
	}

	void deal(
		const Ruleset &ruleset,
		const Seat &leader,
		const Hand &hand
	) override {
		this->leader = leader;
		this->hand = hand;
		this->number = Number::FIRST;
		this->history.clear();
		this->got_score = false;

		send(
			"New deal ", ruleset,
			": staring place ", leader,
			", your cards: ", format(hand),
			"."
		);
	}

	void wrong(
		const Number &number
	) override {
		send(
			"Wrong message received in trick ", number, "."
		);
	}

	void taken(
		const Number &number,
		const Trick &cards,
		const Seat &taker
	) override {
		this->number = number;
		this->number++;

		this->history.push_back(cards);

		Card card = taken_card(this->seat, this->leader, cards);
		this->hand.extract(card);

		this->leader = taker;

		send(
			"A trick ", number,
			" is taken by ", taker,
			", cards ", format(cards),
			"."
		);
	}

	void score(
		const Scores &scores
	) override {
		this->got_score = true;

		send(
			"The scores are:\n",
			scoreboard(scores)
		);
	}

	void total(
		const Scores &scores
	) override {
		this->got_total = true;

		send(
			"The total scores are:\n",
			scoreboard(scores)
		);
	}

	void trick(
		const Number &number,
		const Cards &cards
	) override {
		send(
			"Trick: (", number, ") ",
			format(cards), "\n",
			"Available: ", format(hand)
		);
	}

	void close() override {
		input->close();
		output->close();

		if (engine) engine->leave(
			shared_from_this(),
			this->got_score && this->got_total
		);
	}
};

}

#endif
