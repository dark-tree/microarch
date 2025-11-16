#include "labler.hpp"


bool Labler::has(uint16_t target) const {
	return std::find(targets.begin(), targets.end(), target) != targets.end();
}

void Labler::add(uint16_t target) {
	if (!has(target)) targets.push_back(target);
}