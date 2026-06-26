#pragma once

#include "../math/util.hpp"

#include <memory>
#include <string>
#include <cinttypes>
#include <bit>

namespace util {
	class Binary {
		private:
			using Byte = unsigned char;
			std::unique_ptr<Byte[]> buffer;
			size_t length;
	
			template <std::integral T>
			T endianSwap(T value) {
				T result = 0;
				for (int i = 0; i < sizeof(T); i++)
					result |= ((value >> (i * 8)) & 0xFF) << ((sizeof(T) - i - 1) * 8);
				return result;
			}

			void check(size_t offset) {
				if (offset > length) {
					size_t newLength = offset * 2 + 1;
					std::unique_ptr<Byte[]> newBuffer = std::make_unique<Byte[]>(newLength);
					memcpy(newBuffer.get(), buffer.get(), length);
					length = newLength;
					buffer = std::move(newBuffer);
				}
			}
			
		public:
			enum class Endian { LITTLE, BIG };
	
			Binary(const std::string& data) {
				length = data.size();
				buffer = std::make_unique<Byte[]>(length);
				memcpy(buffer.get(), data.data(), length);
			}

			Binary(const Binary& other) {
				length = other.length;
				buffer = std::make_unique<Byte[]>(length);
				memcpy(buffer.get(), other.buffer.get(), length);
			}
	
			template <math::Numeric T>
			T read(size_t offset, Endian endian = Endian::LITTLE) {
				check(offset + sizeof(T));
				T result = 0;
				for (int i = 0; i < sizeof(T); i++)
					result |= (T)buffer[offset + i] << (i * 8);
				
				return endian == Endian::BIG ? endianSwap(result) : result;
			}
	
			template <>
			float read<float>(size_t offset, Endian endian) {
				return std::bit_cast<float>(read<uint32_t>(offset, endian));
			}
	
			template <>
			double read<double>(size_t offset, Endian endian) {
				return std::bit_cast<double>(read<uint64_t>(offset, endian));
			}
	
			template <math::Numeric T>
			void write(size_t offset, T value, Endian endian = Endian::LITTLE) {
				check(offset + sizeof(T));
				if (endian == Endian::BIG) value = endianSwap(value);
				for (int i = 0; i < sizeof(T); i++)
					buffer[offset + i] = (Byte)(value >> (i * 8));
			}
	
			template <>
			void write(size_t offset, float value, Endian endian) {
				write(offset, std::bit_cast<uint32_t>(value), endian);
			}
	
			template <>
			void write(size_t offset, double value, Endian endian) {
				write(offset, std::bit_cast<uint64_t>(value), endian);
			}

			explicit operator std::string() const {
				std::string result = "";
				for (size_t i = 0; i < length; i++)
					result += buffer[i];
				return result;
			}
	};
	
	class BinaryStream {
		public:
			size_t pointer = 0;
			Binary buffer;

			BinaryStream(const Binary& _buffer) : buffer(_buffer) { }

			template <math::Numeric T>
			T read(Binary::Endian endian = Binary::Endian::LITTLE) {
				T result = buffer.read<T>(pointer, endian);
				advance<T>();
				return result;
			}

			template <math::Numeric T>
			void write(T value, Binary::Endian endian = Binary::Endian::LITTLE) {
				buffer.write(pointer, value, endian);
				advance<T>();
			}

			void advance(size_t amount) {
				pointer += amount;
			}

			template <typename T>
			void advance() {
				advance(sizeof(T));
			}
	};
}