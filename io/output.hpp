#ifndef IO_OUTPUT_HPP
#define IO_OUTPUT_HPP 1

#include "../debug.hpp"
#include "channel.hpp"

#include <functional>
#include <queue>
#include <string>

namespace io {

class Output: virtual public Channel {
	const std::string separator;

	bool open = true;
	size_t offset = 0;
	std::queue<std::string> messages;
	std::queue<std::function<void()>> callbacks;

public:
	const std::string name;

	Output(
		int fd,
		const std::string &separator,
		const std::string &name = ""
	)
	: Channel(fd)
	, separator(separator)
	, name(name) {}

	void close() override {
		open = false;

		if (awaits_output()) return;

		Channel::close();
	}

	bool awaits_output() const override { return !messages.empty(); }
	std::shared_ptr<Channel> handle_output() override {
		if (offset == messages.front().size()) {
			callbacks.front()();

			offset = 0;
			messages.pop();
			callbacks.pop();

			if (messages.empty()) {
				if (!open) close();
				return nullptr;
			}
		}

		const std::string &message = messages.front();
		const ssize_t count = message.size() - offset;

		ssize_t length = ::write(fd, &message[offset], count);
		WRITE(fd, &message[offset], count, length);

		if (length == -1) {
			while (!messages.empty()) {
				messages.pop();
				callbacks.pop();
			}

			close();
			return nullptr;
		}

		offset += length;

		return nullptr;
	}

	virtual bool write(
		const std::string &message,
		const std::function<void()> &callback = [](){}
	) {
		messages.push(message + separator);
		callbacks.push(callback);
	
		if (!awaits_output()) handle_output();

		return true;
	};

};

}

#endif
