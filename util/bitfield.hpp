#pragma once

namespace util {
	template <typename T>
	class Bitfield {
		public:
			T field;

			Bitfield() {
				field = 0;
			}

			Bitfield(T _field) {
				field = _field;
			}

			bool get(int i) const {
				return (field >> i) & 1;
			}

			void set(int i) {
				field |= (1 << i);
			}

			void reset(int i) {
				field &= ~(1 << i);
			}

			void set(int i, bool value) {
				if (value) set(i);
				else reset(i);
			}

			void toggle(int i) {
				set(i, !get(i));
			}
	};
}