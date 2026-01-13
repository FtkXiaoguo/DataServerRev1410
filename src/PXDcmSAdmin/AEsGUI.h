#ifndef AES_GUI__H
#define AES_GUI__H

#include "ui_AEMan.h"
#include "ui_AEItem.h"
 
#include "PxDcmsAEManage.h"
#include "TabPageCom.h" 

class CAEItem : public QWidget {
	Q_OBJECT
	public:
		CAEItem(bool isLocalAE,QWidget *parent = 0);
		void disableDetail(bool enable=false);

		
		bool queryDB();
		void displayAEItem(const std::string AEName);
		void setCurrentAE(const QString &AEName="");

		void setLocalAE(bool is) { m_isLocalAE = is;};
		bool isLocalAE() { return m_isLocalAE;};

		void setAEManage(CPxDcmsAEManage *AEMan) { m_AEManage=AEMan;};
	private:
		Ui::AEItem ui;
	public slots:
		void onAddAEItem();
		void onUpdateAEItem();
		void onDeleteAEItem();
		void onSelectAE(QString);
	 
protected:
	bool	m_isLocalAE;
     AEsMap m_AEList;
	 void readAEItem(AEItemData &AEItem);
	 void addAEItem(const AEItemData &AEItem,bool modify=false);
	void delAEItem(const std::string AEName);

private:

	 
	//
	 CPxDcmsAEManage *m_AEManage;
	 
	 
};

////////////
class CAEsMan : public TabPageComInf {
	Q_OBJECT
	public:
		CAEsMan(QWidget *parent = 0);
		void init();
//////////////////
	virtual void pageEntry();
	virtual void pageExit();
/////////////////		
	private:
		Ui::AEManage ui;
	public slots:
	 
protected:
     CAEItem *m_LocalAEs;
	 CAEItem *m_RemoteAES;
	 //
	 CPxDcmsAEManage *m_LocalAEManage;
	 CPxDcmsAEManage *m_RemoteManage;
private:
 

	 
};

#endif //AES_GUI__H


