#if !defined(_SEARCHLIB_H)

#define _SEARCHLIB_H

void *BinSearch(void *base0, size_t nmemb, size_t size,
                int (*compar)(void * ));

#endif
