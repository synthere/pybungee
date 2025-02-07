// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"

#include <type_traits>

namespace Bungee {
static inline int ctz (int x) {
#ifdef _MSC_VER
	#ifdef _WIN64
			unsigned long ret;
			  _BitScanForward64(&ret, x);
			  return (int)ret;
		#else
				unsigned long ret;
			  _BitScanForward(&ret, x);
			  return (int)ret;
		#endif
  
#else
  // Assume long long is 64 bits
  return __builtin_ctzll (x);
#endif
}
static inline int clz (int x) {
#ifdef _MSC_VER
  #ifdef _WIN64
			unsigned long ret;
			  _BitScanReverse64(&ret, x);
			  return 63 - (int)ret;
		#else
				unsigned long ret;
			  _BitScanReverse(&ret, x);
			  return 63 - (int)ret;
		#endif
  
  
#else
  return __builtin_clzll (x);
#endif
}

template <bool floor = false>
static inline int log2(int x)
{
	BUNGEE_ASSERT1(x > 0);
	BUNGEE_ASSERT1(floor || !(x & (x << 1)));

	int y;
	if constexpr (floor)
		y = clz(1) - clz(x);
	else
		y = ctz(x);

	BUNGEE_ASSERT1(floor ? (1 << y <= x && x < 2 << y) : (x == 1 << y));
	return y;
}

template <int x>
constexpr int log2(std::integral_constant<int, x>)
{
	static_assert(x > 0);
	static_assert(!(x & (x - 1)));

	if constexpr (x == 1)
		return 0;
	else
		return 1 + log2(std::integral_constant<int, x / 2>{});
}

} // namespace Bungee
