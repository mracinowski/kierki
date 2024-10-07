#ifndef KIERKI_REPORT_HPP 
#define KIERKI_REPORT_HPP 1

#include <chrono>
#include <string>

namespace kierki {

class Report {
	static std::string timestamp() {
		const auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);
		std::tm *tm = std::localtime(&time);
		std::string ms = std::to_string(
			duration_cast<std::chrono::milliseconds>(
				now -
				std::chrono::floor<std::chrono::seconds>(now)
			).count()
		);

		std::string result(24, 0);
		std::strftime(
			&result[0], result.size(), "%Y-%m-%dT%H:%M:%S", tm
		);
		result.erase(result.find('\0'));
		result += "." + std::string(3 - ms.length(), '0') + ms;

		return result;
	}

public:
	static std::string format(
		const std::string &name,
		const std::string &message
	) {
		return "[" + name + "," + timestamp() + "] " + message;
	}
};

};

#endif
