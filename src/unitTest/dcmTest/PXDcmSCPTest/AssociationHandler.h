// AssociationHandler.h: CAssociationHandler クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASSOCIATIONHANDLER_H__ECC75A3E_E494_4408_BDCA_0CB42BFD4DC4__INCLUDED_)
#define AFX_ASSOCIATIONHANDLER_H__ECC75A3E_E494_4408_BDCA_0CB42BFD4DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "rtvthread.h"
class TRLogger;
class CTstVLIDicomImage;
class CAssociationHandler : public iRTVThreadProcess  
{
public:
typedef std::vector<CTstVLIDicomImage *> DicomImageList;
typedef std::map<std::string,DicomImageList> SeriesManList;
typedef SeriesManList::iterator SeriesManIterator;

	CAssociationHandler(int id,TRLogger *log);
	virtual ~CAssociationHandler();

	void setupHomeFolder(const std::string &folder) { m_homeFolder = folder;};

	void setWriteFile(bool w_flag){m_writeFile=w_flag;};
	void setProImgFlag(bool proc_img) { m_prodImgFlag=proc_img;};

	void setCheckPixelData(int check){m_checkPixelData = check;};
	void setWriteFileFlag(bool writeFlag) { m_writeFile = writeFlag;};

	/* the PreProcess() is called in the main thread. If Preprocess()
	* returns a negative value, the thread will NOT run Process()
	*/
	virtual  int	PreProcess(void);
	
	/* The main entry for the threaded execution. When Process() ends, the
	* thread terminates.
	*	 The Process() function must check for termination request (TerminationRequested())
	*/
	virtual  int	Process(void);
	//
	bool isEnd() { return m_endFlag;};
protected:
static	bool GetValue( int A_messageid, unsigned long A_tag, 
                            char *A_value, int A_size, char *A_default );
static	bool SetValue( int A_messageid, unsigned long A_tag,
                            const char *A_value, const char *A_default, 
                            bool A_required );
	bool proc_CStore();
	bool response_CStore();
	bool procImage();
	bool procVolume();
	bool proc_end();
	bool closeAssociation();
	//
	void clearSeriesManList();

	int m_ImageCount;
	char*   m_serviceName;
	int		m_AssociationID;
	int		m_MessageID;
	//
	bool m_endFlag;
	//
//	DicomImageList m_ImageList;
	SeriesManList	m_SeriesManList;
	//
	int m_checkPixelData;
	//
	TRLogger  *m_Logger;
	//
	bool m_writeFile;
	bool m_prodImgFlag;
	//
	unsigned long m_AsoStartTime;
	std::string m_homeFolder;
};

#endif // !defined(AFX_ASSOCIATIONHANDLER_H__ECC75A3E_E494_4408_BDCA_0CB42BFD4DC4__INCLUDED_)
