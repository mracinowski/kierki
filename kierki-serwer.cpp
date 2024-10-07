#include "common.hpp"
#include "config.hpp"
#include "io/broker.hpp"
#include "io/tcp/server.hpp"
#include "engines/local.hpp"
#include "players/remote.hpp"

#include <chrono>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>

using namespace kierki;

std::tuple<uint16_t, std::string, std::chrono::seconds> parse_args(
	int argc, char *argv[]
) {
	uint16_t port = 0;
	std::optional<std::string> filename;
	std::chrono::seconds timeout(DEFAULT_TIMEOUT);

	for (int i = 1; i < argc; i++) {
		std::string option(argv[i]);

		if (option.length() != 2 || option.front() != '-')
			throw ParameterError(
				std::string(INVALID_OPTION) + " " + option
			);

		else if (option == "-p")
			if (i + 1 >= argc)
				throw ParameterError(INVALID_PORT);
			else if (
				auto value = to_number<uint16_t>(argv[++i]);
				!value
			)
				throw ParameterError(INVALID_PORT);
			else
				port = value.value();

		else if (option == "-f")
			if (i + 1 >= argc)
				throw ParameterError(INVALID_FILENAME);
			else
				filename = std::string(argv[++i]);

		else if (option == "-t")
			if (i + 1 >= argc)
				throw ParameterError(INVALID_TIMEOUT);
			else if (
				auto value =
					to_number<std::chrono::seconds::rep>(
						argv[++i]);
				!value
			)
				throw ParameterError(INVALID_TIMEOUT);
			else
				timeout = std::chrono::seconds(value.value());

		else
			throw ParameterError(
				std::string(UNKNOWN_OPTION) + " " + option
			);
	}

	if (!filename) throw ParameterError(MISSING_FILENAME);

	return {port, filename.value(), timeout};
}

int main(int argc, char *argv[]) {
	int result = 1;

	std::optional<std::tuple<
		uint16_t, std::string, std::chrono::seconds
	>> params;

	try {
		params = parse_args(argc, argv);
	} catch (const ParameterError &e) {
		ERROR_STREAM << e.what() << std::endl;
		return 1;
	}

	const auto [port, filename, timeout] = params.value();

	std::signal(SIGPIPE, SIG_IGN);

	std::ifstream file(filename);
	if (!file.is_open()) {
		ERROR_STREAM << FILE_NOT_OPENED << " " << filename
			<< std::endl;
		return 1;
	}

	std::shared_ptr<engines::Engine> engine;

	std::shared_ptr<io::Output> report =
		std::make_shared<io::Output>(1, PROTOCOL_ENDLINE);

	std::shared_ptr<io::tcp::Server> server = io::tcp::Server::build(
		PROTOCOL_ENDLINE, port, [&engine, report, timeout](
			const std::shared_ptr<io::tcp::Connection> &connection
		) {
			players::Remote::from_io(
				engine,
				connection,
				connection,
				report,
				timeout
			);
		}, [](int) {}
	);

	if (!server) {
		ERROR_STREAM << UNKNOWN_IO_ERROR << " "
			<< strerror(errno) << std::endl;

		report->close();
		return 1;
	}

	engine = engines::Local::parse(file, [report, server, &result](){
		server->close();
		report->close();
		result = 0;
	});

	std::shared_ptr<io::Broker> broker = io::Broker::build();
	broker->attach(report);
	broker->attach(server);
	broker->run();

	return result;
}
