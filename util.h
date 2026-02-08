void die(const char *fmt, ...);

#ifdef _DEBUG
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif
#define xstr(s) str(s)
#define str(s) #s
#define FREE(A) if ((A) != NULL) { free((A)); DEBUG("free called "xstr(A)"\n"); (A) = NULL; }
