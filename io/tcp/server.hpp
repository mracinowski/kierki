#ifndef IO_TCP_SERVER_HPP
#define IO_TCP_SERVER_HPP 1

#include "../channel.hpp"
#include "address.hpp"
#include "connection.hpp"

#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <functional>
#include <memory>

namespace io::tcp {

class Server : public Channel {
	const std::string separator;
	const std::function<void(std::shared_ptr<Connection>)> on_accept;
	const std::function<void(int)> on_error;

public:
	Server(
		int fd,
		const std::string &separator,
		const decltype(on_accept) &on_accept,
		const decltype(on_error) &on_error
	)
	: Channel(fd)
	, separator(separator)
	, on_accept(on_accept)
	, on_error(on_error) {}

	static std::shared_ptr<Server> build(
		const std::string &separator,
		int port,
		const decltype(on_accept) &on_accept,
		const decltype(on_error) &on_error
	) {
		int backlog = 5;

		int fd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (fd == -1) return nullptr;

		int on = 1;
		if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)))
			return nullptr;

		struct sockaddr_in6 address{
			.sin6_family = AF_INET6,
			.sin6_port = htons(port),
			.sin6_flowinfo = 0,
			.sin6_addr = in6addr_any,
			.sin6_scope_id = 0
		};

		if (bind(
			fd, (struct sockaddr *)&address,
			sizeof(struct sockaddr_in6)
		))
			return nullptr;

		if (listen(fd, backlog))
			return nullptr;

		return make_shared<Server>(fd, separator, on_accept, on_error);
	}

	virtual bool awaits_input() const override { return true; }
	virtual std::shared_ptr<Channel> handle_input() override {
		int connection_fd = accept(fd, NULL, 0);
		if (connection_fd < 0)
			return nullptr;

		std::shared_ptr<Connection> connection = Connection::from_fd(
			connection_fd,
			separator
		);

		if (!connection)
			return nullptr;

		on_accept(connection);

		return connection;
	}
};

};

#endif
