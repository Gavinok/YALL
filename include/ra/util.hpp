#ifndef UTIL
#define UTIL

// Used to enable debugging of YALL
// #define DEBUG_YALL
#ifdef DEBUG_YALL
#define DBG(X)                                                                 \
  { std::cout << X << std::endl; }
#else
#define DBG(X)                                                                 \
  {}
#endif

#endif
