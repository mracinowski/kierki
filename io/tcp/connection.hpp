#ifndef IO_TCP_CONNECTION_HPP
#define IO_TCP_CONNECTION_HPP 1

#include "../channel.hpp"
#include "../input.hpp"
#include "../output.hpp"

#include <optional>

namespace io::tcp {

struct Connection : virtual public Input, virtual public Output {
	Connection(
		int fd,
		const std::string &separator,
		const Address &local,
		const Address &remote
	)
	: Channel(fd)
	, Input(
		fd,
		separator,
		std::string(remote) + "," + std::string(local)
	)
	, Output(
		fd,
		separator,
		std::string(local) + "," + std::string(remote)
	) {}

	static std::shared_ptr<Connection> from_fd(
		int fd,
		const std::string &separator
	) {
		const auto [sock, local] = Address::from_getsockname(fd);
		if (sock != 0) return nullptr;

		const auto [peer, remote] = Address::from_getpeername(fd);
		if (peer != 0) return nullptr;

		return std::make_shared<Connection>(
			fd, separator, local, remote
		);
	}

	bool closed() const override {
		return Input::closed() && Output::closed();
	}

	using Output::close;
};

}

#endif
