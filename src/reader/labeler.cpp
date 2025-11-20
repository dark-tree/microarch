#include "labeler.hpp"

#include <algorithm>

/*
 * class Labeler
 */

bool Labeler::has(uint16_t target) const {
	return std::find(targets.begin(), targets.end(), target) != targets.end();
}

void Labeler::add(uint16_t target) {
	if (!has(target)) targets.push_back(target);
}