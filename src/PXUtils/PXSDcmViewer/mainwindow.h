#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_viewer.h"
#include "imageViewer.h"

class CHistogramData;

 
class CanvasWidget;


class MainWindow : public QMainWindow {
	Q_OBJECT
	public:
		MainWindow(QWidget *parent = 0);
		enum StatusIndex {
			StatusIndex_debug0 = 0,
			StatusIndex_debug1	,
			StatusIndex_debug2	,
			StatusIndex_DispFrame = 4,
			StatusIndex_DispCursor = 6,
		};
		void initDcm();

		static std::string read_shader_file(const QString &fileNameInRes);
		void openDicomSeries(const QStringList &dicomList);

	void setupStatusBarMsg(const QString &msg,int index=0);
	void onFrameStep(int step);

	void notifyWWWL(int ww,int wl);
	void chgCursorStatus(bool reverse = false);
	void setCursorStatus(int index);
//	void change2OpenGL();
	private:
		Ui::MainWindow ui;
	public slots:
	 
		void onChangeFilter(int );
		void onChangeLut(int );
		void onFileOpen();
		void onFileOpenDir();
		void onAppExit();
		void onInformation();
		void onChgMouse();
		void onHistoGram();

		void onFrameStepLeft() ;
		void onFrameStepRight() ;
		void onFrameMove(int) ;
		//
		void onChgSliderWW(int);
		void onChgSliderWL(int);
		void onChgEditWW(int);
		void onChgEditWL(int);
		//
		void onFitZoom() ;
        void onAutoWWWL();

protected:
	void setupMultiFrameGUI(bool multiFrame);
	void openDicomFile(const QString &dicomFile);
	void openDicomFolder(const QString &dicomDir);
	void setupImageDisplay(int ww,int wl,int pixel_max,float hu_offset=0.0f);
	void initGLSL();
    void paintEvent(QPaintEvent *event);
	virtual void resizeEvent(QResizeEvent *);

	virtual void closeEvent(QCloseEvent *);
	virtual void showEvent(QShowEvent *);
	virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
	//
	void setupWWWL();
private:
	
	QStringList m_DicomFileList;

	CHistogramData *m_hist_data;
	CanvasWidget *m_openGLWidget;

	bool m_valid ;

	bool m_first_show_flag;

	bool m_filter_created_flag;

	PxImagePixel *m_CurrentPixelData;
	PxDicomInfor *m_CurrentDicomInfo;

	bool m_internalGUIChgFlag;
	
	bool m_newSeriesFlag;
#define STATUS_FIELD_NUM (7)
	QString m_DispStatusBarMsg[STATUS_FIELD_NUM]; 
	QLabel *m_status_field[STATUS_FIELD_NUM];

	bool m_multiFrameFlag;

	//
	CanvasWidget	*m_CanvasWidgetOpenGLTemp ;
	CanvasWidget	*m_CanvasWidgetGLShaderTemp;

	bool m_settingWWWL_flag;
	float m_disp_HU_val;
};

#endif


