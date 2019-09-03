#pragma once
#include <optional>
#include <functional>
#include <string>
#include <optional>

#include "caf/all.hpp"

namespace string_util
{
	struct splitter
	{
		enum empties_t { empties_ok, no_empties };
	};

	template <typename Container>
	Container& split(
		Container& result,
		const typename Container::value_type& s,
		const typename Container::value_type& delimiters,
		splitter::empties_t empties = splitter::empties_ok)
	{
		result.clear();
		size_t current;
		size_t next = -1;
		do
		{
			if (empties == splitter::no_empties)
			{
				next = s.find_first_not_of(delimiters, next + 1);
				if (next == Container::value_type::npos) break;
				next -= 1;
			}
			current = next + 1;
			next = s.find_first_of(delimiters, current);
			result.push_back(s.substr(current, next - current));
		} while (next != Container::value_type::npos);
		return result;
	}

	// trim leading & trailing spaces
	std::string trim(std::string s);

	// convert string to int
	// tries to convert `str` to an int
	std::optional<int> to_integer(const std::string& str);

}

