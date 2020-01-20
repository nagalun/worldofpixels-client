#include "Packet.hpp"

#include <typeindex>

#include "demangler.hpp"

namespace pktdetail {

sz_t getSize(const std::type_index& ti) {
	return getSize(demangle(ti));
}

sz_t writeToBuf(u8 *& b, const std::type_index& ti, sz_t remaining) {
	return writeToBuf(b, demangle(ti), remaining);
}

}
