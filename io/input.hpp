#ifndef IO_INPUT_HPP
#define IO_INPUT_HPP

#include "../debug.hpp"
#include "channel.hpp"

#include <functional>

namespace io {

class Input: virtual public Channel {
	const std::size_t CHUNK = 4096;

	const std::string separator;
	std::function<void(std::string)> on_message = [](auto...){};
	std::function<void()>            on_timeout = [](){};
	std::function<void()>            on_close   = [](){};
	std::function<void()>            on_error   = [](){};

	std::string buffer;
	std::optional<TimePoint> next_deadline = std::nullopt;

public:
	const std::string name;

	Input(
		int fd,
		const std::string &separator,
		const std::string &name = ""
	) : Channel(fd)
	, separator(separator)
	, name(name) {}

	void read(
		const decltype(on_message) &on_message,
		const std::optional<decltype(on_close)> &on_close =
			std::nullopt,
		const std::optional<decltype(on_error)> &on_error =
			std::nullopt
	) {
		this->on_message = on_message;
		if (on_close) this->on_close = on_close.value();
		if (on_error) this->on_error = on_error.value();
	}

	void expire(
		const std::chrono::seconds &timeout,
		const std::optional<decltype(on_timeout)> &on_timeout
	) {
		next_deadline = std::chrono::steady_clock::now() + timeout;
		if (on_timeout) this->on_timeout = on_timeout.value();
	}

	std::optional<TimePoint> deadline() const override {
		return next_deadline;
	}

	bool awaits_input() const override { return true; }
	std::shared_ptr<Channel> handle_input() override {
		next_deadline = std::nullopt;
	
		std::size_t size = buffer.size();
		buffer.resize(size + CHUNK);
		ssize_t length = ::read(fd, &buffer[size], CHUNK);
		READ(fd, &buffer[size], CHUNK, length);

		if (length == 0) {
			close();
			on_close();
			return nullptr;
		}
		if (length == -1) {
			buffer.resize(size);
			on_error();
			return nullptr;
		}

		buffer.resize(size + length);

		size_t pos;
		while ((pos = buffer.find(separator)) != std::string::npos) {
			std::string message = buffer.substr(0, pos);
			on_message(message);
			buffer.erase(0, pos + separator.length());
		}
	
		return nullptr;
	}

	std::shared_ptr<Channel> handle_close() {
		return nullptr;
	}

	std::shared_ptr<Channel> handle_timeout() override {
		next_deadline = std::nullopt;

		on_timeout();

		return nullptr;
	}
};

}

#endif
