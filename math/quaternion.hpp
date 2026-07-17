#pragma once

#include "matrix.hpp"

namespace math {
	template <std::floating_point T>
	class BaseQuaternion {
		private:
			using Quaternion = BaseQuaternion<T>; 
		
		public:
			using Vector = BaseVectorN<T, 3>;
		
			T r;
			Vector v;

			BaseQuaternion() {
				r = 1;
				v = { 0, 0, 0 };
			}
			
			BaseQuaternion(T _r, const Vector& _v) {
				r = _r;
				v = _v;
			}

			BaseQuaternion(const Vector& _v) {
				r = 0;
				v = _v;
			}

			BaseQuaternion(T a, T b, T c, T d) {
				r = a;
				v = { b, c, d };
			}

			Quaternion conjugate() const {
				return { r, -v };
			}

			T sqrNorm() const {
				return r * r + v.sqrMag();
			}

			T norm() const {
				return sqrt(sqrNorm());
			}

			Quaternion reciprocal() const {
				Quaternion conj = conjugate();
				conj /= sqrNorm();
				return conj;
			}

			Quaternion versor() const {
				return *this / norm();
			}

			Vector rotate(const Vector& vec) const {
				return (*this * Quaternion(vec) * conjugate()).v;

				Vector v1 = r * vec + v.cross(vec);
				return v.dot(vec) * v + r * v1 - v1.cross(v);
			}

			Matrix3 matrix() const {
				Matrix3 result;
				result.axes.x = rotate({ 1, 0, 0 });
				result.axes.y = rotate({ 0, 1, 0 });
				result.axes.z = rotate({ 0, 0, 1 });
				return result;
			}

			bool operator ==(const Quaternion& other) const {
				return r == other.r && v == other.v;
			}

			bool operator !=(const Quaternion& other) const {
				return !(*this == other);
			}

			Quaternion& operator +=(const Quaternion& other) {
				r += other.r;
				v += other.v;
				return *this;
			}
			
			Quaternion& operator +=(T other) {
				r += other;
				v += other;
				return *this;
			}

			template <typename O>
			Quaternion operator +(const O& other) const {
				return Quaternion(*this) += other;
			}
			
			Quaternion& operator -=(const Quaternion& other) {
				r -= other.r;
				v -= other.v;
				return *this;
			}

			Quaternion& operator -=(T other) {
				r -= other;
				v -= other;
				return *this;
			}

			template <typename O>
			Quaternion operator -(const O& other) const {
				return Quaternion(*this) -= other;
			}
			
			Quaternion& operator *=(const Quaternion& other) {
				T _r = r * other.r - v.dot(other.v);
				v = r * other.v + other.r * v + v.cross(other.v);
				r = _r;
				return *this;
			}

			Quaternion& operator *=(T other) {
				r *= other;
				v *= other;
				return *this;
			}

			template <typename O>
			Quaternion operator *(const O& other) const {
				return Quaternion(*this) *= other;
			}

			Quaternion& operator /=(T other) {
				r /= other;
				v /= other;
				return *this;
			}

			Quaternion operator /(T other) const {
				return Quaternion(*this) /= other;
			}

			Quaternion& then(const Quaternion& q) {
				*this = q * *this;
				return *this;
			}

			Vector toRotation() const {
				T mag = v.mag();
				T angle = 2 * atan2(mag, r);
				if (!angle) return { };
				Vector axis = v / mag;
				return axis * angle;
			}

			static Quaternion fromRotation(const Vector& axis, T angle) {
				if (!angle) return { };
				T phi = angle / 2;
				auto cosphi = cos(phi);
				auto scale = sin(phi) / axis.mag();
				return { cosphi, axis * scale };
			}

			static Quaternion fromRotation(const Vector& rotation) {
				T mag = rotation.mag();
				return fromRotation(rotation / mag, mag);
			}
	};

	template <std::floating_point T>
	std::ostream& operator <<(std::ostream& out, const BaseQuaternion<T>& q) {
		out << q.r << " + " << out << (T)q.v.x << "i + " << out << (T)q.v.y << "j + " << out << (T)q.v.z << "k";
		return out;
	}

	using Quaternion = BaseQuaternion<float>;
	using dQuaternion = BaseQuaternion<double>;
}