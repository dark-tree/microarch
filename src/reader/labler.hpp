#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

class Labler {

	private:

		std::vector<uint16_t> targets;

	public:

		bool has(uint16_t target) const;

		void add(uint16_t target);

};
