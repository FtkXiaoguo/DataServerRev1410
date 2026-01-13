#pragma once

#include "DicomDBComIf.h"

class DicomDBComMain : public DicomDBComAPI::DicomDBComInterface
{
public:
	DicomDBComMain();
	~DicomDBComMain();
	virtual void destroy(void);
	virtual  bool initMain(void);
	virtual  void initDBCom(void);
	virtual  bool getMediaPointList(char **MediaPointListBuf, int &size);
	virtual  void getDBName(char *strBuff, int size);
	//
	virtual  void generatStudyFolder(const char *MediaPointTopFolder, const char *StudyUID, char *strBuff, int size);
	virtual  void generatSeriesFolder(const char *StudyFolder, const char *SeriesUID, char *strBuff, int size) ;
	virtual  void generatDicomFile(const char *SeriesFolder, const char *InstanceUID, int imageNum, char *strBuff, int size);

	virtual  bool getStudyDate(const char *DicomFile, SimpleDicomInfoDef *dcmInfo);
	virtual  bool doImportDicomfile(const char *DicomFile);
	virtual  bool findSeriesUIDFromDB(const char *seriesUID);
	virtual  bool doConform(const char* folder, DicomDBComAPI::CDicomDBComSearchFolderIf *callback, bool &toCancelFlag);
 
protected:
	 
};