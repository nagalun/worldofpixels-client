#ifndef OWOP_VERSION
#	define OWOP_VERSION unknown
#endif
#ifdef DEBUG
#	define BUILD_TYPE dbg
#else
#	define BUILD_TYPE rel
#endif

// doesn't actually work, very annoying
//#define _STR(x) "x"
//#define STR(x) _STR(x)

#define _CONCAT(x,y) x/**/y
#define CONCAT(x,y) _CONCAT(x,y)

#define OWOP_SCRIPT_PATH CONCAT(CONCAT(/js/owop-,OWOP_VERSION),.js)
#define OWOP_WASM_PATH CONCAT(CONCAT(/js/owop-,OWOP_VERSION),.wasm)
