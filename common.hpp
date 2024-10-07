#ifndef KIERKI_COMMON_HPP
#define KIERKI_COMMON_HPP 1

#include <stdexcept>
#include <optional>

namespace kierki {
	struct ParameterError: public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	template <class T>
	std::optional<T> to_number(const char *string) {
		unsigned long result = 0;

		for (const char *digit = string; *digit != '\0'; digit++) {
			if ('0' > *digit || *digit > '9')
				return std::nullopt;

			result *= 10;
			result += *digit - '0';

			if (result > std::numeric_limits<T>::max())
				return std::nullopt;
		}

		return T(result);
	}
}


#endif
