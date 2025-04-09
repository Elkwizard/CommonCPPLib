#pragma once

namespace math {
	float smoothT(float t) {
		return -2 * t * t * t + 3 * t * t;
	}

	template <typename T>
	T lerp(const T& a, const T& b, float t) {
		return a * (1 - t) + b * t;
	}

	template <typename T>
	T slerp(const T& a, const T& b, float t) {
		return lerp(a, b, smoothT(t));
	}

	template <typename T>
	T quadLerp(const T& a, const T& b, const T& c, const T& d, float tx, float ty) {
		T l = lerp(a, c, ty);
		T r = lerp(b, d, ty);
		float per = lerp(l, r, tx);
		return per;
	}

	template <typename T>
	T squadLerp(const T& a, const T& b, const T& c, const T& d, float tx, float ty) {
		return quadLerp(a, b, c, d, smoothT(tx), smoothT(ty));
	}

	template <typename T>
	T cubeLerp(const T& a, const T& b, const T& c, const T& d, const T& a2, const T& b2, const T& c2, const T& d2, float tx, float ty, float tz) {
		T top = quadLerp(a, b, c, d, tx, ty);
		T bottom = quadLerp(a2, b2, c2, d2, tx, ty);
		return lerp(top, bottom, tz);
	}
	
	template <typename T>
	T scubeLerp(const T& a, const T& b, const T& c, const T& d, const T& a2, const T& b2, const T& c2, const T& d2, float tx, float ty, float tz) {
		return cubeLerp(a, b, c, d, a2, b2, c2, d2, smoothT(tx), smoothT(ty), smoothT(tz));
	}
}