#pragma once

#include "../util/math.hpp"

#include <string>

namespace gl {
	struct HalfFloat {
		private:
			short f16 = 0;
		
		public:
			HalfFloat() { }

			HalfFloat(float f32) {
				int i32;
				memcpy(&i32, &f32, 4);

				int mantissa = i32 & 0b000000000011111111111111111111111;
				int exponent = ((i32 >> 23) & 0xFF) - 127;
				int sign = i32 >> 31; 

				if (f32 != 0.0f) {
					f16 = 0;
					short exp = util::clamp(exponent, -14, 15) + 15;
					f16 |= sign << 15;
					f16 |= exp << 10;
					f16 |= mantissa >> 13;	
				}
			}

			bool operator ==(const HalfFloat& other) const {
				return f16 == other.f16;
			}
			
			bool operator !=(const HalfFloat& other) const {
				return f16 != other.f16;
			}

			operator bool() const {
				return f16;
			}

			operator float() const {
				if (f16 == 0) return 0.0f;

				int sign = f16 >> 15;
				int exponent = ((f16 >> 10) & 0b11111) - 15;
				int mantissa = f16 & 0b0000001111111111;
				int i32 = 0;
				int exp = exponent + 127;
				i32 |= sign << 31;
				i32 |= exp << 23;
				i32 |= mantissa << 13;

				float f32;
				memcpy(&f32, &i32, 4);
				return f32;
			}

			bool operator <(float other) const {
				return (float)(*this) < other;
			}
			
			bool operator >(float other) const {
				return (float)(*this) > other;
			}
			
			bool operator <=(float other) const {
				return (float)(*this) <= other;
			}
			
			bool operator >=(float other) const {
				return (float)(*this) >= other;
			}
			
			HalfFloat& operator +=(float other) {
				return *this = (float)(*this) + other;
			}

			HalfFloat operator +(float other) const {
				return HalfFloat(*this) += other;
			}
			
			HalfFloat& operator -=(float other) {
				return *this = (float)(*this) - other;
			}

			HalfFloat operator -(float other) const {
				return HalfFloat(*this) -= other;
			}
			
			HalfFloat& operator *=(float other) {
				return *this = (float)(*this) * other;
			}

			HalfFloat operator *(float other) const {
				return HalfFloat(*this) *= other;
			}

			HalfFloat& operator /=(float other) {
				return *this = (float)(*this) / other;
			}

			HalfFloat operator /(float other) const {
				return HalfFloat(*this) /= other;
			}
	};

	template <typename T>
	class BaseColor {
		private:
			using Color = BaseColor<T>;

		public:
			using Channel = T;

			T r, g, b, a;
			
			BaseColor() {
				r = 0.0f;
				g = 0.0f;
				b = 0.0f;
				a = 0.0f;
			}

			BaseColor(int hex) {
				r = (hex & 0xFF) / 255.0f;
				g = ((hex >> 8) & 0xFF) / 255.0f;
				b = (hex >> 16) / 255.0f;
				a = 1.0f;
			}

			BaseColor(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255) {
				r = _r / 255.0f;
				g = _g / 255.0f;
				b = _b / 255.0f;
				a = _a / 255.0f;
			}

			BaseColor(int _r, int _g, int _b, int _a = 255) {
				r = _r / 255.0f;
				g = _g / 255.0f;
				b = _b / 255.0f;
				a = _a / 255.0f;
			}

			BaseColor(T _r, T _g, T _b, T _a = 1.0f) {
				r = _r;
				g = _g;
				b = _b;
				a = _a;
			}

			template <typename O>
			BaseColor(const BaseColor<O>& other) {
				r = (T)other.r;
				g = (T)other.g;
				b = (T)other.b;
				a = (T)other.a;
			}

			#define COLOR_OP(op) \
			Color& operator op##=(const Color& o) { \
				r op##= o.r; \
				g op##= o.g; \
				b op##= o.b; \
				return *this; \
			} \
			Color& operator op##=(float o) { \
				r op##= o; \
				g op##= o; \
				b op##= o; \
				return *this; \
			} \
			template <typename O> \
			Color operator op(const O& other) const { \
				return Color(*this) op##= other; \
			}

			COLOR_OP(+)
			COLOR_OP(-)
			COLOR_OP(*)
			COLOR_OP(/)

			#undef COLOR_OP

			operator bool() const {
				return a;
			}

			void operator =(const std::nullptr_t&) {
				a = 0.0f;
			}

			template <typename O>
			bool operator==(const BaseColor<O>& other) {
				return (!other.a && !a) || (other.a == a && other.r == r && other.g == g && other.b == b);
			}

			template <typename O>
			bool operator!=(const BaseColor<O>& other) {
				return !operator==(other);
			}

			template <typename O>
			operator BaseColor<O>() const {
				return { (O)r, (O)g, (O)b, (O)a };
			}
	};

	template <typename T>
	std::ostream operator <<(std::ostream& out, const BaseColor<T>& color) {
		out << "rgba(" << (float)color.r << ", " << (float)color.g << ", " << (float)color.b << ", " << (float)color.a << ")";
	}

	using Color = BaseColor<HalfFloat>;
	using FullColor = BaseColor<float>;
}