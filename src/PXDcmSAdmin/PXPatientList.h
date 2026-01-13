#ifndef PX_PATIENT_LIST_H
#define PX_PATIENT_LIST_H

 
#include "PXTableWidget.h"

class SeriesCmdItem
{
public:
	SeriesCmdItem(){
		m_type = 0;
		m_defaultCmd = 0;
		
	}
	std::string m_menuName;
	std::string m_cmdLine;
	int m_type;
	int m_defaultCmd;

};

typedef std::vector<SeriesCmdItem> CmdLineList;

#define LIST_COLUMN_MAX 64

class PatListMain;
class CPXPatientList: public CPXTableWidget
{
Q_OBJECT
	public:
#define SERIES_CMD_MAX 4

		enum ListType {
			ListType_Unknown,
			ListType_Study,
			ListType_Series,
		};
	CPXPatientList(QWidget *parent = 0);
     
    ~CPXPatientList();
	virtual void setupTableWidget(ListType type,PatListMain *parentListMan);

//	bool updateSelectedList();

//	QVector<int> &getSelectedRows() { return m_selectedRows;};
	public slots:
		
		void onDeleteStudy();
		void onDeleteSeries();
		//
		void onPushStudy();
		void onPushSeries();
		//
		void onSeriesCmd0();
		void onSeriesCmd1();
		void onSeriesCmd2();
		void onSeriesCmd3();

		//
		
//	void selectColumn(int column);
//	void selectRow(int row);
 
	//#12 2012/03/26 K.Ko
	QString getStudyUID(int row);
	QString getSeriesUID(int row);
	//
//	int getCurrentSortColumn() const { return m_currentSortColumn;};
//	void updateSortColumn();
//	void setDefaultSortColumn(int col,bool AscendingOrder=true);
protected:
//	void drawLineColor();
	virtual void createActions();
	virtual void onMenu(const QPoint & pos);
//	virtual void mousePressEvent(QMouseEvent *);
//   virtual void mouseReleaseEvent(QMouseEvent *);
//	virtual void paintEvent(QPaintEvent *event);
	//
	QMenu *m_myMenu ;
	QAction *m_Act_deleteStudy;
	QAction *m_Act_deleteSeries;
	//
	QAction *m_Act_pushStudy;
	QAction *m_Act_pushSeries;
	//
	QAction *m_Act_OnSeriesCmd[SERIES_CMD_MAX];
	ListType m_type;
	//
	PatListMain *m_parentListMan;

#if 0
	int m_columnSortOrder[LIST_COLUMN_MAX];
	int m_currentSortColumn;


 	QVector<int>	m_selectedRows;
#endif
};

 

#endif //PX_PATIENT_LIST_H


