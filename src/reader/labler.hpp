#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

class Labelr {

	private:

		std::vector<uint16_t> targets;

	public:

		bool has(uint16_t target) const {
			return std::find(targets.begin(), targets.end(), target) != targets.end();
		}

		void add(uint16_t target) {
			if (!has(target)) targets.push_back(target);
		}

};
