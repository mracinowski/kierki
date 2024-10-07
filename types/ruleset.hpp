#ifndef KIERKI_TYPES_RULESET_HPP
#define KIERKI_TYPES_RULESET_HPP 1

#include "common.hpp"
#include "card.hpp"
#include "trick.hpp"
#include "number.hpp"
#include "score.hpp"

#include <algorithm>
#include <ranges>
#include <type_traits>

namespace kierki {

class Ruleset {
	enum class Value {
		TRICKS, HEARTS, LADIES, LORDS, KING, GOLDEN, ROBBER
	} value;
	using Underlying = std::underlying_type_t<Value>;

	static Score tricks(const Number &, const Trick &) { return 1; }

	static Score hearts(const Number &, const Trick &cards) {
		return std::ranges::count_if(cards, [](const Card &card){
			return card.suit() == Suit::HEART;
		});
	}

	static Score ladies(const Number &, const Trick &cards) {
		return 5 * std::ranges::count_if(cards, [](const Card &card){
			return card.rank() == Rank::QUEEN;
		});
	}

	static Score lords(const Number &, const Trick &cards) {
		return 2 * std::ranges::count_if(cards, [](const Card &card){
			return card.rank() == Rank::JACK
				|| card.rank() == Rank::KING;
		});
	}

	static Score king(const Number &, const Trick &cards) {
		return 18 * std::ranges::count(cards, Card(
			Rank::KING, Suit::HEART
		));
	}

	static Score golden(const Number &number, const Trick &) {
		return number == Number::SEVENTH
			|| number == Number::LAST ? 10 : 0;
	}

	static Score robber(const Number &number, const Trick &cards) {
		return
			  tricks(number, cards)
			+ hearts(number, cards)
			+ ladies(number, cards)
			+ lords(number, cards)
			+ king(number, cards)
			+ golden(number, cards);
	}

public:
	using Value::TRICKS, Value::HEARTS, Value::LADIES, Value::LORDS,
		Value::KING, Value::GOLDEN, Value::ROBBER;
	static const Underlying COUNT = 7;

	constexpr Ruleset(Value value): value(value) {}

	const Score tally(const Number &number, const Trick &cards) const {
		switch (value) {
			case TRICKS: return tricks(number, cards);
			case HEARTS: return hearts(number, cards);
			case LADIES: return ladies(number, cards);
			case LORDS:  return lords(number, cards);
			case KING:   return king(number, cards);
			case GOLDEN: return golden(number, cards);
			case ROBBER: return robber(number, cards);
			default:    throw std::domain_error("Invalid ruleset");
		}
	}

	operator char const *() const {
		switch (value) {
			case TRICKS: return "1";
			case HEARTS: return "2";
			case LADIES: return "3";
			case LORDS:  return "4";
			case KING:   return "5";
			case GOLDEN: return "6";
			case ROBBER: return "7";
			default:    throw std::domain_error("Invalid ruleset");
		}
	}
		
	static Ruleset rparse(std::istream &stream) {
		switch (stream.get()) {
			case '1': return Ruleset::TRICKS;
			case '2': return Ruleset::HEARTS;
			case '3': return Ruleset::LADIES;
			case '4': return Ruleset::LORDS;
			case '5': return Ruleset::KING;
			case '6': return Ruleset::GOLDEN;
			case '7': return Ruleset::ROBBER;
			default:  throw ParserError();
		}
	}
};

}

#endif
