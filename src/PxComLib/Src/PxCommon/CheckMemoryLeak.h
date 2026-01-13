
//
// for Memory Leak Dump

#define MemoryLeakCheck


#ifdef WIN32
#ifdef _DEBUG
 
#ifdef MemoryLeakCheck 
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
 
//
#define   new                   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)             _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   calloc(c, s)          _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)         _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _recalloc(p, c, s)    _recalloc_dbg(p, c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _expand(p, s)         _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif //WIN32
#endif //_DEBUG
#endif