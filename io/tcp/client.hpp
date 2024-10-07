#ifndef IO_TCP_CLIENT_HPP
#define IO_TCP_CLIENT_HPP 1

#include "../channel.hpp"
#include "address.hpp"
#include "connection.hpp"

#include <netinet/tcp.h>

namespace io::tcp {

class Client : public Channel {
	const std::string separator;
	const std::function<void(std::shared_ptr<Connection>)> on_connect;
	const std::function<void(int)> on_fail;
	const std::function<void(int)> on_error;
	bool connected = false;

public:
	Client(
		int fd,
		const std::string &separator,
		const decltype(on_connect) &on_connect,
		const decltype(on_fail) &on_fail,
		const decltype(on_error) &on_error
	)
	: Channel(fd)
	, separator(separator)
	, on_connect(on_connect)
	, on_fail(on_fail)
	, on_error(on_error) {}

	static std::shared_ptr<Client> from_address(
		const std::string &separator,
		int domain,
		std::string host,
		int port,
		const decltype(on_connect) &on_connect,
		const decltype(on_fail) &on_fail,
		const decltype(on_error) &on_error
	) {
		auto result = Address::from_address(domain, host, port);
		if (!result) {
			errno = 0;
			return nullptr;
		}

		Address address = result.value();

		int fd = socket(
			address.family(), SOCK_STREAM | SOCK_NONBLOCK, 0
		);
		if (fd == -1)
			return nullptr;

		int on = 1;
		if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)))
			return nullptr;

		if (
			int result = connect(fd, address, address.size());
			result != 0 && !(result == -1 && errno == EINPROGRESS)
		)
			return nullptr;

		return std::make_shared<Client>(
			fd, separator, on_connect, on_fail, on_error
		);
	}

	bool closed() const override {
		return connected || Channel::closed();
	}

	bool awaits_output() const override {
		return !closed();
	}

	std::shared_ptr<Channel> handle_output() override {
		int error;
		socklen_t len = sizeof(error);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) != 0) {

			close();
			on_error(errno);

			return nullptr;
		}

		connected = true;

		if (error) {
			close();
			on_fail(error);

			return nullptr;
		}

		std::shared_ptr<Connection> connection =
			Connection::from_fd(
				fd,
				separator
			);

		if (!connection) {
			close();
			on_error(errno);

			return nullptr;
		}

		on_connect(connection);

		return connection;
	}
};

}

#endif

