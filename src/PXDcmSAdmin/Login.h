#ifndef LOGIN_GUI__H
#define LOGIN_GUI__H

#include "ui_AEMan.h"
#include "ui_Login.h"
 
 

class CLogin : public QDialog {
	Q_OBJECT
	public:
		CLogin( QWidget *parent = 0);
		 
		bool isLogin() const { return m_loginFlag;};
	private:
		Ui::LoginMain ui;
	public slots:
	
	void onLogin();
	void onExit();
	 
protected:
	bool verifyUserInf();
	bool m_loginFlag;

private:

	 
	 
};


#endif //LOGIN_GUI__H


