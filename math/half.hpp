#pragma once

#include <string.h>
#include "util.hpp"

namespace math {
	class HalfFloat {
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
					short exp = static_cast<short>(clamp(exponent, -14, 15) + 15);
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
}