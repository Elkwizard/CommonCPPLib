#include "window.hpp"

namespace window {
	std::ostream& operator <<(std::ostream& out, const Rect& rect) {
		out << rect.width << "x" << rect.height << " at " << rect.x << ", " << rect.y;
		return out;
	}	
}