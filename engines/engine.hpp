#ifndef KIERKI_ENGINE_HPP
#define KIERKI_ENGINE_HPP

#include "../types/message.hpp"
#include "../players/player.hpp"

#include <memory>

namespace kierki::players { class Player; }

namespace kierki::engines {

struct Engine {
	virtual bool active() const = 0;

	virtual void join(
		const std::shared_ptr<players::Player> &who,
		const Seat &seat
	) = 0;

	virtual void play(
		const std::shared_ptr<players::Player> &who,
		const Number &number,
		const Card &card
	) = 0;

	virtual void leave(
		const std::shared_ptr<players::Player> &who,
		const bool complete
	) = 0;
};

}

#endif
