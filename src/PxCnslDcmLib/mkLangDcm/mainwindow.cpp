#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "testPxDcmProc.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
	m_bInvalid(true),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
 
	ui->comboMode->addItem("Pano");
	ui->comboMode->addItem("Cepha");
	ui->comboMode->addItem("CT");
	ui->comboMode->setCurrentIndex(0);
	//
	ui->comboBoxLang->addItem("Chinese");
	ui->comboBoxLang->addItem("Russian");
	ui->comboBoxLang->addItem("Japanese");
	ui->comboBoxLang->addItem("Latin");
	ui->comboBoxLang->addItem("Chinese(TW)");
	ui->comboBoxLang->setCurrentIndex(0);
	//
	ui->comboVolSize->addItem("64");
	ui->comboVolSize->addItem("128");
	ui->comboVolSize->addItem("256");
	ui->comboVolSize->addItem("512");
	ui->comboVolSize->addItem("1024");
	ui->comboVolSize->setCurrentIndex(0);
	ui->comboVolSize->hide();
 

	ui->lineEdit_ID->setText("123455555");

	m_bInvalid = false;
	onChangedLang(CTestDcmAPICls::LANG_ID_CHINESE);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onChangedMode(int)
{
	onChangeLangAndMode();
}
void MainWindow::onChangedLang(int LangId)
{
	ui->checkBoxUTF8->setVisible(false);
	if (LangId == CTestDcmAPICls::LANG_ID_CHINESE_TW){
		ui->checkBoxUTF8->setVisible(true);
		ui->checkBoxUTF8->setChecked(true) ;
		ui->checkBoxUTF8->setDisabled(true);
	}
	if (LangId == CTestDcmAPICls::LANG_ID_JAPANESE){
		ui->checkBoxUTF8->setVisible(true);
		ui->checkBoxUTF8->setDisabled(false);
	}
	
	onChangeLangAndMode();
}
void MainWindow::onChangedUTF8(void)
{
	onChangeLangAndMode();
}
void MainWindow::onChangeLangAndMode(void) 
{
	if (m_bInvalid) return;

	int ModalityId = ui->comboMode->currentIndex();
	int LangId = ui->comboBoxLang->currentIndex();
	bool bIsUTF8 = ui->checkBoxUTF8->isChecked();
	//
	ui->comboVolSize->hide();
	std::wstring str_modality = L"CT";
	switch (ModalityId){
	case CTestDcmAPICls::MODALITY_ID_PANO:
		str_modality = L"Pano";
		break;
	case CTestDcmAPICls::MODALITY_ID_CEPHA:
		str_modality = L"Cepha";
		break;
	case CTestDcmAPICls::MODALITY_ID_CT:
		str_modality = L"CT";
		ui->comboVolSize->show();
		break;
	}
	switch (LangId){
	case CTestDcmAPICls::LANG_ID_CHINESE://-Chinese
	{
		switch (ModalityId){
		case CTestDcmAPICls::MODALITY_ID_PANO:
			str_modality = L"全景";
			break;
		case CTestDcmAPICls::MODALITY_ID_CEPHA:
			str_modality = L"头颅侧位片";
			break;
		}
	 
		std::wstring name1 = L"子龙";
		std::wstring name2 = L"赵";
		ui->lineEdit_2ndName->setText(QString::fromStdWString(name2));
		ui->lineEdit_1stName->setText(QString::fromStdWString(name1));
		//
		ui->lineEdit_ID->setText("123456001");
		//
		std::wstring OpName1 = L"居易";
		std::wstring OpName2 = L"白";
		ui->lineEdit_Op_2ndName->setText(QString::fromStdWString(OpName2));
		ui->lineEdit_Op_1stName->setText(QString::fromStdWString(OpName1));
		//
		ui->lineEdit_Comment->setText(QString::fromStdWString(L"病人的描述"));
		ui->lineEdit_StudyDes->setText(QString::fromStdWString(str_modality) + QString::fromStdWString(L"检查的描述"));
		ui->lineEdit_SeriesDes->setText(QString::fromStdWString(L"Series的描述"));
		ui->lineEdit_Institutaion->setText(QString::fromStdWString(L"牙科"));
		ui->lineEdit_Station->setText(QString::fromStdWString(L"牙科-A"));
	}
		break;
	case CTestDcmAPICls::LANG_ID_RUSSIAN://-Russian
	{
		std::wstring name1 = L"Анна";
		std::wstring name2 = L"Каренина";
		ui->lineEdit_2ndName->setText(QString::fromStdWString(name2));
		ui->lineEdit_1stName->setText(QString::fromStdWString(name1));
		//
		ui->lineEdit_ID->setText("123456002");
		//
		std::wstring OpName1 = L"Алексея";
		std::wstring OpName2 = L"Каренина";
		ui->lineEdit_Op_2ndName->setText(QString::fromStdWString(OpName2));
		ui->lineEdit_Op_1stName->setText(QString::fromStdWString(OpName1));
		//
		ui->lineEdit_Comment->setText(QString::fromStdWString(L"Комментарий пациента"));
		ui->lineEdit_StudyDes->setText(QString::fromStdWString(str_modality) + QString::fromStdWString(L" пациента"));
		ui->lineEdit_SeriesDes->setText(QString::fromStdWString(L"Seriesпациента"));
		ui->lineEdit_Institutaion->setText(QString::fromStdWString(L"Institutaion пациента"));
		ui->lineEdit_Station->setText(QString::fromStdWString(L"Institutaion пациента-A"));

	}
		break;
	case CTestDcmAPICls::LANG_ID_JAPANESE://-Japanese
	{
		std::wstring name1 = L"太郎";
		std::wstring name2 = L"田中";
		if (bIsUTF8) {
			name1 = name1 + L"UTF8";
		}
		ui->lineEdit_2ndName->setText(QString::fromStdWString(name2));
		ui->lineEdit_1stName->setText(QString::fromStdWString(name1));
		//
		ui->lineEdit_ID->setText("123456003");
		//
		std::wstring OpName1 = L"三郎";
		std::wstring OpName2 = L"鈴木";
		ui->lineEdit_Op_2ndName->setText(QString::fromStdWString(OpName2));
		ui->lineEdit_Op_1stName->setText(QString::fromStdWString(OpName1));
		//
		ui->lineEdit_Comment->setText(QString::fromStdWString(L"インプラント"));
		ui->lineEdit_StudyDes->setText(QString::fromStdWString(str_modality) + QString::fromStdWString(L"検査"));
		ui->lineEdit_SeriesDes->setText(QString::fromStdWString(L"Series検査"));
		ui->lineEdit_Institutaion->setText(QString::fromStdWString(L"クリニック"));
		ui->lineEdit_Station->setText(QString::fromStdWString(L"ステーション"));
	}
		break;
	case CTestDcmAPICls::LANG_ID_LATIN://-Latin
	{
		std::wstring name1 = L"1stPatName";
		std::wstring name2 = L"2ndPatName";
		ui->lineEdit_2ndName->setText(QString::fromStdWString(name2));
		ui->lineEdit_1stName->setText(QString::fromStdWString(name1));
		//
		ui->lineEdit_ID->setText("123456004");
		//
		std::wstring OpName1 = L"1stOpName";
		std::wstring OpName2 = L"2ndOpName";
		ui->lineEdit_Op_2ndName->setText(QString::fromStdWString(OpName2));
		ui->lineEdit_Op_1stName->setText(QString::fromStdWString(OpName1));
		//
		ui->lineEdit_Comment->setText(QString::fromStdWString(L"Patient Comments"));
		ui->lineEdit_StudyDes->setText(QString::fromStdWString(str_modality) + QString::fromStdWString(L" StudyDes"));
		ui->lineEdit_SeriesDes->setText(QString::fromStdWString(L"SeriesDes"));
		ui->lineEdit_Institutaion->setText(QString::fromStdWString(L"Institutaion"));
		ui->lineEdit_Station->setText(QString::fromStdWString(L"ation"));
	}
	break;
	case CTestDcmAPICls::LANG_ID_CHINESE_TW://-Chinese(TW)
	{
		switch (ModalityId){
		case CTestDcmAPICls::MODALITY_ID_PANO:
			str_modality = L"全景";
			break;
		case CTestDcmAPICls::MODALITY_ID_CEPHA:
			str_modality = L"頭顱側位片";
			break;
		}
	 
		std::wstring name1 = L"中文";
		std::wstring name2 = L"台灣";
		ui->lineEdit_2ndName->setText(QString::fromStdWString(name2));
		ui->lineEdit_1stName->setText(QString::fromStdWString(name1));
		//
		ui->lineEdit_ID->setText("123456005");
		//
		std::wstring OpName1 = L"國民黨";
		std::wstring OpName2 = L"台灣";
		ui->lineEdit_Op_2ndName->setText(QString::fromStdWString(OpName2));
		ui->lineEdit_Op_1stName->setText(QString::fromStdWString(OpName1));
		//
		ui->lineEdit_Comment->setText(QString::fromStdWString(L"病人的描述"));
		//
		ui->lineEdit_StudyDes->setText(QString::fromStdWString(str_modality) + QString::fromStdWString(L"檢查的描述"));
		ui->lineEdit_SeriesDes->setText(QString::fromStdWString(L"SeriesDes描述"));
		ui->lineEdit_Institutaion->setText(QString::fromStdWString(L"牙科診所"));
		ui->lineEdit_Station->setText(QString::fromStdWString(L"診所1"));

	}
	break;
	}
}
void MainWindow::on_pushButton_clicked()
{
	CTestDcmAPICls testDcm;
	QString name = ui->lineEdit_2ndName->text() + "^" + ui->lineEdit_1stName->text();
	QString OpName = ui->lineEdit_Op_2ndName->text() + "^" + ui->lineEdit_Op_1stName->text();
	/* 
	testDcm.setupPatInfo(name, ui->lineEdit_ID->text(),
		OpName, ui->lineEdit_Comment->text());
	*/
	testDcm.m_PatientName = name;
	testDcm.m_PatientID = ui->lineEdit_ID->text();
	testDcm.m_OpName = OpName;
	testDcm.m_PatientComment = ui->lineEdit_Comment->text();
	testDcm.m_StudyDes = ui->lineEdit_StudyDes->text();
	testDcm.m_SeriesDes = ui->lineEdit_SeriesDes->text();
	testDcm.m_InstitutionName = ui->lineEdit_Institutaion->text();
	testDcm.m_StationName = ui->lineEdit_Station->text();
	//////////////////////////////////////////////
	//ƒOƒ[ƒoƒ‹‰Šú‰»

	if (!testDcm.init()){
		printf("init error ---stop!\n");
		return ;
	}
	CTestDcmAPICls::LANG_DEF lang = (CTestDcmAPICls::LANG_DEF)ui->comboBoxLang->currentIndex();
	CTestDcmAPICls::MODALITY_DEF modality = (CTestDcmAPICls::MODALITY_DEF)ui->comboMode->currentIndex();
	bool bIsUTF8 = ui->checkBoxUTF8->isChecked();
	if (lang == CTestDcmAPICls::LANG_ID_CHINESE_TW){
		bIsUTF8 = true;
	}
	int CTVolSize = 64;
	int CTVOL_ID = ui->comboVolSize->currentIndex();
	CTVolSize = 64 * (pow(2, CTVOL_ID));

	testDcm.makeMulitLangDcm(lang, modality, bIsUTF8, CTVolSize);
	// testDcm.doTestCepha();

	//////////////////////////////////////////////
	//‘S‚ÄI—¹
	//////////////////////////////////////////////
	testDcm.destroy();

}
