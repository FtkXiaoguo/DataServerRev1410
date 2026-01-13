#ifndef AELIST_GUI__H
#define AELIST_GUI__H

#include "ui_AEMan.h"
#include "ui_AEList.h"
 
#include "PXDcmsAEManage.h"

#include "PXTableWidget.h"



class CPXMyAEList;
class CAEList : public QDialog {
	Q_OBJECT
	public:
		CAEList( QWidget *parent = 0);
	virtual ~CAEList();	 
	private:
		Ui::AEListMain ui;
	public slots:
		 
	void onSelectAE();
	void onOK();
	void onExit();
	 
	AEItemData getSelectedAE();
	bool isOK() const { return m_isOK;};
protected:
	void listAEs();

private:

	 AEsMap m_remoteAEList; 
	 
	 CPXMyAEList *m_AEListGUI;
	 int m_curRow;
	 bool m_isOK;
};


#endif //AELIST_GUI__H


