#pragma once

#include <cstdint>
#include <vector>

class Labeler {

	private:

		std::vector<uint16_t> targets;

	public:

		/// Check if there is any label targeting this address
		bool has(uint16_t target) const;

		/// Add label target
		void add(uint16_t target);

};
