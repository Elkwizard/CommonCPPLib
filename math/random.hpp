#pragma once

#include "util.hpp"
#include "interpolation.hpp"
#include "../../Common/math/util.hpp"

#include <cmath>
#include <cassert>

namespace math {
	class Random {
		public:
			enum Distribution { UNIFORM, NORMAL };
			Distribution distribution = UNIFORM;

			long lfsr = 1;
			using Seed = long;
			Seed seed, sampleSeed;

			Random(Seed _seed = 0, Seed _sampleSeed = 0) {
				seed = _seed;
				sampleSeed = _sampleSeed;
			}

			float seedRand(Seed seed) {
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

			float random() {
				seed++;
				return seedRand(seed);
			}

			float range(float min, float max) {
				return random() * (max - min) + min;
			}

			uint index(uint length) {
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

			uint fastIndex(uint length) {
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
			
			int intRange(int min, int max) {
				return min + (int)index(max - min + 1);
			}

			float angle() {
				return range(0, 2 * PI);
			}

			bool chance(float chance = 0.5f) {
				if (chance == 0.0f) return false;
				return random() < chance;
			}
			
			bool fastChance() {
				lfsr ^= lfsr << 7;
				lfsr ^= lfsr >> 9;
				// lfsr ^= lfsr << 13;
				return lfsr % 2;
			}

			int sign() {
				return chance() ? 1 : -1;
			}

			template <std::same_as<float>... floats>
			float octave(int octaves, float (Random::*alg)(float, floats...), float frequency, floats ...samples) {
				float total = 0.0f;
				float scale = 0.0f;
				for (int i = 1; i <= octaves; i++) {
					float scl = 1.0f / i;
					total += scl * (this->*alg)(samples..., frequency * i);
					scale += scl;
				}

				return total / scale;
			}

			template <std::same_as<float>... floats>
			float octave(Seed seed, int octaves, float (Random::*alg)(float, floats...), float frequency, floats ...samples) {
				float total = 0.0f;
				float scale = 0.0f;
				
				for (int i = 1; i <= octaves; i++) {
					float scl = 1.0f / i;
					total += scl * alg(samples..., frequency * i, seed);
					scale += scl;
				}

				return total / scale;
			}

			float s_p1D(long x, Seed seed) {
				return seedRand(x ^ seed << 10);
			}

			float perlin(float x, float f) {
				return perlin(x, f, sampleSeed);
			}

			float perlin(float x, float f, Seed seed) {
				x *= f;
				long ix = (long)floor(x);
				return lerp(
					s_p1D(ix, seed),
					s_p1D(ix + 1, seed),
					x - ix
				);
			}

			float s_p2D(long x, long y, Seed seed) {
				return seedRand(x ^ y << 10 ^ seed << 20);
			}

			float perlin2D(float x, float y, float f) {
				return perlin2D(x, y, f, sampleSeed);
			}

			float perlin2D(float x, float y, float f, Seed seed) {
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

			float s_p3D (long x, long y, long z, Seed seed) {
				return seedRand(x ^ y << 10 ^ z << 20 ^ seed << 30);
			}

			float perlin3D(float x, float y, float z, float f) {
				return perlin3D(x, y, z, f, sampleSeed);
			}

			float perlin3D(float x, float y, float z, float f, Seed seed) {
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

			struct VoronoiCell {
				float x = 0.0f, y = 0.0f, z = 0.0f;
			};

			VoronoiCell getVoronoiCell(float x) {
				return { floor(x) + seedRand(floor(x)) };
			}

			float voronoi(float x, float f) {
				return voronoi(x, f, sampleSeed);
			}

			float voronoi(float x, float f, Seed seed) {
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

			VoronoiCell getVoronoiCell2D(float x, float y) {
				return { 
					floor(x) + seedRand(floor(x) + floor(y) * 1000.0f), 
					floor(y) + seedRand(floor(y) + floor(x) * 10000.0f),
				};
			}

			float voronoi2D(float x, float y, float f) {
				return voronoi2D(x, y, f, sampleSeed);
			}

			float voronoi2D(float x, float y, float f, Seed seed) {
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

			VoronoiCell getVoronoiCell3D(float x, float y, float z) {
				return {
					floor(x) + seedRand(floor(x) + floor(y) * 1000.0f + floor(z) * 10000),
					floor(y) + seedRand(floor(y) + floor(x) * 1000000.0f + floor(z) * 900.0f),
					floor(z) + seedRand(floor(y) * 10000.0f + floor(x) * 100.0f + floor(z) * 90000.0f)
				};
			}

			float voronoi3D(float x, float y, float z, float f) {
				return voronoi3D(x, y, z, f, sampleSeed);
			}

			float voronoi3D(float x, float y, float z, float f, Seed seed) {
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
	} random;
}