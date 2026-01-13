// TstSrcData.h: CTstSrcData クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTSRCDATA_H__BE9ED418_DC49_4111_9DAB_E493CE0CA452__INCLUDED_)
#define AFX_TSTSRCDATA_H__BE9ED418_DC49_4111_9DAB_E493CE0CA452__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstVLIDicomImage.h"

#pragma warning (disable: 4786)

#include "vector"

using namespace std;

typedef std::vector<CTstVLIDicomImage *> SrcDataImageList;
typedef std::vector<CTstVLIDicomImage *>::iterator SrcDataImageIter;
//
typedef std::vector<SrcDataImageList *>	SrcDataSeriesList;
typedef std::vector<SrcDataImageList *>::iterator SrcDataSeriesIter;	

class CTstSrcData  
{
public:
	CTstSrcData();
	virtual ~CTstSrcData();
	bool openStudy(string studyFloder);

	SrcDataSeriesList getSeriesList() const { return m_SeriesList;};
protected:
	void destroy();
	bool procStudy(string studyFolder);
	bool procSeries(string SeriesFolder);
	bool procDicomFile(string DicomFile,int ImageNumber, SrcDataImageList * ImageList);

	SrcDataSeriesList m_SeriesList;

};

#endif // !defined(AFX_TSTSRCDATA_H__BE9ED418_DC49_4111_9DAB_E493CE0CA452__INCLUDED_)
