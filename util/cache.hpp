#pragma once

namespace util {
	template <typename T>
	class Cache {
		private:
			T value;
			bool valid;
		
		public:
			Cache() { }
			
			T& operator =(const T& newValue) {
				value = newValue;
				valid = true;
				return value;
			}
			
			void invalidate() {
				valid = false;
			}

			bool isValid() const {
				return valid;
			}

			T& get() {
				return value;
			}

			const T& get() const {
				return value;
			}
	};
}