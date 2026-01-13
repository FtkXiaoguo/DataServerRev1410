#ifndef PAT_LIST_H
#define PAT_LIST_H

#include "ui_PatList.h"

#include <QTableWidget>

#include "PXPatientList.h"

#include "TabPageCom.h" 



class PatListMain;

class CImageDrawWidget : public QWidget
{
 Q_OBJECT 

 public:
 CImageDrawWidget(QWidget *parent = 0);
     
    ~CImageDrawWidget();
 
	void creatDrawArea(QWidget *parent);

	void setupBitMap(const unsigned char *bitmap,int sizeX,int sizeY,int bpp);
	void clearImage();
	public slots:
		
		 
protected:
	 virtual void paintEvent(QPaintEvent *event);
	 int m_sizeX;
	 int m_sizeY;
	 unsigned char *m_bitMap;
	 QImage *m_qImage ;
	 int m_drawSizeX;
	 int m_drawSizeY;
};

  


class PxImageInfor;
class QProcess;
class AEItemData;
class PxDicomInfor;
class CImageViewer;
class PatListMain : public TabPageComInf {
	Q_OBJECT
	public:
		PatListMain(QWidget *parent = 0);
		void onSeriesCmd(int );
//////////////////
	virtual void pageEntry();
	virtual void pageExit();
/////////////////
		const CmdLineList &getCmdLine() const { return m_cmdLineList;};
/////
	 void resetStudyColumnWidth(int w);
	 void resetSeriesColumnWidth(int w);

	private:
		Ui::PatList ui;
	public slots:
	   void onSelectStudy();
	   void onSelectSeries();
	   void onDclickStudy();
	   void onDclickSeries();
	   //
	   void onSearch();
	   void onSelect(QTableWidgetItem*,QTableWidgetItem*);
	   //
	   void onImageFrameNo(int);
	   void onSliderFrameNo(int);
	   //
	   void onDeleteStudy();
	   void onDeleteSeries();
	   //
	   void onPushStudy();
	   void onPushSeries();

	   void onStudyDateChk();
	   void onStudyDateSelChk();
	   //
	   void cancelDeleteStudy();

	   //
	   void listupToday();
protected:
	virtual ~PatListMain();
     
	QString  checkMenuCmd(const QString &cmd_line); //#42 2012/12/20 menu launcher
	void exeDefaultSeries(const std::string &studyUID,const std::string &seriesUID); //#42 2012/12/20
	void exeCmdLine(const SeriesCmdItem &cmd, const std::string &arg);//#42 2012/12/20

	bool pickCmdLine(SeriesCmdItem &outCmdItem,bool oneImage=false);//#42 2012/12/20

	void dispStudyHeader();
	void dispSeriesHeader();
	//
	void dispSearchStudys();
	void dispSearchSeries();

	void searchSeries(const std::string &studyUID);
	//
	void searchImages(const std::string &studyUID,const std::string &seriesUID);
	void dispSOPImage(const PxImageInfor *Image,PxDicomInfor *dicom_info=0/*for safty check*/); //#27 2012/06/14
	//
	void setupFrameNoRange(int frames,int curNo=0);
	//
	void readCdmLines();
	//
	void switchStudyDate();
	//
	bool pushSelectStudy(const AEItemData *AE_title);
	bool pushSelectSeries(const AEItemData *AE_title);
protected:
	PxDicomInfor *getCurrentStudyInfo() const ;
    // Event handlers
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void closeEvent(QCloseEvent *);

	virtual void resizeEvent ( QResizeEvent * event ) ;
private:
	std::string m_CharacterSet;//#140_search_Japanese_JIS_UTF8
 
	CPXPatientList *m_StudyList;
	//
	int	m_studyListColumn;
 
	CPXPatientList *m_SeriesList;
	int	m_seriesListColumn;
	 
	//
	QProcess *m_ProcSeriesCmd1;
	//
	CImageDrawWidget *m_preViewImageArea;
//	CImageViewer *m_imageViewer;
	
	//
	CmdLineList m_cmdLineList;
	//
	int m_curStudyWidth;
	int m_refStudyWidth;
	bool m_viewDispStudyReady;
	 
	bool m_viewDispSeriesReady;
};

#endif


