#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_main_w.h"

 
class MainWindow : public QMainWindow {
	Q_OBJECT
	public:
		MainWindow(QWidget *parent = 0);

		void loadStyleSheet(std::string fileName);
	private:
		Ui::MainWindow ui;
	public slots:
		void onTabChanged(int);

protected:
    void paintEvent(QPaintEvent *event);

	virtual void showEvent(QShowEvent *);
private:
	void showTabePage(int index);
 

	 int m_TabPages;
	 int m_curTabeIndex;
	 bool m_TabPageReady;

	 bool m_firstFlag ;;//#39
	 
	QString m_ver_info;//2012/11/19 adjust GUI
};

#endif


