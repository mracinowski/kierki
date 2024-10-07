#ifndef IO_CHANNEL_HPP
#define IO_CHANNEL_HPP 1

#include <chrono>
#include <memory>
#include <optional>
#include <sys/ioctl.h>

namespace io {

class Channel {
	bool open = true;

	int setup(int fd) {
		int on = 1;
		if (ioctl(fd, FIONBIO, (char *)&on) < 0) exit(1);

		return fd;
	}

protected:
	Channel(int fd) : fd(setup(fd)) {}

public:
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	const int fd;

	virtual bool closed() const { return !open; }
	virtual void close() {
		if (closed()) return;

		open = false;
		::close(fd);
	}

	virtual std::optional<TimePoint> deadline() const {
		return std::nullopt;
	}

	virtual bool awaits_input() const { return false; }
	virtual std::shared_ptr<Channel> handle_input() { return nullptr; }

	virtual bool awaits_output() const { return false; }
	virtual std::shared_ptr<Channel> handle_output() { return nullptr; }

	virtual std::shared_ptr<Channel> handle_timeout() { return nullptr; }
};

}

#endif
