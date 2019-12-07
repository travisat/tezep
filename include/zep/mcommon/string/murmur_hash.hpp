#pragma once

#include <cstdint>

auto murmur_hash(const void * key, int len, uint32_t seed) -> uint32_t;
auto murmur_hash_64(const void * key, uint32_t len, uint64_t seed) -> uint64_t;
