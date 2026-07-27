#pragma once

#include <concepts>
#include "util.hpp"

namespace math {
	class Random {
		public:
			using Seed = long;
			
		private:
			struct VoronoiCell {
				float x = 0.0f, y = 0.0f, z = 0.0f;
			};
			float s_p1D(long x, Seed seed);
			float s_p2D(long x, long y, Seed seed);
			float s_p3D(long x, long y, long z, Seed seed);
			VoronoiCell getVoronoiCell(float x);
			VoronoiCell getVoronoiCell2D(float x, float y);
			VoronoiCell getVoronoiCell3D(float x, float y, float z);

		public:
			enum Distribution { UNIFORM, NORMAL };
			Distribution distribution = UNIFORM;

			long lfsr = 1;
			Seed seed, sampleSeed;

			Random(Seed _seed = 0, Seed _sampleSeed = 0);

			template <std::same_as<float>... floats>
			float octave(Seed seed, int octaves, float (*alg)(float, floats...), float frequency, floats ...samples) {
				float total = 0.0f;
				float scale = 0.0f;
				
				for (int i = 1; i <= octaves; i++) {
					float scl = 1.0f / i;
					total += scl * alg(samples..., frequency * i, seed);
					scale += scl;
				}

				return total / scale;
			}
			
			template <std::same_as<float>... floats>
			float octave(int octaves, float (*alg)(float, floats...), float frequency, floats ...samples) {
				return octave(sampleSeed, octaves, alg, frequency, samples...);
			}

			template <typename T>
			auto& choice(const T& container) {
				return container[index(container.size())];
			}

			float seedRand(Seed seed);
			float random();
			float range(float min, float max);
			uint index(uint length);
			uint fastIndex(uint length);
			int intRange(int min, int max);
			float angle();
			bool chance(float chance = 0.5f);
			bool fastChance();
			int sign();
			float perlin(float x, float f);
			float perlin(float x, float f, Seed seed);
			float perlin2D(float x, float y, float f);
			float perlin2D(float x, float y, float f, Seed seed);
			float perlin3D(float x, float y, float z, float f);
			float perlin3D(float x, float y, float z, float f, Seed seed);
			float voronoi(float x, float f);
			float voronoi(float x, float f, Seed seed);
			float voronoi2D(float x, float y, float f);
			float voronoi2D(float x, float y, float f, Seed seed);
			float voronoi3D(float x, float y, float z, float f);
			float voronoi3D(float x, float y, float z, float f, Seed seed);
	};

	extern Random random;
}