#ifndef TAB_PAGE_COM_DEF_H
#define TAB_PAGE_COM_DEF_H
 
#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTabWidget>
#include <QWidget>

class TabPageComInf :public QWidget 
{
	Q_OBJECT
	public:
	TabPageComInf( QWidget *parent = 0);
	virtual ~TabPageComInf();	 
 

	virtual void pageEntry()	= 0;
	virtual void pageExit()		= 0;
	public slots:
		 
};

#endif //TAB_PAGE_COM_DEF_H


