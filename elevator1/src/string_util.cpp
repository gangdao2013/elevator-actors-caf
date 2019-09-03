#include <string>
#include <optional>
#include <functional>
#include "string_util.hpp"

using namespace std;

namespace string_util
{

	// removes leading and trailing whitespaces
	string trim(std::string s)
	{
		auto not_space = [](char c) { return isspace(c) == 0; };
		// trim left
		s.erase(s.begin(), find_if(s.begin(), s.end(), not_space));
		// trim right
		s.erase(find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
		return s;
	}

	// tries to convert `str` to an int
	std::optional<int> to_integer(const string& str) {
		char* end;
		auto result = static_cast<int>(strtol(str.c_str(), &end, 10));
		if (end == str.c_str() + str.size())
			return result;
		return nullopt;
	}
}