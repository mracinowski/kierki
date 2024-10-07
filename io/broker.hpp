#ifndef IO_BROKER_HPP
#define IO_BROKER_HPP

#include "channel.hpp"
#include "../debug.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <poll.h>
#include <ranges>
#include <set>
#include <vector>

namespace io {

class Broker {
	std::set<std::shared_ptr<Channel>> channels;
	std::vector<struct pollfd> pollfds;
	decltype(channels) new_channels;

	static short events(const std::shared_ptr<Channel> &channel) {
		short result = 0;
		if (channel->awaits_input())  result |= POLLIN;
		if (channel->awaits_output()) result |= POLLOUT;
		return result;
	}

	int setup() {
		pollfds.clear();
	
		Channel::TimePoint now = std::chrono::steady_clock::now();
	
		bool no_timeout = true;
		std::chrono::duration timeout = std::chrono::nanoseconds::max();
	
		for (const std::shared_ptr<Channel> &channel: channels) {
			pollfds.emplace_back(channel->fd, events(channel), 0);

			std::optional<Channel::TimePoint> deadline =
				channel->deadline();

			if (!deadline) continue;

			no_timeout = false;
			std::chrono::duration left =
				channel->deadline().value() - now;
	
			if (left < timeout) timeout = left;
		}
	
		if (no_timeout) return -1;
	
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			timeout
		).count();
	}

	std::set<std::shared_ptr<Channel>> deliver(
		const Channel::TimePoint &now,
		const std::shared_ptr<Channel> &channel,
		const struct pollfd &pollfd
	) {
		std::set<std::shared_ptr<Channel>> new_channels;

		if (channel->awaits_input() && (pollfd.revents & POLLIN)) {
			new_channels.insert(channel->handle_input());
		}

		if (channel->awaits_output() && (pollfd.revents & POLLOUT)) {
			new_channels.insert(channel->handle_output());
		}

		if (channel->deadline() && channel->deadline().value() <= now) {
			new_channels.insert(channel->handle_timeout());
		}
	
		new_channels.erase(nullptr);
	
		this->new_channels.insert(
			new_channels.begin(), new_channels.end()
		);
	
		return new_channels;
	}

	void process() {
		Channel::TimePoint now = std::chrono::steady_clock::now();

		auto pollfd = pollfds.cbegin();
		for (const std::shared_ptr<Channel> &channel: channels) {
			deliver(now, channel, *pollfd);
			pollfd++;
		}
	
		std::ranges::for_each(new_channels,
			std::bind(&Broker::attach, this, std::placeholders::_1)
		);
		new_channels.clear();
	
		std::erase_if(channels, [](const auto &channel) {
			return channel->closed();
		});
	}

public:
	static std::shared_ptr<Broker> build() {
		return std::make_shared<Broker>();
	}

	void attach(const std::shared_ptr<Channel> &channel) {
		channels.insert(channel);
	}

	int run() {
		while (!channels.empty()) {
			const int timeout = setup();

			BEFORE_POLL(pollfds, timeout);
			int result = poll(&pollfds[0], pollfds.size(), timeout);
			AFTER_POLL(pollfds, result);

			if (result == -1) return 1;

			process();
		}
	
		return 0;
	}
};

}

#endif
