#ifndef KIERKI_ENGINES_REMOTE_HPP
#define KIERKI_ENGINES_REMOTE_HPP 1

#include "../report.hpp"
#include "../io/input.hpp"
#include "../io/output.hpp"
#include "../types/message.hpp"
#include "engine.hpp"

namespace kierki::engines {

class Remote : public Engine, public std::enable_shared_from_this<Remote> {
	std::shared_ptr<players::Player> player;
	const std::shared_ptr<io::Input> input;
	const std::shared_ptr<io::Output> output;
	const std::shared_ptr<io::Output> report;
	const std::function<void(bool)> finalize;

	void send(const auto &... parts) {
		std::string frame = (std::string(parts) + ...);

		if (report) report->write(Report::format(output->name, frame));
		output->write(frame);
	}

	void ignore(const std::string &) {}

	void receive(const std::string &frame) {
		if (report) report->write(Report::format(input->name, frame));

		Message::parse(frame, {
			.BUSY = std::bind(
				&players::Player::busy,
				player,
				std::placeholders::_1
			),
			.DEAL = std::bind(
				&players::Player::deal,
				player,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3
			),
			.WRONG = std::bind(
				&players::Player::wrong,
				player,
				std::placeholders::_1
			),
			.TAKEN = std::bind(
				&players::Player::taken,
				player,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3
			),
			.SCORE = std::bind(
				&players::Player::score,
				player,
				std::placeholders::_1
			),
			.TOTAL = std::bind(
				&players::Player::total,
				player,
				std::placeholders::_1
			),
			.TRICK = std::bind(
				&players::Player::trick,
				player,
				std::placeholders::_1,
				std::placeholders::_2
			),
			.invalid = std::bind(
				&Remote::ignore,
				shared_from_this(),
				""
			)
		});
	}

public:
	Remote(
		const std::shared_ptr<io::Input> &input,
		const std::shared_ptr<io::Output> &output,
		const std::shared_ptr<io::Output> &report,
		const decltype(finalize) &finalize
	)
	: input(input)
	, output(output)
	, report(report)
	, finalize(finalize) {}

	static std::shared_ptr<Remote> from_io(
		const std::shared_ptr<io::Input> &input,
		const std::shared_ptr<io::Output> &output,
		const std::shared_ptr<io::Output> &report,
		const decltype(finalize) &finalize
	) {
		std::shared_ptr<Remote> engine = std::make_shared<Remote>(
			input, output, report, finalize
		);

		input->read(
			[](auto ...){},
			std::bind(
				&Remote::leave,
				engine,
				nullptr,
				false
			),
			std::bind(
				&Remote::leave,
				engine,
				nullptr,
				false
			)
		);

		return engine;
	}

	bool active() const override { return true; }

	void join(
		const std::shared_ptr<players::Player> &who,
		const Seat &seat
	) override {
		if (player != nullptr) return;
		player = who;

		input->read(
			std::bind(
				&Remote::receive,
				shared_from_this(),
				std::placeholders::_1
			),
			std::bind(
				&players::Player::close,
				player
			),
			std::bind(
				&players::Player::close,
				player
			)
		);

		send("IAM", seat);
	}

	void play(
		const std::shared_ptr<players::Player> &who,
		const Number &number,
		const Card &card
	) override {
		if (who != player) return;

		send("TRICK", number, card);
	}

	void leave(
		const std::shared_ptr<players::Player> &who,
		const bool complete
	) override {
		if (who != player) return;

		output->close();
		if (report) report->close();
		finalize(complete);
	}
};

}

#endif
