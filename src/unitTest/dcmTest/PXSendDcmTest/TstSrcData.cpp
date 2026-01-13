// TstSrcData.cpp: CTstSrcData クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TstSrcData.h"

#include "AqCore/TRPlatform.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTstSrcData::CTstSrcData()
{

}

CTstSrcData::~CTstSrcData()
{
	destroy();
}
void CTstSrcData::destroy()
{
	SrcDataImageIter ImageIter;
	SrcDataSeriesIter SeriesIter;
	for (SeriesIter = m_SeriesList.begin(); SeriesIter < m_SeriesList.end(); SeriesIter++)
	{
		for(ImageIter = (*SeriesIter)->begin();ImageIter<(*SeriesIter)->end();ImageIter++){
			delete *ImageIter;
		}
		delete *SeriesIter;
	}
}
bool CTstSrcData::openStudy(std::string studyFloder)
{
	destroy();

	return procStudy( studyFloder);
}


bool CTstSrcData::procStudy(string studyFolder)
{
	std::vector<TRFileName> SeriesFolderList;
	std::vector<TRFileName>::iterator iter;

	std::string seriesPath;
	int tmpStatus = TRPlatform::iGetDirectoryList(studyFolder.c_str(), "*", SeriesFolderList);
	if (tmpStatus < 0 || SeriesFolderList.size() < 3){
		return false;
	}

	unsigned long start_time = ::GetTickCount();

	for (iter = SeriesFolderList.begin(); iter < SeriesFolderList.end(); iter++)
	{
		if (!strcmp(iter->GetName(),".") || !strcmp(iter->GetName(), ".."))
			continue;
		seriesPath = studyFolder + std::string("/") + iter->GetName();
		if (!TRPlatform::IsDirectory(seriesPath.c_str()))
			continue;
		if(!procSeries(seriesPath)){
			return false;
		}
	}

	unsigned long end_time = ::GetTickCount();
	printf("\n <<< maek study list spent time %.2f >>> \n",(end_time - start_time)/1000.0f);

	return true;
}
bool CTstSrcData::procSeries(string SeriesFolder)
{

	std::vector<TRFileName> DicomFilList;
	std::vector<TRFileName>::iterator iter;
	std::string DicomFileName;

	int tmpStatus = TRPlatform::iGetDirectoryList(SeriesFolder.c_str(), "*", DicomFilList);
	if (tmpStatus < 0 || DicomFilList.size() < 3){
		return false;
	}

	SrcDataImageList *new_image_list = new SrcDataImageList;
	int imageNum = 0;
	 
	for (iter = DicomFilList.begin(); iter < DicomFilList.end(); iter++)
	{
		if (!strcmp(iter->GetName(),".") || !strcmp(iter->GetName(), ".."))
			continue;
		DicomFileName = SeriesFolder + std::string("/") + iter->GetName();
		if(!procDicomFile(DicomFileName,imageNum++,new_image_list)){
			 continue;
		}
		;;
	}
	m_SeriesList.push_back(new_image_list);


	return true;
}
bool CTstSrcData::procDicomFile(string DicomFile,int ImageNumber, SrcDataImageList * ImageList)
{
	CTstVLIDicomImage *dicomImage = new CTstVLIDicomImage;
	if(!dicomImage->loadDicom(DicomFile.c_str())){
		return false;
	}

 	ImageList->push_back(dicomImage);
	 
	return true;
}