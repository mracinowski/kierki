#include "config.hpp"
#include "common.hpp"
#include "io/broker.hpp"
#include "io/tcp/client.hpp"
#include "engines/remote.hpp"
#include "players/automatic.hpp"
#include "players/interactive.hpp"

#include <csignal>
#include <cstring>
#include <iostream>

using namespace kierki;

std::tuple<std::string, uint16_t, int, Seat, bool> parse_args(
	int argc, char *argv[]
) {
	std::optional<std::string> host;
	std::optional<int> port;
	std::optional<Seat> seat;
	int domain = AF_UNSPEC;
	bool automatic = false;

	for (int i = 1; i < argc; i++) {
		std::string option(argv[i]);

		if (option.length() != 2 || option.front() != '-')
			throw ParameterError(
				std::string(INVALID_OPTION) + " " + option
			);

		else if (option == "-h")
			if (i + 1 >= argc) 
				throw ParameterError(INVALID_HOST);
			else
				host = std::string(argv[++i]);

		else if (option == "-p")
			if (i + 1 >= argc)
				throw ParameterError(INVALID_PORT);
			else if (
				auto value = to_number<uint16_t>(argv[++i]);
				!value
			)
				throw ParameterError(INVALID_PORT);
			else if (value.value() == 0)
				throw ParameterError(INVALID_PORT);
			else
				port = value.value();

		else if (option == "-4") domain = AF_INET;
		else if (option == "-6") domain = AF_INET6;

		else if (option == "-N") seat = Seat::NORTH;
		else if (option == "-E") seat = Seat::EAST;
		else if (option == "-S") seat = Seat::SOUTH;
		else if (option == "-W") seat = Seat::WEST;

		else if (option == "-a") automatic = true;

		else throw ParameterError(
			std::string(UNKNOWN_OPTION) + " " + option
		);
	}

	if (!host) throw ParameterError(MISSING_HOST);
	if (!port) throw ParameterError(MISSING_PORT);
	if (!seat) throw ParameterError(MISSING_SEAT);

	return {host.value(), port.value(), domain, seat.value(), automatic};
}

std::shared_ptr<players::Player> automatic(
	std::shared_ptr<io::Broker> &broker,
	std::shared_ptr<io::Output> &report,
	const Seat &seat
) {
	report = std::make_shared<io::Output>(1, PROTOCOL_ENDLINE);
	broker->attach(report);

	return players::Automatic::from_seat(seat);

}

std::shared_ptr<players::Player> interactive(
	std::shared_ptr<io::Broker> &broker,
	const Seat &seat
) {
	auto input = std::make_shared<io::Input>(0, CLI_ENDLINE);
	broker->attach(input);

	auto output = std::make_shared<io::Output>(1, CLI_ENDLINE);
	broker->attach(output);

	return players::Interactive::from_io(seat, input, output);
}

int main(int argc, char *argv[]) {
	int result = 0;

	std::optional<std::tuple<std::string, int, int, Seat, bool>> params;
	try {
		params = parse_args(argc, argv);
	} catch (const ParameterError &e) {
		ERROR_STREAM << e.what() << std::endl;
		return 1;
	}

	const auto [host, port, domain, seat, type] = params.value();

	std::signal(SIGPIPE, SIG_IGN);

	std::shared_ptr<io::Broker> broker = io::Broker::build();

	std::shared_ptr<io::Output> report = nullptr;
	auto player = type
		? automatic(broker, report, seat)
		: interactive(broker, seat);

	auto client = io::tcp::Client::from_address(
		PROTOCOL_ENDLINE, domain, host, port,
		[player, report, &result](
			const std::shared_ptr<io::tcp::Connection> &connection
		) {
			player->join(engines::Remote::from_io(
				connection,
				connection,
				report,
				[&result](bool complete) {
					result = complete ? 0 : 1;
				}
			));
		},
		[ player, report, &result ](int error) {
			std::cout << CONNECTION_FAILED << " "
				<< strerror(error) << std::endl;

			if (report) report->close();
			player->close();
			result = 1;
		},
		[ player, report, &result ](int) {
			if (report) report->close();
			player->close();
			result = 1;
		}
	);

	if (!client) {
		if (errno == 0)
			ERROR_STREAM << UNRESOLVABLE_HOST << std::endl;
		else
			ERROR_STREAM << UNKNOWN_IO_ERROR << std::endl;

		if (report) report->close();
		if (player) player->close();
		return 1;
	}

	broker->attach(client);
	broker->run();

	return result;
}
