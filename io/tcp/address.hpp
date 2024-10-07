#ifndef IO_TCP_ADDRESS_HPP
#define IO_TCP_ADDRESS_HPP 1

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <optional>
#include <sys/socket.h>
#include <utility>

namespace io::tcp {

class Address {
	const struct sockaddr_storage sockaddr;

	static struct sockaddr_storage construct(
		const struct in_addr &addr, in_port_t port
	) {
		struct sockaddr_storage result;
		struct sockaddr_in *result4 = (struct sockaddr_in *)&result;

		memset(result4, 0, sizeof(struct sockaddr_storage));
		result4->sin_family = AF_INET;
		result4->sin_port = htons(port);
		memmove(&result4->sin_addr, &addr, sizeof(struct in_addr));

		return result;
	}

	static struct sockaddr_storage construct(
		const struct in6_addr &addr, in_port_t port
	) {
		struct sockaddr_storage result;
		struct sockaddr_in6 *result6 = (struct sockaddr_in6 *)&result;

		memset(result6, 0, sizeof(struct sockaddr_storage));
		result6->sin6_family = AF_INET6;
		result6->sin6_port = htons(port);
		memmove(&result6->sin6_addr, &addr, sizeof(struct in6_addr));

		return result;
	}

public:
	Address(const auto &addr, in_port_t port)
	: sockaddr(construct(addr, port)) {}

	operator std::string() const {
		int family = sockaddr.ss_family == AF_INET
			? AF_INET
			: AF_INET6;

		std::string host(
			family == AF_INET
			? INET_ADDRSTRLEN
			: INET6_ADDRSTRLEN,
			0
		);

		auto *sockaddr4 = (const struct sockaddr_in  *)&sockaddr;
		auto *sockaddr6 = (const struct sockaddr_in6 *)&sockaddr;

		const void *addr = family == AF_INET
			? (const void *) &sockaddr4->sin_addr
			: (const void *) &sockaddr6->sin6_addr;

		int port = family == AF_INET
			? sockaddr4->sin_port
			: sockaddr6->sin6_port;

		inet_ntop(
			family, addr,
			&host[0], host.size()
		);

		host.resize(host.find('\0'));

		return	host + ":" + std::to_string(port);
	}

	operator struct sockaddr *() const {
		return (struct sockaddr *)&sockaddr;
	}

	static std::optional<Address> from_address(
		int domain,
		const std::string &host,
		int port
	) {
		domain = domain == AF_INET
			? AF_INET
			: domain == AF_INET6
			? AF_INET6
			: AF_UNSPEC;

		struct addrinfo hints;
		struct addrinfo *result;
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = domain;

		if (getaddrinfo(&host[0], NULL, &hints, &result) != 0) {
			return std::nullopt;
		}

		auto *sockaddr4 = (const struct sockaddr_in  *)result->ai_addr;
		auto *sockaddr6 = (const struct sockaddr_in6 *)result->ai_addr;

		return result->ai_family == AF_INET
			? Address(sockaddr4->sin_addr, port)
			: Address(sockaddr6->sin6_addr, port);
	}

	static std::pair<int, Address> from_syscall(
		int (*syscall)(int, struct sockaddr *, socklen_t *),
		int fd
	) {
		struct sockaddr_storage sockaddr;
		socklen_t socklen = sizeof(sockaddr);
	
		int result = syscall(
			fd, (struct sockaddr *)&sockaddr, &socklen
		);

		auto *sockaddr4 = (const struct sockaddr_in  *)&sockaddr;
		auto *sockaddr6 = (const struct sockaddr_in6 *)&sockaddr;

		return {
			result,
			sockaddr.ss_family == AF_INET
				? Address(sockaddr4->sin_addr,
					sockaddr4->sin_port)
				: Address(sockaddr6->sin6_addr,
					sockaddr6->sin6_port)
		};
	}

	static std::pair<int, Address> from_getpeername(int fd) {
		return from_syscall(getpeername, fd);
	}

	static std::pair<int, Address> from_getsockname(int fd) {
		return from_syscall(getsockname, fd);
	}

	int family() const { return sockaddr.ss_family; }
	int size() const {
		return sockaddr.ss_family == AF_INET
			? sizeof(struct sockaddr_in)
			: sockaddr.ss_family == AF_INET6
			? sizeof(struct sockaddr_in6)
			: sizeof(sockaddr);
	}
};

}

#endif
