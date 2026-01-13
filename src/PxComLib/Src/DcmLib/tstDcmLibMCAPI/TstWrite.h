#pragma once

#include "TstApi.h"

////
#define MemoryLeakCheck

#ifdef WIN32
#ifdef _DEBUG
 
#ifdef MemoryLeakCheck 
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define     malloc(p1)  _malloc_dbg(p1,_NORMAL_BLOCK,__FILE__,__LINE__)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif //WIN32
#endif //_DEBUG
#endif

class CTstWrite : public CTstApi
{
public:
	CTstWrite(void);
	~CTstWrite(void);

	void doTest();
	void doTestVector();
	void doTestConfig();
	void dotestItem();
protected:

	void writeFile();

};
