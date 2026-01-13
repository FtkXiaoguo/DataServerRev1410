#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
	void onChangedLang(int);
	void onChangedMode(int);
	void onChangedUTF8(void);
private:
	void onChangeLangAndMode(void);
    Ui::MainWindow *ui;
	bool m_bInvalid;
};

#endif // MAINWINDOW_H
