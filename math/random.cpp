#include "random.hpp"
#include "interpolation.hpp"

#include <cmath>
#include <cassert>
#include <concepts>

namespace math {
	Random random;

	Random::Random(Seed _seed, Seed _sampleSeed) {
		seed = _seed;
		sampleSeed = _sampleSeed;
	}
	
	float Random::seedRand(Seed seed) {
		seed ^= seed << 7;
		seed ^= seed >> 9;
		seed ^= seed << 13;
		seed -= 192834821;
		seed ^= seed << 7;
		seed ^= seed >> 9;
		seed ^= seed << 13;
		seed = abs(seed);
		
		float result = seed / 65e3f;
		return result - (long)result;
		// double a = fmod((double)seed * 6.12849, 8.7890975);
		// double b = fmod(a * 256783945.4758903, 238462.567890);
		// float r = fmod(a * b, 1.0);
		// return r;
	}

	float Random::random() {
		seed++;
		return seedRand(seed);
	}

	float Random::range(float min, float max) {
		return random() * (max - min) + min;
	}

	uint Random::index(uint length) {
		seed++;
		
		unsigned long s = seed;
		s ^= s << 7;
		s ^= s >> 9;
		s ^= s << 13;
		s -= 192834821;
		s ^= s << 7;
		s ^= s >> 9;
		s ^= s << 13;
		return s % length;
	} 

	uint Random::fastIndex(uint length) {
		seed++;

		unsigned long s = seed;
		s ^= s << 7;
		s ^= s >> 9;
		s ^= s << 13;
		s -= 192834821;
		s ^= s << 7;
		s ^= s >> 9;
		
		return s % length;
	}
	
	int Random::intRange(int min, int max) {
		return min + (int)index(max - min + 1);
	}

	float Random::angle() {
		return range(0, 2 * PI);
	}

	bool Random::chance(float chance) {
		if (chance == 0.0f) return false;
		return random() < chance;
	}
	
	bool Random::fastChance() {
		lfsr ^= lfsr << 7;
		lfsr ^= lfsr >> 9;
		// lfsr ^= lfsr << 13;
		return lfsr % 2;
	}

	int Random::sign() {
		return chance() ? 1 : -1;
	}

	float Random::s_p1D(long x, Seed seed) {
		return seedRand(x ^ seed << 10);
	}

	float Random::perlin(float x, float f) {
		return perlin(x, f, sampleSeed);
	}

	float Random::perlin(float x, float f, Seed seed) {
		x *= f;
		long ix = (long)floor(x);
		return lerp(
			s_p1D(ix, seed),
			s_p1D(ix + 1, seed),
			x - ix
		);
	}

	float Random::s_p2D(long x, long y, Seed seed) {
		return seedRand(x ^ y << 10 ^ seed << 20);
	}

	float Random::perlin2D(float x, float y, float f) {
		return perlin2D(x, y, f, sampleSeed);
	}

	float Random::perlin2D(float x, float y, float f, Seed seed) {
		x *= f;
		y *= f;
		long ix = (long)floor(x);
		long iy = (long)floor(y);
		return squadLerp(
			s_p2D(ix, iy, seed),
			s_p2D(ix + 1, iy, seed),
			s_p2D(ix, iy + 1, seed),
			s_p2D(ix + 1, iy + 1, seed),
			x - ix, y - iy
		);
	}

	float Random::s_p3D(long x, long y, long z, Seed seed) {
		return seedRand(x ^ y << 10 ^ z << 20 ^ seed << 30);
	}

	float Random::perlin3D(float x, float y, float z, float f) {
		return perlin3D(x, y, z, f, sampleSeed);
	}

	float Random::perlin3D(float x, float y, float z, float f, Seed seed) {
		x *= f;
		y *= f;
		z *= f;
		long ix = (long)floor(x);
		long iy = (long)floor(y);
		long iz = (long)floor(z);
		return scubeLerp(
			s_p3D(ix, iy, iz, seed),
			s_p3D(ix + 1, iy, iz, seed),
			s_p3D(ix, iy + 1, iz, seed),
			s_p3D(ix + 1, iy + 1, iz, seed),
			s_p3D(ix, iy, iz + 1, seed),
			s_p3D(ix + 1, iy, iz + 1, seed),
			s_p3D(ix, iy + 1, iz + 1, seed),
			s_p3D(ix + 1, iy + 1, iz + 1, seed),
			x - ix, y - iy, z - iz
		);
	}

	Random::VoronoiCell Random::getVoronoiCell(float x) {
		return { floor(x) + seedRand(static_cast<Seed>(floor(x))) };
	}

	float Random::voronoi(float x, float f) {
		return voronoi(x, f, sampleSeed);
	}

	float Random::voronoi(float x, float f, Seed seed) {
		x *= f;
		x += seed;
		float bestDist = INFINITY;
		for (int i = -1; i < 2; i++) {
			VoronoiCell cell = getVoronoiCell(x + i);
			float dist = cell.x - x;
			dist = dist * dist;
			if (dist < bestDist) bestDist = dist;
		}
		return bestDist;
	}

	Random::VoronoiCell Random::getVoronoiCell2D(float x, float y) {
		return { 
			floor(x) + seedRand(static_cast<Seed>(floor(x) + floor(y) * 1000.0f)), 
			floor(y) + seedRand(static_cast<Seed>(floor(y) + floor(x) * 10000.0f))
		};
	}

	float Random::voronoi2D(float x, float y, float f) {
		return voronoi2D(x, y, f, sampleSeed);
	}

	float Random::voronoi2D(float x, float y, float f, Seed seed) {
		x *= f;
		y *= f;
		x += seed;
		y += seed * 2000.0f;
		float bestDist = INFINITY;
		for (int i = -1; i < 2; i++) for (int j = -1; j < 2; j++) {
			VoronoiCell cell = getVoronoiCell2D(x + i, y + j);
			float dx = cell.x - x;
			float dy = cell.y - y;
			float dist = dx * dx + dy * dy;
			if (dist < bestDist) bestDist = dist;
		}
		return bestDist;
	}

	Random::VoronoiCell Random::getVoronoiCell3D(float x, float y, float z) {
		return {
			floor(x) + seedRand(static_cast<Seed>(floor(x) + floor(y) * 1000.0f + floor(z) * 10000)),
			floor(y) + seedRand(static_cast<Seed>(floor(y) + floor(x) * 1000000.0f + floor(z) * 900.0f)),
			floor(z) + seedRand(static_cast<Seed>(floor(y) * 10000.0f + floor(x) * 100.0f + floor(z) * 90000.0f))
		};
	}

	float Random::voronoi3D(float x, float y, float z, float f) {
		return voronoi3D(x, y, z, f, sampleSeed);
	}

	float Random::voronoi3D(float x, float y, float z, float f, Seed seed) {
		x *= f;
		y *= f;
		z *= f;
		x += seed;
		y += seed * 2000.0f;
		z += seed * 2000000.0f;
		float bestDist = INFINITY;
		for (int i = -1; i < 2; i++) for (int j = -1; j < 2; j++) for (int k = -1; k < 2; k++) {
			VoronoiCell cell = getVoronoiCell3D(x + i, y + j, z + k);
			float dx = cell.x - x;
			float dy = cell.y - y;
			float dz = cell.z - z;
			float dist = dx * dx + dy * dy + dz * dz;
			if (dist < bestDist) bestDist = dist;
		}
		return bestDist;
	}
}