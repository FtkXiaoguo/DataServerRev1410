#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)
#pragma warning (disable: 4244)

#include "dcmtk/ofstd/oftypes.h"
#include "dcmtk/dcmdata/dcdicent.h"



#include "DcmXTDicomMessageMain.h"

#include "DcmXTDataSetMain.h"
#include "DcmXTUtilMain.h"
//////////////////
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"
//#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcistrmz.h"    /* for dcmZlibExpectRFC1950Encoding */

#include "dcmtk/dcmnet/dimse.h"


#include "dcmtk/ofstd/ofstdinc.h"

#include "DcmTkBase.h"

#include "CheckMemoryLeak.h"

void test_dataset()
{
	DCMDATA_TRACE("start " <<   "\" test_dataset");

#ifdef _MemoryLeakCheck 
	_CrtMemState s1,s2,ss;
	_CrtMemCheckpoint(&s1);
#endif

	OFCondition cond;

	DcmDataset *data_set = new DcmDataset;

	DcmTag new_tag = GenDcmTagKey(0x00180022);
//	
 
	DcmElement *new_elem=0;
		 

	cond = DcmItem::newDicomElement(new_elem,new_tag);
	if(cond.bad()){
		return ;
	}
	 
 	cond = new_elem->putString("ttttttttttttttttttt");
 
	if(cond.bad()){
		return ;
	}

#if 1 
  	cond = data_set->insert(new_elem,true/*replaceOld*/);
 
	if(cond.bad()){
		return ;
	}
#else
	DcmList *elementList = new DcmList;
	elementList->insert(new_elem, ELP_first);

	//
	/* dump some information if required */
    DCMDATA_TRACE("DcmItem::insert() Element " << new_elem->getTag()
                 << " VR=\"" << DcmVR(new_elem->getVR()).getVRName() << "\" at beginning inserted");
	///
	elementList->deleteAllElements();
    delete elementList;
	 
#endif

	delete data_set;

#ifdef _MemoryLeakCheck 
	_CrtMemCheckpoint(&s2);
	if(_CrtMemDifference(&ss,&s1,&s2)){
		_CrtMemDumpStatistics( &ss );
	}else{
		printf("OK \n");
	}
#endif

}