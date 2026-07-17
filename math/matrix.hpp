#pragma once

#include "util.hpp"

#include <iostream>
#include <concepts>
#include <cmath>

namespace math {
	template <typename Self, Numeric T, size_t S>
	class BaseVectorLike {
		protected:
			std::array<T, S> m;

		public:
			static constexpr size_t size = S;

			BaseVectorLike() : BaseVectorLike(0) { }

			explicit BaseVectorLike(T arg) {
				for (size_t i = 0; i < S; i++)
					m[i] = arg;
			}

			template <typename... Args>
			BaseVectorLike(T first, T second, Args... args) : m{ first, second, (T)args... } { }

			BaseVectorLike(const Self& other) {
				for (size_t i = 0; i < S; i++)
					m[i] = other[i];
			}

			template <typename K, size_t S2> requires (S2 >= S)
			Self& operator =(const BaseVectorLike<K, T, S2>& other) {
				size_t size = min(S, S2);
				for (size_t i = 0; i < size; i++)
					m[i] = other[i];
				return static_cast<Self&>(*this);
			}

			bool operator ==(const Self& other) const {
				for (size_t i = 0; i < S; i++)
					if (m[i] != other.m[i]) return false;
				return true;
			}

#define VECTOR_OP(op) \
			Self& operator op##=(const Self& b) { \
				for (size_t i = 0; i < S; i++) \
					m[i] op##= b.m[i]; \
				return static_cast<Self&>(*this); \
			} \
			Self& operator op##=(T b) { \
				for (size_t i = 0; i < S; i++) \
					m[i] op##= b; \
				return static_cast<Self&>(*this); \
			} \
			template <typename O> \
			Self operator op(const O& other) const { \
				return Self(static_cast<const Self&>(*this)) op##= other; \
			}

			VECTOR_OP(+)
			VECTOR_OP(-)
			VECTOR_OP(*)
			VECTOR_OP(/)

#undef VECTOR_OP

			Self operator -() const {
				Self result = *this;
				for (size_t i = 0; i < S; i++)
					result[i] = -result[i];
				return result;
			}

			T& operator[](size_t inx) {
				return m[inx];
			}

			const T& operator[](size_t inx) const {
				return m[inx];
			}

			T dot(const Self& other) const {
				T sum = 0;
				for (size_t i = 0; i < S; i++)
					sum += m[i] * other[i];
				return sum;
			}
			
			T sqrMag() const {
				T sum = 0;
				for (size_t i = 0; i < S; i++)
					sum += m[i] * m[i];
				return sum;
			}

			auto cross(const Self& v) const {
				static_assert(S == 2 || S == 3, "Tried to cross invalid vector type");

				if constexpr (S == 2)
					return m[0] * v[1] - m[1] * v[0];
				if constexpr (S == 3)
					return Self(
						m[1] * v[2] - m[2] * v[1],
						m[2] * v[0] - m[0] * v[2],
						m[0] * v[1] - m[1] * v[0]
					);	
			}
	};

	template <typename K, Numeric T, size_t S>
	std::ostream& operator <<(std::ostream& out, const BaseVectorLike<K, T, S>& vec) {
		out << "{ ";
		for (size_t i = 0; i < S; i++)
			out << vec[i] << (i == S - 1 ? "" : ", ");
		out << " }";
		return out;
	}

	template <std::integral T, size_t S>
	class BaseCoordN : public BaseVectorLike<BaseCoordN<T, S>, T, S> { };

	using Coord2 = BaseCoordN<int, 2>;
	using Coord3 = BaseCoordN<int, 3>;

	template <std::floating_point T, size_t S>
	class BaseVectorN : public BaseVectorLike<BaseVectorN<T, S>, T, S> {
		private:
			using Vector = BaseVectorN<T, S>;

		public:
			using BaseVectorLike<Vector, T, S>::BaseVectorLike;

			T mag() const {
				return sqrt(this->sqrMag());
			}

			Vector& normalize() {
				return (*this) /= mag();
			}

			Vector normalized() const {
				return Vector(*this).normalize();
			}

			Vector projectOnto(const Vector& other) const {
				return other * (dot(other) / other.sqrMag());
			}

			Vector& orthogonalize(const Vector& other) {
				*this -= projectOnto(other);
				return *this;
			}

			Vector orthogonal(const Vector& other) const {
				return Vector(*this).orthogonalize(other);
			}

			template <std::integral... I>
			BaseVectorN<T, sizeof...(I)> swizzle(I... inxs) const {
				BaseVectorN<T, sizeof...(I)> result;
				size_t indices[] { inxs... };
				for (int i = 0; i < sizeof...(I); i++)
					result.m[i] = this->m[indices[i]];
				return result;
			}

			static void rotate(T& x, T& y, T angle) {
				T c = cos(angle);
				T s = sin(angle);
				T tx = x * c - y * s;
				T ty = x * s + y * c;
				x = tx;
				y = ty;
			}

			static T dist(const Vector& a, const Vector& b) {
				return (a - b).mag();
			}

			static T sqrDist(const Vector& a, const Vector& b) {
				return (a - b).sqrMag();
			}

			template <typename T2, size_t S2> requires (std::convertible_to<T, T2> && S2 <= S)
			operator BaseVectorN<T2, S2>() const {
				size_t copy = min(S, S2);
				BaseVectorN<T2, S2> result { };
				for (size_t i = 0; i < copy; i++)
					result[i] = (T2)this->m[i];
				return result;
			}
			
			template <typename I>
			static Vector avg(const I& begin, const I& end) {
				Vector sum;
				int count = 0;
				for (I it = begin; it != end; ++it) {
					sum += *it;
					count++;
				}
				return sum / count;
			}
	};

	const size_t X = 0;
	const size_t Y = 1;
	const size_t Z = 2;
	const size_t W = 3;

	template <typename K, Numeric T, size_t S>
	BaseVectorLike<K, T, S> operator +(T a, const BaseVectorLike<K, T, S>& b) {
		return b + a;
	}

	template <typename K, Numeric T, size_t S>
	BaseVectorLike<K, T, S> operator -(T a, const BaseVectorLike<K, T, S>& b) {
		return BaseVectorLike<K, T, S>(a) - b;
	}

	template <typename K, Numeric T, size_t S>
	BaseVectorLike<K, T, S> operator *(T a, const BaseVectorLike<K, T, S>& b) {
		return b * a;
	}

	template <typename K, Numeric T, size_t S>
	BaseVectorLike<K, T, S> operator /(T a, const BaseVectorLike<K, T, S>& b) {
		return BaseVectorLike<K, T, S>(a) / b;
	}

#define VECTOR_1_ARG_FN(fn) \
	using std::fn; \
	template <std::floating_point T, size_t S> \
	BaseVectorN<T, S> fn(const BaseVectorN<T, S>& v) { \
		BaseVectorN<T, S> result = v; \
		for (size_t i = 0; i < S; i++) \
			result[i] = fn(result[i]); \
		return result; \
	}
	
	VECTOR_1_ARG_FN(floor)
	VECTOR_1_ARG_FN(ceil)
	VECTOR_1_ARG_FN(round)
	VECTOR_1_ARG_FN(sqrt)
	VECTOR_1_ARG_FN(cbrt)
	VECTOR_1_ARG_FN(abs)
	VECTOR_1_ARG_FN(log2)
	VECTOR_1_ARG_FN(log10)
	VECTOR_1_ARG_FN(log)
	VECTOR_1_ARG_FN(sin)
	VECTOR_1_ARG_FN(cos)
	VECTOR_1_ARG_FN(tan)
	VECTOR_1_ARG_FN(asin)
	VECTOR_1_ARG_FN(acos)
	VECTOR_1_ARG_FN(atan)
	VECTOR_1_ARG_FN(exp)
	VECTOR_1_ARG_FN(exp2)

#undef VECTOR_1_ARG_FN

	template <typename K, Numeric T, size_t S>
	BaseVectorLike<K, T, S> min(const BaseVectorLike<K, T, S>& a, const BaseVectorLike<K, T, S>& b) {
		BaseVectorLike<K, T, S> result = a;
		for (size_t i = 0; i < S; i++)
			result[i] = min(result[i], b[i]);
		return result;
	}
	
	template <typename K, Numeric T, size_t S>
	BaseVectorLike<K, T, S> max(const BaseVectorLike<K, T, S>& a, const BaseVectorLike<K, T, S>& b) {
		BaseVectorLike<K, T, S> result = a;
		for (size_t i = 0; i < S; i++)
			result[i] = max(result[i], b[i]);
		return result;
	}

	template <typename T, size_t R, size_t C>
	union BaseMatrixN {
		private:
			using Matrix = BaseMatrixN<T, R, C>;
			using Vector = BaseVectorN<T, C>;
			using HVector = BaseVectorN<T, C - 1>;
			using Minor = BaseMatrixN<T, std::max(size_t{1}, R - 1), std::max(size_t{1}, C - 1)>;

			static Vector homogenous(const HVector& v) {
				Vector result = v;
				result[C - 1] = 1;
				return result;
			}

		public:
			static constexpr size_t rows = R;
			static constexpr size_t columns = C;

			std::array<std::array<T, R>, C> m;

			std::array<Vector, C> axes;
			
			Vector& operator [](size_t column) {
				return *(Vector*)(&m[column][0]);
			}

			const Vector& operator[](size_t column) const {
				return *(const Vector*)(&m[column][0]);
			}

			BaseMatrixN() : BaseMatrixN(0) {
				if constexpr (R == C) for (size_t i = 0; i < C; i++)
					m[i][i] = 1;
			}

			BaseMatrixN(T val) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] = val;
			}

			template <typename... Args>
			BaseMatrixN(T first, T second, Args... args) : m{ first, second, (T)args... } { }

			BaseMatrixN(const Matrix& other) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] = other.m[i][j];
			}
			
			Matrix& operator =(const Matrix& other) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] = other.m[i][j];
				return *this;
			}

			HVector homogenousMult(const HVector& v) const {
				Vector product = (*this) * homogenous(v);
				product /= product[C - 1];
				return product;
			}

			HVector operator *(const HVector& v) const {
				return (*this) * homogenous(v);
			}

			Vector operator *(const Vector& v) const {
				Vector result { };
				for (size_t i = 0; i < R; i++) {
					T sum = 0;
					for (size_t j = 0; j < C; j++)
						sum += m[j][i] * v[j];
					result[i] = sum;
				}
				return result;
			}

			template <size_t R2, size_t C2>
			BaseMatrixN<T, R, C2> operator *(const BaseMatrixN<T, R2, C2>& m2) const {
				BaseMatrixN<T, R, C2> result { };

				for (size_t i = 0; i < C2; i++)
				for (size_t j = 0; j < R; j++) {
					T sum = 0;

					for (size_t k = 0; k < C; k++)
						sum += m[k][j] * m2.m[i][k];

					result.m[i][j] = sum;
				}

				return result;
			}

			Matrix& operator +=(const Matrix& other) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] += other.m[i][j];
				return *this;
			}

			Matrix operator +(const Matrix& other) const {
				return Matrix(*this) += other;
			}
			
			Matrix& operator -=(const Matrix& other) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] -= other.m[i][j];
				return *this;
			}

			Matrix operator -(const Matrix& other) const {
				return Matrix(*this) -= other;
			}

			Matrix& operator *=(T other) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] *= other;
				return *this;
			}
			
			Matrix operator *(T other) const {
				return Matrix(*this) *= other;
			}

			Matrix& operator /=(T other) {
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] /= other;
				return *this;
			}
			
			Matrix operator /(T other) const {
				return Matrix(*this) /= other;
			}

			Matrix operator -() const {
				Matrix result { };
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					result.m[i][j] = -m[i][j];
				return result;
			}

			Minor minor(size_t c, size_t r) const {
				Minor result;
				size_t setR = 0;
				size_t setC = 0;
				for (size_t getC = 0; getC < C; getC++) {
					if (getC == c) continue;

					for (size_t getR = 0; getR < R; getR++) {
						if (getR == r) continue;
						result.m[setC][setR] = m[getC][getR];
						setR++;
					}

					setC++;
				}
				return result;
			}
			
			T cofactor(size_t c, size_t r) const {
				bool neg = ((r + c) & 1) == 1;
				return minor(c, r).determinant() * (neg ? -1 : 1);
			}

			T determinant() const {
				if constexpr (C != R)
					return 0;
				
				if constexpr (R == 1)
					return m[0][0];
				
				if constexpr (R == 2)
					return m[0][0] * m[1][1] - m[1][0] * m[0][1];
				
				T det = 0;
				for (size_t c = 0; c < C; c++)
					det += m[c][0] * cofactor(c, 0);
				return det;
			}

			Matrix cofactorMatrix() const {
				Matrix result;
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					result.m[i][j] = cofactor(i, j);

				return result;
			}

			Matrix& invert() {
				static_assert(C == R, "Tried to invert non-square matrix");
				static_assert(C <= 4 && R <= 4, "Tried to invert matrix of excessive size");
				if constexpr (R == 3) {
					*this = ((BaseMatrixN<T, 4, 4>)(*this)).inverse();
					// *this = (1.0f / determinant()) * cofactorMatrix().transposed();
					return *this;
				}
				
				T a = this->m[0][0];
				T b = this->m[1][0];
				T c = this->m[2][0];
				T d = this->m[3][0];
				T e = this->m[0][1];
				T f = this->m[1][1];
				T g = this->m[2][1];
				T h = this->m[3][1];
				T i = this->m[0][2];
				T j = this->m[1][2];
				T k = this->m[2][2];
				T l = this->m[3][2];
				T m = this->m[0][3];
				T n = this->m[1][3];
				T o = this->m[2][3];
				T p = this->m[3][3];

				T 	mult0 = k * p,
					mult1 = l * o,
					mult2 = j * p,
					mult3 = l * n,
					mult4 = j * o,
					mult5 = k * n,
					mult6 = i * p,
					mult7 = l * m,
					mult8 = i * o,
					mult9 = k * m,
					mult10 = i * n,
					mult11 = j * m,
					mult12 = g * p,
					mult13 = h * o,
					mult14 = f * p,
					mult15 = h * n,
					mult16 = f * o,
					mult17 = g * n,
					mult18 = e * p,
					mult19 = h * m,
					mult20 = e * o,
					mult21 = g * m,
					mult22 = e * n,
					mult23 = f * m,
					mult24 = g * l,
					mult25 = h * k,
					mult26 = f * l,
					mult27 = h * j,
					mult28 = f * k,
					mult29 = g * j,
					mult30 = e * l,
					mult31 = h * i,
					mult32 = e * k,
					mult33 = g * i,
					mult34 = e * j,
					mult35 = f * i;
				T det0 = mult0 - mult1,
					det1 = mult2 - mult3,
					det2 = mult4 - mult5,
					det3 = mult6 - mult7,
					det4 = mult8 - mult9,
					det5 = mult10 - mult11,
					det6 = mult12 - mult13,
					det7 = mult14 - mult15,
					det8 = mult16 - mult17,
					det9 = mult18 - mult19,
					det10 = mult20 - mult21,
					det11 = mult22 - mult23,
					det12 = mult24 - mult25,
					det13 = mult26 - mult27,
					det14 = mult28 - mult29,
					det15 = mult30 - mult31,
					det16 = mult32 - mult33,
					det17 = mult34 - mult35;
				T co0 = f * det0 - g * det1 + h * det2,
					co1 = e * det0 - g * det3 + h * det4,
					co2 = e * det1 - f * det3 + h * det5,
					co3 = e * det2 - f * det4 + g * det5,
					co4 = b * det0 - c * det1 + d * det2,
					co5 = a * det0 - c * det3 + d * det4,
					co6 = a * det1 - b * det3 + d * det5,
					co7 = a * det2 - b * det4 + c * det5,
					co8 = b * det6 - c * det7 + d * det8,
					co9 = a * det6 - c * det9 + d * det10,
					co10 = a * det7 - b * det9 + d * det11,
					co11 = a * det8 - b * det10 + c * det11,
					co12 = b * det12 - c * det13 + d * det14,
					co13 = a * det12 - c * det15 + d * det16,
					co14 = a * det13 - b * det15 + d * det17,
					co15 = a * det14 - b * det16 + c * det17;
				T idet = 1 / (a * co0 - b * co1 + c * co2 - d * co3);
				T m00 = co0 * idet,
					m01 = -co1 * idet,
					m02 = co2 * idet,
					m03 = -co3 * idet,
					m10 = -co4 * idet,
					m11 = co5 * idet,
					m12 = -co6 * idet,
					m13 = co7 * idet,
					m20 = co8 * idet,
					m21 = -co9 * idet,
					m22 = co10 * idet,
					m23 = -co11 * idet,
					m30 = -co12 * idet,
					m31 = co13 * idet,
					m32 = -co14 * idet,
					m33 = co15 * idet;

				this->m[0][0] = m00;
				this->m[0][1] = m01;
				this->m[0][2] = m02;
				this->m[0][3] = m03;
				this->m[1][0] = m10;
				this->m[1][1] = m11;
				this->m[1][2] = m12;
				this->m[1][3] = m13;
				this->m[2][0] = m20;
				this->m[2][1] = m21;
				this->m[2][2] = m22;
				this->m[2][3] = m23;
				this->m[3][0] = m30;
				this->m[3][1] = m31;
				this->m[3][2] = m32;
				this->m[3][3] = m33;

				return *this;
			}

			Matrix inverse() const {
				return Matrix(*this).invert();
			}

			Matrix& transpose() {
				static_assert(R == C, "Tried to transpose a non-square matrix");

				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					m[i][j] = m[j][i];

				return *this;
			}

			BaseMatrixN<T, C, R> transposed() const {
				BaseMatrixN<T, C, R> result { };
				for (size_t i = 0; i < C; i++)
				for (size_t j = 0; j < R; j++)
					result.m[j][i] = m[i][j];
				return result;
			}
	
			template <typename T2, size_t R2, size_t C2> requires (R2 <= R && C2 <= C)
			operator BaseMatrixN<T2, R2, C2>() const {
				BaseMatrixN<T2, R2, C2> result { };
				size_t rows = std::min(R, R2);
				size_t columns = std::min(C, C2);
				for (size_t i = 0; i < columns; i++)
				for (size_t j = 0; j < rows; j++)
					result.m[i][j] = (T2)m[i][j];
				return result;
			}
			
			static Matrix translation(const HVector& offset) {
				static_assert(C == R && R == 4, "Cannot construct non-3D translation matrix");

				Matrix result { };
				result.axes[3] = offset;
				return result;
			}

			static Matrix scale(const HVector& factor) {
				static_assert(C == R, "Cannot construct rectangular scaling matrix");

				Matrix result { };
				for (int i = 0; i < C - 1; i++)
					result.m[i][i] = factor.m[i];
				return result;
			}

			static Matrix cross(const Vector& u) {
				static_assert(C == R && R == 3, "Cannot construct non-3D cross-product matrix");

				return {
					0, -u[2], u[1],
					u[2], 0, -u[0],
					-u[1], u[0], 0
				};
			}
	};

	template <typename T, size_t R, size_t C>
	std::ostream& operator <<(std::ostream& out, const BaseMatrixN<T, R, C>& m) {
		out << "[\n";
		for (int j = 0; j < R; j++) {
			out << "\t";
			for (int i = 0; i < C; i++)
				out << m[i][j] << (i < C - 1 ? ", " : "\n");
		}
		out << "]";
		return out;
	}

	template <typename T, size_t R, size_t C>
	BaseMatrixN<T, R, C> operator *(T f, const BaseMatrixN<T, R, C>& m) {
		return m * f;
	}

	using Matrix3 = BaseMatrixN<float, 3, 3>;
	using Matrix4 = BaseMatrixN<float, 4, 4>;
	using dMatrix3 = BaseMatrixN<double, 3, 3>;
	using dMatrix4 = BaseMatrixN<double, 4, 4>;

	using Vector2 = BaseVectorN<float, 2>;
	using Vector3 = BaseVectorN<float, 3>;
	using dVector2 = BaseVectorN<double, 2>;
	using dVector3 = BaseVectorN<double, 3>;
}