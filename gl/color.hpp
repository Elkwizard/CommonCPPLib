#pragma once

#include "../math/util.hpp"
#include "../math/half.hpp"

#include <string>

namespace gl {
	template <typename T>
	class BaseColor {
		private:
			using Color = BaseColor<T>;

		public:
			using Channel = T;

			static constexpr int RED = 0xFF0000;
			static constexpr int GREEN = 0x00FF00;
			static constexpr int BLUE = 0x0000FF;
			static constexpr int YELLOW = 0xFFFF00;
			static constexpr int PURPLE = 0xFF00FF;
			static constexpr int CYAN = 0x00FFFF;
			static constexpr int ORANGE = 0xFF9900;
			static constexpr int BLACK = 0x000000;
			static constexpr int WHITE = 0xFFFFFF;

			T r, g, b, a;
			
			BaseColor() {
				r = 0.0f;
				g = 0.0f;
				b = 0.0f;
				a = 0.0f;
			}

			BaseColor(int hex) {
				r = (hex >> 16) / 255.0f;
				g = ((hex >> 8) & 0xFF) / 255.0f;
				b = (hex & 0xFF) / 255.0f;
				a = 1.0f;
			}

			BaseColor(int _r, int _g, int _b, float _a = 1.0f) {
				r = _r / 255.0f;
				g = _g / 255.0f;
				b = _b / 255.0f;
				a = _a;
			}

			BaseColor(float _r, float _g, float _b, float _a = 1.0f) {
				r = _r;
				g = _g;
				b = _b;
				a = _a;
			}

			template <typename O>
			BaseColor(const BaseColor<O>& other) {
				*this = other;
			}

			template <typename O>
			BaseColor& operator=(const BaseColor<O>& other) {
				r = (T)other.r;
				g = (T)other.g;
				b = (T)other.b;
				a = (T)other.a;
				return *this;
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
	std::ostream& operator <<(std::ostream& out, const BaseColor<T>& color) {
		out << "rgba(" << (float)color.r << ", " << (float)color.g << ", " << (float)color.b << ", " << (float)color.a << ")";
		return out;
	}

	using Color = BaseColor<math::HalfFloat>;
	using FullColor = BaseColor<float>;
}