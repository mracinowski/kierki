#ifndef KIERKI_PLAYERS_REMOTE_HPP
#define KIERKI_PLAYERS_REMOTE_HPP 1

#include "player.hpp"
#include "../report.hpp"
#include "../io/input.hpp"
#include "../io/output.hpp"
#include "../types/message.hpp"

namespace kierki::players {

class Remote : public Player, public std::enable_shared_from_this<Remote> {
	const std::shared_ptr<engines::Engine> engine;
	const std::shared_ptr<io::Input> input;
	const std::shared_ptr<io::Output> output;
	const std::shared_ptr<io::Output> report;
	const std::chrono::seconds timeout;

	void send(const auto &... parts) {
		std::string frame = (std::string(parts) + ...);

		if (report) report->write(Report::format(output->name, frame));
		output->write(frame);
	}

	void receive(const std::string &frame) {
		if (report) report->write(Report::format(input->name, frame));

		Message::parse(frame, {
			.IAM = std::bind(
				&engines::Engine::join,
				engine,
				shared_from_this(),
				std::placeholders::_1
			),
			.TRICK = std::bind(
				&engines::Engine::play,
				engine,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2
			),
			.invalid = std::bind(
				&Remote::close,
				shared_from_this()
			),
		});
	}

public:
	Remote(
		const std::shared_ptr<engines::Engine> &engine,
		const std::shared_ptr<io::Input> &input,
		const std::shared_ptr<io::Output> &output,
		const std::shared_ptr<io::Output> &report,
		const std::chrono::seconds &timeout
	) : engine(engine)
	, input(input)
	, output(output)
	, report(report)
	, timeout(timeout) {}

	static auto from_io(
		const std::shared_ptr<engines::Engine> &engine,
		const std::shared_ptr<io::Input> &input,
		const std::shared_ptr<io::Output> &output,
		const std::shared_ptr<io::Output> &report,
		const std::chrono::seconds &timeout
	) {
		auto player = std::make_shared<Remote>(
			engine, input, output, report, timeout
		);
	
		input->read(
			std::bind(
				&Remote::receive,
				player,
				std::placeholders::_1
			),
			std::bind(
				&Remote::close,
				player
			),
			std::bind(
				&Remote::close,
				player
			)
		);

		input->expire(
			timeout,
			std::bind(
				&Remote::close,
				player
			)
		);
	
		return player;
	}

	void join(const std::shared_ptr<engines::Engine> &) override {}

	void busy(
		const Seats &seats
	) override {
		send("BUSY", seats);
	}

	void deal(
		const Ruleset &ruleset,
		const Seat &leader,
		const Hand &hand
	) override {
		send("DEAL", ruleset, leader, hand);
	}

	void trick(
		const Number &number,
		const Cards &cards
	) override {
		if (std::string(number) == "14")
			throw std::runtime_error(">.<");

		if (!engine->active()) return;

		send("TRICK", number, cards);
		input->expire(
			timeout,
			std::bind(
				&Remote::trick,
				shared_from_this(),
				number,
				cards
			)
		);
	}

	void wrong(
		const Number &number
	) override {
		send("WRONG", number);
	}

	void taken(
		const Number &number,
		const Trick &trick,
		const Seat &taker
	) override {
		send("TAKEN", number, trick, taker);
	}

	void score(
		const Scores &scores
	) override {
		send("SCORE", scores);
	}

	void total(
		const Scores &scores
	) override {
		send("TOTAL", scores);
	}

	void close() override {
		output->close();
		engine->leave(shared_from_this(), false);
	}
};

}

#endif
