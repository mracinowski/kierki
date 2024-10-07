#ifndef DEBUG_HPP
#define DEBUG_HPP 1

#ifdef KIERKI_DEBUG

#include <cstring>
#include <iostream>

#define BEFORE_POLL(FDS, TIMEOUT) do { \
	std::clog << "\033[94m" << "poll(["; \
	int i = 0; \
	for (const struct pollfd &pollfd: FDS) \
		std::clog << (i++ ? ", " : "") \
			<< "{fd = " << pollfd.fd << ", " \
			<< "events = " << pollfd.events << "}"; \
	std::clog << "], " << TIMEOUT << ")\033[0m" << std::flush; \
} while (0)

#define AFTER_POLL(FDS, RESULT) do { \
	std::clog << "\033[94m" << " = " << (RESULT) << " ["; \
	int i = 0; \
	for (const struct pollfd &pollfd: (FDS)) \
		std::clog << (i++ ? ", " : "") \
			<< "{fd = " << pollfd.fd << ", " \
			<< "revents = " << pollfd.revents << "}"; \
	std::clog << "]\033[0m" << std::endl; \
} while (0)

#define READ(FD, BUF, COUNT, RESULT) do { \
	std::clog << "\033[91m" << "read(" \
		<< (FD) << ", " \
		<< "\033[90m\""; \
	for (char c: std::string((BUF), (RESULT) < 0 ? 0 : (RESULT))) { \
		if (32 <= c && c <= 126) std::clog << c; \
		else if (c == '\r') std::clog << "\\r"; \
		else if (c == '\n') std::clog << "\\n"; \
		else std::clog << '?'; \
	} \
	std::clog << "\"\033[91m, " \
		<< (COUNT) << ") = " \
		<< (RESULT) << " " \
		<< "(" << strerror(errno) << ")" \
		<< "\033[0m" << std::endl; \
} while (0)

#define WRITE(FD, BUF, COUNT, RESULT) do { \
	std::clog << "\033[92m" << "write(" \
		<< (FD) << ", " \
		<< "\033[90m\""; \
	for (char c: std::string((BUF), (COUNT))) { \
		if (32 <= c && c <= 126) std::clog << c; \
		else if (c == '\r') std::clog << "\\r"; \
		else if (c == '\n') std::clog << "\\n"; \
		else std::clog << '?'; \
	} \
	std::clog << "\"\033[92m, " \
		<< (COUNT) << ") = " \
		<< (RESULT) << " " \
		<< "(" << strerror(errno) << ")" \
		<< "\033[0m" << std::endl; \
} while (0)

#else

#define BEFORE_POLL(FDS, TIMEOUT)
#define AFTER_POLL(FDS, RESULT)
#define READ(FD, BUF, COUNT, RESULT)
#define WRITE(FD, BUF, COUNT, RESULT)

#endif

#endif
