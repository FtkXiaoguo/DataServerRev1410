#ifndef SETTING_GUI__H
#define SETTING_GUI__H

#include "ui_AEMan.h"
#include "ui_Setting.h"
#include "TabPageCom.h" 
 

class CSetting : public TabPageComInf{
	Q_OBJECT
	public:
		CSetting( QWidget *parent = 0);
//////////////////
	virtual void pageEntry();
	virtual void pageExit();
/////////////////		 
	private:
		Ui::Setting ui;
	public slots:
		 void onTabChanged(int);
	 
protected:
	 

private:

	void showTabePage(int index); 
	 
	int m_TabPages;
	 int m_curTabeIndex;
	 bool m_TabPageReady;
};


#endif //AES_GUI__H


