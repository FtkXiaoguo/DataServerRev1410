#include "Login.h"

#include "QtHelper.h"

#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>

 #include "AEsGUI.h"
  #include "PxDcmDbManage.h"

 
CLogin::CLogin(QWidget *parent) : QDialog(parent)
{
	m_loginFlag = false;
	ui.setupUi(this);
	 
	ui.msgDisp->setText("");
 
 
	 
}
 
void CLogin::onLogin()
{
	ui.msgDisp->setText("");
	m_loginFlag = false;
#ifdef _DEBUG
#pragma message("login without password")
#else
	if(!verifyUserInf()){
		ui.msgDisp->setText("            *** Invalid user/password ***");
		return;
	}
#endif
	m_loginFlag = true;
	accept();
}
void CLogin::onExit()
{
	m_loginFlag = false;
	reject();
}
bool CLogin::verifyUserInf()
{
	bool ret_b = false;
	//
	std::string userName	= QString2Str(ui.edUser->text());
	std::string passwd		= QString2Str(ui.edPasswd->text());
	ret_b = CPxDcmDbManage::verifyUserAccount(userName, passwd);
	return ret_b;
}