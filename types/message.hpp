#ifndef KIERKI_PARSERS_MESSAGE_HPP
#define KIERKI_PARSERS_MESSAGE_HPP 1

#include "common.hpp"
#include "number.hpp"
#include "cards.hpp"
#include "ruleset.hpp"
#include "hand.hpp"
#include "trick.hpp"
#include "seats.hpp"
#include "scores.hpp"

namespace kierki {

class Message {
public:
	static bool validate(
		const auto &parser,
		const auto &... args
	) {
		try {
			parser(args...);
			return true;
		} catch (const ParserError &) {
			return false;
		}
	}

	static auto unpack(
		const std::string &frame,
		const std::string &header
	) {
		if (!frame.starts_with(header)) throw ParserError();
		
		return std::istringstream(std::string(
			frame.crbegin(), frame.crend() - header.size()
		));
	}

	struct Clients {
		std::function<void(Seat)> IAM;
		std::function<void(Number, Card)> TRICK;
		std::function<void()> invalid;
	};

	struct Client {
		static void IAM(
			const std::string &frame,
			const decltype(Clients::IAM) &callback
		) {
			auto stream = unpack(frame, "IAM");
			Seat seat = Seat::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(seat);
		}

		static void TRICK(
			const std::string &frame,
			const decltype(Clients::TRICK) &callback
		) {
			auto stream = unpack(frame, "TRICK");
			Card card = Card::rparse(stream);
			Number number = Number::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(number, card);
		}
	};

	struct Servers {
		std::function<void(Seats)> BUSY;
		std::function<void(Ruleset, Seat, Hand)> DEAL;
		std::function<void(Number)> WRONG;
		std::function<void(Number, Trick, Seat)> TAKEN;
		std::function<void(Scores)> SCORE;
		std::function<void(Scores)> TOTAL;
		std::function<void(Number, Cards)> TRICK;
		std::function<void()> invalid;
	};

	struct Server {
		static void BUSY(
			const std::string &frame,
			const decltype(Servers::BUSY) &callback
		) {
			auto stream = unpack(frame, "BUSY");
			Seats seats = Seats::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(seats);
		}

		static void DEAL(
			const std::string &frame,
			const decltype(Servers::DEAL) &callback
		) {
			auto stream = unpack(frame, "DEAL");
			Hand hand = Hand::rparse(stream);
			Seat seat = Seat::rparse(stream);
			Ruleset ruleset = Ruleset::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(ruleset, seat, hand);
		}

		static void WRONG(
			const std::string &frame,
			const decltype(Servers::WRONG) &callback
		) {
			auto stream = unpack(frame, "WRONG");
			Number number = Number::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(number);
		}

		static void TAKEN(
			const std::string &frame,
			const decltype(Servers::TAKEN) &callback
		) {
			auto stream = unpack(frame, "TAKEN");
			Seat seat = Seat::rparse(stream);
			Trick trick = Trick::rparse(stream);
			Number number = Number::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(number, trick, seat);
		}

		static void SCORE(
			const std::string &frame,
			const decltype(Servers::SCORE) &callback
		) {
			auto stream = unpack(frame, "SCORE");
			Scores scores = Scores::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(scores);
		}

		static void TOTAL(
			const std::string &frame,
			const decltype(Servers::TOTAL) &callback
		) {
			auto stream = unpack(frame, "TOTAL");
			Scores scores = Scores::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(scores);
		}

		static void TRICK(
			const std::string &frame,
			const decltype(Servers::TRICK) &callback
		) {
			auto stream = unpack(frame, "TRICK");
			Cards cards = Cards::rparse(stream);
			Number number = Number::rparse(stream);
			if (stream.get(); !stream.eof()) throw ParserError();
			callback(number, cards);
		}
	};

public:
	static void parse(const std::string &frame, const Clients &callbacks) {
		if (validate(Client::IAM, frame, callbacks.IAM)) return;
		if (validate(Client::TRICK, frame, callbacks.TRICK)) return;
		callbacks.invalid();
	}

	static void parse(const std::string &frame, const Servers &callbacks) {
		if (validate(Server::BUSY, frame, callbacks.BUSY)) return;
		if (validate(Server::DEAL, frame, callbacks.DEAL)) return;
		if (validate(Server::WRONG, frame, callbacks.WRONG)) return;
		if (validate(Server::TAKEN, frame, callbacks.TAKEN)) return;
		if (validate(Server::SCORE, frame, callbacks.SCORE)) return;
		if (validate(Server::TOTAL, frame, callbacks.TOTAL)) return;
		if (validate(Server::TRICK, frame, callbacks.TRICK)) return;
		callbacks.invalid();
	}
};

}

#endif
