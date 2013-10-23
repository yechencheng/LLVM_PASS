#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#ifdef DEBUG_FLAG
#define DEBUG(LEVEL, X) { if((LEVEL) >= (DEBUG_FLAG)) {X;} };

#else
#define DEBUG(X) { }

#endif
#endif
