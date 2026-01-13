#ifndef DICOM_TAG_LIST_MAN__H
#define DICOM_TAG_LIST_MAN__H
 
#include "ui_DicomTagList.h"
 
#include "PxTagFilterManager.h"

class CPXTableWidget;
 
class CDicomTagList : public QDialog {
	Q_OBJECT
	public:
		CDicomTagList( QWidget *parent = 0);
	virtual ~CDicomTagList();	 
	virtual void resetColumnWidth(int w);
	 
	void doInit();
	void onRefresh();
	 
	protected:
		Ui::TagNameList ui;
	public slots:
		void onDelete() ;
		void onAdd();
	    void onGetTagName();
		void onClose();
	 
	
protected:
	 
	int getQueueID(int sel_row);
 //	QueuViewItem *getViewItem(int sel_row);
	virtual void resizeEvent(QResizeEvent *);
	//
 
	
	virtual int dispHeader(int col_index=0);//return last column index
	virtual void dispQueueList();
	virtual int dispQueueItem(int row,PxDBUtil::DB_ID_NameEx &dicomTag_item,int col_index=0);//return last column index
	/////////////
	virtual int getListColumn() { return m_QueueListColumn;};
	virtual void updateList(){};
	//
	int m_QueueListColumn;
	CPXTableWidget *m_DicomTagListTableWidget;
	
	//
	std::vector<PxDBUtil::DB_ID_NameEx> m_TagNameList;
	//
 
private:
	int m_curWidth;
	int m_refWidth;
	bool m_viewDispReady;
	//
	
};
 

#endif //DICOM_TAG_LIST_MAN__H


