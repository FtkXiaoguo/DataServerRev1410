#include "StdAfx.h"
#include "TstApi.h"

static char blankPixelData[] = {0x00, 0x00, 0x00, 0x00};

MC_STATUS SetBlankPixelData(int A_msgID, unsigned long A_tag, int A_isFirst, void* A_info, 
							int* A_dataSize, void** A_dataBufferPtr, int* A_isLastPtr)
{
//	assert(A_tag == MC_ATT_PIXEL_DATA);
//	assert(A_isFirst != 0);

	if (A_isFirst)
	{
		*A_dataSize = 4;
		*A_dataBufferPtr = blankPixelData;
		*A_isLastPtr = 1;
	}

    return MC_NORMAL_COMPLETION;
}
MC_STATUS SetDataBuffer(int messageID, unsigned long tag, int isFirst, void* userInfo, 
							   int* dataSizePtr, void** dataBufferPtr, int* isLastPtr)
{
	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	CBDataStruct* cbdata = (CBDataStruct*) userInfo;
	
	if (!cbdata->m_buf)
		return MC_CALLBACK_CANNOT_COMPLY;

	*dataBufferPtr = cbdata->m_buf;
	*dataSizePtr = cbdata->m_bufsize;
	*isLastPtr = 1;
	return MC_NORMAL_COMPLETION;
}

CTstApi::CTstApi(void)
{
}

CTstApi::~CTstApi(void)
{
}

void CTstApi::init()
{
	int status;
	//	Initialize the toolkit
//	status = MC_Library_Initialization(MC_Config_Values, MC_Dictionary_Values, NULL);

	status = MC_Library_Initialization(NULL, NULL, NULL);
	
	//  moved up
	if ( status == MC_LIBRARY_ALREADY_INITIALIZED )
		return  ;
}

void CTstApi::release()
{
	MC_Library_Release();
}