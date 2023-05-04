#pragma once

#include "Hash.hpp"
#include "Core\Time.hpp"

static inline U64 Random(U64 seed)
{
	gettimeofday(&t, 0);
	U64	teed = (((U64)t.tv_sec) << 32) | t.tv_usec;
	teed = Mix(teed ^ secret0, seed ^ secret1);
	seed = Mix(teed ^ secret0, secret2);
	return Mix(seed, seed ^ secret3);
}

//a useful 64bit-64bit mix function to produce deterministic pseudo random numbers that can pass BigCrush and PractRand
static inline U64 wyhash64(U64 A, U64 B) { A ^= secret0; B ^= secret1; Multiply(A, B); return Mix(A ^ secret0, B ^ secret1); }

//The wyrand PRNG that pass BigCrush and PractRand
static inline U64 wyrand(U64* seed) { *seed += secret0; return Mix(*seed, *seed ^ secret1); }

//convert any 64 bit pseudo random numbers to uniform distribution [0,1). It can be combined with wyrand, wyhash64 or wyhash.
static inline double wy2u01(U64 r) { const double _wynorm = 1.0 / (1ull << 52); return (r >> 12) * _wynorm; }

//convert any 64 bit pseudo random numbers to APPROXIMATE Gaussian distribution. It can be combined with wyrand, wyhash64 or wyhash.
static inline double wy2gau(U64 r) { const double _wynorm = 1.0 / (1ull << 20); return ((r & 0x1fffff) + ((r >> 21) & 0x1fffff) + ((r >> 42) & 0x1fffff)) * _wynorm - 3.0; }

//fast range integer random number generation on [0,k) credit to Daniel Lemire. May not work when WYHASH_32BIT_MUM=1. It can be combined with wyrand, wyhash64 or wyhash.
static inline U64 wy2u0k(U64 r, U64 k) { Multiply(r, k); return k; }