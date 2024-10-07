#ifndef KIERKI_ENGINES_LOCAL_HPP
#define KIERKI_ENGINES_LOCAL_HPP 1

#include "../io/channel.hpp"
#include "../types/round.hpp"
#include "engine.hpp"

#include <forward_list>
#include <istream>
#include <unordered_set>
#include <vector>

namespace kierki::engines {

class Local : public Engine {
	const std::forward_list<Round> rounds;
	const std::function<void()> finalize;

	std::unordered_map<Seat, std::shared_ptr<players::Player>> players;
	decltype(rounds)::const_iterator round;
	Scores totals;
	bool complete = false;

	std::unordered_set<Seat> queue;

	std::unordered_map<Seat, Hand> hands;
	std::vector<std::tuple<Number, Trick, Seat>> tricks;
	Number number;
	Scores scores;

	Seat turn = Seat::NORTH;
	Cards cards;

	std::optional<Seat> locate(
		const std::shared_ptr<players::Player> &who
	) {
		const auto players = this->players;

		for (const auto &[seat, player]: players)
			if (who == player) return seat;

		return std::nullopt;
	}

	bool check(
		const std::shared_ptr<players::Player> &player,
		const Number &number,
		const Card &card
	) const {
		if (player != players.at(turn)) {
			return false;
		}

		if (number != this->number) {
			return false;
		}

		const Hand &hand = hands.at(turn);

		if (!hand.contains(card)) {
			return false;
		}

		if (cards.empty()) return true;

		Suit leading = cards.front().suit();

		if (card.suit() == leading) return true;

		if (std::ranges::count_if(hand, 
			[leading] (const auto &card) {
				return card.suit() == leading;
			}
		) == 0) {
			return true;
		}

		return false;
	}

	Seat judge(const Seat &leader, const Trick &trick) {
		Suit suit = trick.front().suit();

		std::multiset<Rank> ranks;
		for (Card card: trick)
			if (card.suit() == suit) ranks.insert(card.rank());

		Card highest(*--ranks.cend(), suit);

		Seat result = leader;
		for (Card card: trick) {
			if (card == highest)
				return result;

			result++;
		}

		return Seat::NORTH;
	}

	bool setup() {
		if (round != rounds.cbefore_begin() && tricks.size() < 13)
			return true;

		round++;

		if (round == rounds.end())
			return false;

		hands = round->hands();
		turn = round->leader();
		tricks.clear();
		number = Number::FIRST;
		scores = Scores();

		return true;
	}

	void deal(const Seat &seat, const bool fresh = false) {
		queue.insert(seat);

		if (
			(fresh && !active()) ||
			(!fresh && queue.size() < Seat::COUNT)
		) return;

		if (!setup()) { total(); return; }

		for (const Seat &seat: queue) {
			players.at(seat)->deal(
				round->ruleset(),
				round->leader(),
				round->hands().at(seat)
			);
		}

		for (const auto &[number, trick, taker]: tricks) {
			for (const auto &seat: queue) {
				players.at(seat)->taken(
					number, trick, taker
				);
			}
		}

		queue.clear();

		trick();
	}

	void trick() {
		players.at(turn)->trick(number, cards);
	}

	void taken() {
		Trick trick{ cards[0], cards[1], cards[2], cards[3] };
		Seat taker = judge(turn, trick);

		tricks.emplace_back(number, trick, taker);

		Score points = round->ruleset().tally(number, trick);
		scores.at(taker) += points;
		totals.at(taker) += points;

		const auto players = this->players;

		for (const auto &[seat, player]: players) {
			player->taken(number, trick, taker);
		}

		turn = taker;
		cards.clear();

		if (number == Number::LAST) score();
		else { number++; Local::trick(); }
	}

	void score() {
		const auto players = this->players;

		for (const auto &[seat, player]: players)
			player->score(scores);

		for (const auto &[seat, player]: players)
			deal(seat);
	}

	void total() {
		const auto players = this->players;

		for (const auto &[seat, player]: players) {
			player->total(totals);
			player->close();
		}

		finalize();
	}

public:
	Local(
		const std::forward_list<Round> &_rounds,
		const decltype(finalize) &finalize
	)
	: rounds(_rounds)
	, finalize(finalize)
	, round(rounds.before_begin()) {}

	static std::shared_ptr<Local> parse(
		std::istream &stream,
		const decltype(finalize) &finalize
	) {
		std::stringstream ss;
		ss << stream.rdbuf();
		std::string s = ss.str();
		std::reverse(s.begin(), s.end());
		std::stringstream sss(s);

		std::forward_list<Round> rounds;
		do {
			rounds.push_front(Round::rparse(sss));
		} while (sss.peek() == '\n');

		return std::make_shared<Local>(rounds, finalize);
	}

	bool active() const override {
		return players.size() == Seat::COUNT;
	}

	void join(
		const std::shared_ptr<players::Player> &who,
		const Seat &seat
	) override {
		if (locate(who)) {
			who->close();
			return;
		}

		if (!players.emplace(seat, who).second) {
			Seats seats;

			for (const auto &[seat, player]: players)
				seats.insert(seat);

			who->busy(seats);
			who->close();

			return;
		}

		deal(seat, true);
	}

	void play(
		const std::shared_ptr<players::Player> &who,
		const Number &number,
		const Card &card
	) override {
		if (!locate(who)) {
			who->close();
			return;
		}

		if (!active()) {
			who->wrong(this->number);
			return;
		}

		if (!check(who, number, card)) {
			who->wrong(this->number);
			return;
		}

		hands.at(turn).extract(card);
		cards.push_back(card);
		++turn;

		if (cards.size() < Seat::COUNT) trick();
		else taken();
	}

	void leave(
		const std::shared_ptr<players::Player> &who,
		const bool
	) override {
		auto seat = locate(who);
		if (!seat) return;

		queue.erase(seat.value());
		players.erase(seat.value());
	}
};

}

#endif
