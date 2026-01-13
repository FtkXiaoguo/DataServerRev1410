#include "AutoRouting.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>
 
#include "PxTagFilterModel.h"
#include "PxRoutingPatternModel.h"

#include "PxDBUtil.h"
#include "PxTagFilterManager.h"

#include "QtHelper.h"

#include "PXTableWidget.h"
#include "DicomTagList.h"

#include "QMenu.h"

////////////////////////////////////
CMyTableView::CMyTableView(QWidget *parent):QTableView(parent)
{
	m_myMenu = 0;//#49
	m_MyHeaderView = new MyQHeaderView(Qt::Horizontal,parent);

	m_Act_openAddTag = new QAction(tr("Edit TagName List"),this);
	connect(m_Act_openAddTag, SIGNAL(triggered()), this, SLOT(onAddTag()));
}
     
CMyTableView::~CMyTableView()
{
		
}
//#49
void CMyTableView::mousePressEvent(QMouseEvent *e)
{
	Qt::MouseButton button = e->button() ;
    Qt::MouseButtons buttonStates = e->buttons() ;

	if(button == Qt::RightButton){
		onMenu(e->pos());
	}
	QTableView::mousePressEvent(e);
}
//#49
void CMyTableView::onMenu(const QPoint & pos)
{
	if(!m_myMenu) m_myMenu = new QMenu(this);
	m_myMenu->clear();
	 
	m_myMenu->addAction(m_Act_openAddTag);
	 
	QPoint adj_pos = pos + QPoint(5,20);
	 
	m_myMenu->popup(this->mapToGlobal(adj_pos),m_Act_openAddTag); 
}
extern DB_ID_NameList g_TagNameList;
void CMyTableView::onAddTag()
{
	 CDicomTagList addDicomTag;
	 addDicomTag.exec();
	 
	 CPxTagFilterManager TagFilterMan;
	TagFilterMan.getTagNameList(g_TagNameList);
	 
}
void CMyTableView::resetHeaderViewer()
{
	QHeaderView *old_header_view = horizontalHeader();
	setHorizontalHeader(m_MyHeaderView);
//	if(old_header_view) delete old_header_view;

}

bool CMyTableView::updateSelectedList()
{
	m_selectedRows.clear();

 	QModelIndexList sel_list =  selectedIndexes();

 
	int sel_size = sel_list.size();

	 
	/*
	* 行、列の配置順番は選択方法／範囲により、変る。
	*/
	for(int index=0;index<sel_size;	){
		QModelIndex model_index = sel_list.at(index);
		int col = model_index.column();
		int row = model_index.row();
		 
		if(!m_selectedRows.contains(row)){
			m_selectedRows.append(row);
		}
		 
		index ++;
	}
 
	sel_list.clear();

	

	return true;
}
/////////////////////////////////////
MyQHeaderView::MyQHeaderView(Qt::Orientation orientation,QWidget *parent):QHeaderView(orientation,parent)
{
}
MyQHeaderView::~MyQHeaderView()
{
	 
}
 

QSize MyQHeaderView::sectionSizeFromContents(int logicalIndex) const
{
	QSize ret_size = QHeaderView::sectionSizeFromContents( logicalIndex) ;
	ret_size.setHeight(TABLE_W_HEADER_HEIGHT);
	return ret_size;
}
/////////////////////////////////////

#define OP_FIELD_W (40)

DB_ID_NameList g_TagFilterList;

 
DB_ID_NameList g_RoutingPatternList;
#define TagListTabeView (m_tbVFilterList)
#define ARPatternTabView (m_tbVAutoRoutingList)
CAutoRoutingConf::CAutoRoutingConf(QWidget *parent) : TabPageComInf(parent)
{
	
	ui.setupUi(this);
	 
	//m_RemoteAEManage=0;
	m_tagFilterModel = 0;
	m_routingPatternModel = 0;
	
	///////////////////////////
	/// TagFilter TabeView
	ui.LayoutTagFilter->removeWidget(ui.tbVFilterList);
	ui.tbVFilterList->hide();
	// ui.tbVFilterList 使わないこと,
	// 代わりにm_tbVFilterListを使う。
	////////////
	 m_tbVFilterList = new CMyTableView(ui.gTagFilterList);
	 m_tbVFilterList->setObjectName(QString::fromUtf8("MytbVFilterList"));
     ui.LayoutTagFilter->addWidget(m_tbVFilterList, 3, 1, 1, 2);

	 ///////////////////////////
	/// AutoRouting Pattern TabeView
	ui.LayoutTagFilter->removeWidget(ui.tbVAutoRoutingList);
	ui.tbVAutoRoutingList->hide();
	// ui.tbVAutoRoutingList 使わないこと,
	// 代わりにm_tbVFilterListを使う。
	////////////
	 m_tbVAutoRoutingList = new CMyTableView(ui.gARList);
	 m_tbVAutoRoutingList->setObjectName(QString::fromUtf8("MygtbVAutoRoutingList"));
	 ui.LayoutARList->insertWidget(3,m_tbVAutoRoutingList);
	///
	m_curTagFilterViewerWidth = m_refTagFilterViewerWidth = 476;
	m_curRoutingPatternViewerWidth = m_refRoutingPatternViewerWidth = 324;

	 
	
}
CAutoRoutingConf::~CAutoRoutingConf( )
{
	destroy();
	if(m_tagFilterModel) delete m_tagFilterModel;
	if(m_routingPatternModel) delete m_routingPatternModel;
}

void CAutoRoutingConf::init()
{
	m_TagFilterModifyStatus = TagFilter_NoChanged;
	//
//	if(m_RemoteAEManage) delete m_RemoteAEManage;
//	m_RemoteAEManage	= new CPxDcmsAEManageRemote ;
	 

	queryDB();
	

	if(m_tagFilterModel) delete m_tagFilterModel;
	m_tagFilterModel = new CPxTagFilterModel(this);
	m_tagFilterModel->initDBList();
#if 0
	 m_tagFilterModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Tag"));
     m_tagFilterModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Comparator"));
     m_tagFilterModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Value"));
#endif
 
	TagListTabeView->setModel(m_tagFilterModel);
	//
	TagListTabeView->setItemDelegate(new QSqlRelationalDelegate(TagListTabeView));

	{
		if(m_routingPatternModel) delete m_routingPatternModel;
		m_routingPatternModel = new CPxRoutingPatternModel(this);
		m_routingPatternModel->initDBList();
		ARPatternTabView->setModel(m_routingPatternModel);
		ARPatternTabView->setItemDelegate(new CPxRoutingPatternDelegate(TagListTabeView));

	}
	updateTagFilter();
	
	updateRoutingPattern();
}
void CAutoRoutingConf::updateTagFilter()
{
	CPxTagFilterManager TagFilterMan;
	 
	TagFilterMan.getTagFilterNameList(g_TagFilterList);
	ui.cbBTagFilter->clear();
	for(int i=0;i<g_TagFilterList.size();i++){
		ui.cbBTagFilter->addItem(Str2QString(g_TagFilterList[i].m_name));
	}
	updateDisplay();

	///
	TagListTabeView->resetHeaderViewer();
}

bool CAutoRoutingConf::queryDB()
{
#if 0
	if(!m_RemoteAEManage) return false;
	

	m_AEList.clear();
	m_RemoteAEManage->queryAEs(m_AEList);

	ui.cbBDestAE->clear();
	int size = m_AEList.size();


	AEsMap::iterator it = m_AEList.begin();
	while(it!=m_AEList.end()){
		std::string AEName = it->first;
		if(AEName.size()>0){
			ui.cbBDestAE->addItem(AEName.c_str());
		}
		it++;
	}
	 
#endif

	return true;
}
void CAutoRoutingConf::destroy()
{
	int tagFilterSize = m_TagFilterList.size();
	for(int i=0;i<tagFilterSize;i++){
		delete m_TagFilterList[i];
	}
	m_TagFilterList.clear();
}
void CAutoRoutingConf::dispCurrentTagFilter()
{
	int cur_list_no = ui.cbBTagFilter->currentIndex();
	if( (cur_list_no<0) || (cur_list_no>=g_TagFilterList.size())){
		return;
	}

	ui.lEFilterName->setText(Str2QString(g_TagFilterList[cur_list_no].m_name));
	int tag_filer_id = g_TagFilterList[cur_list_no].m_ID;
	//
//	std::vector<TagFilterRule> entry_list;
	std::vector<PxDBUtil::TagFilterRuleTagName> entry_list;
	CPxTagFilterManager TagFilterMan;

	TagFilterMan.getTagFilterRules(entry_list, tag_filer_id);
	m_tagFilterModel->clearEntry();
	for(int i=0;i<entry_list.size();i++){
		TagFilterEntry new_item;
		new_item.m_TagID		= CPxTagFilterManager::getIDFromList(CPxTagFilterModel::getTagNameList(),
										entry_list[i].m_tagName);
		new_item.m_ComparatorID = entry_list[i].m_comparatorID;
		new_item.m_ValStr		= entry_list[i].m_valStr;
		m_tagFilterModel->addEntry(new_item);
	}
 
	updateTagFilterTableViewer();
 
	///
	TagListTabeView->resetHeaderViewer();

}
void CAutoRoutingConf::updateTagFilterTableViewer()
{
	TagListTabeView->setModel( NULL); 
 	TagListTabeView->setModel(m_tagFilterModel);
	//
	TagListTabeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

	updateDisplay();
}
void CAutoRoutingConf::updateRoutingPatternTableViewer()
{
	ARPatternTabView->setModel( NULL); 
	ARPatternTabView->setModel(m_routingPatternModel);
	ARPatternTabView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

	updateDisplay();
}
void CAutoRoutingConf::dispCurrentRoutingPattern()
{
	if(m_routingPatternModel){
		updateRoutingPatternTableViewer();
		//
		ui.chBAutoRouting->setCheckState(
			m_routingPatternModel->isRoutingOnShcedule() ? Qt::Checked : Qt::Unchecked
			);
		 
	}
	///
	ARPatternTabView->resetHeaderViewer();
	
}
void CAutoRoutingConf::addTagFilter(std::string name)
{
	CPxTagFilterModel *new_item = new CPxTagFilterModel(this);
	new_item->m_tagFilterName = name;
	m_TagFilterList.push_back(new_item);
}
 
 void CAutoRoutingConf::onAddFilter()
 {
	 std::string tagFilterName = QString2Str(ui.lEFilterName->text());

	 m_tagFilterModel->m_tagFilterName = tagFilterName;
	 m_tagFilterModel->addToDB(m_TagFilterModifyStatus == TagFilter_AddNew);
	
	 if(m_TagFilterModifyStatus == TagFilter_AddNew){
		 updateTagFilter();
	 }
 }
 void CAutoRoutingConf::onDeleteFilter()
 {
	int cur_list_no = ui.cbBTagFilter->currentIndex();
	if(cur_list_no<0) return;//#40

	std::string tagFilterName  =  g_TagFilterList[cur_list_no].m_name ;
	 PxDBUtil::CPxDBUtil DbUtil;
	 DbUtil.deleteTagFilter(tagFilterName);
 
	 updateTagFilter();
 }
 //
 void CAutoRoutingConf::onAddFilterEntry()
 {
	 TagFilterEntry new_item;
	 m_tagFilterModel->addEntry(new_item);
	 
 
	
 	updateTagFilterTableViewer(); 
 
 
	 
 }

 void CAutoRoutingConf::onApplyFilterEntry()
 {
	 std::string tagFilterName = QString2Str(ui.lEFilterName->text());

	 m_tagFilterModel->m_tagFilterName = tagFilterName;
	 m_tagFilterModel->addToDB(m_TagFilterModifyStatus == TagFilter_AddNew);
	
	 if(m_TagFilterModifyStatus == TagFilter_AddNew){
		 updateTagFilter();
	 }
 }
 void CAutoRoutingConf::onDeleteFilterEntry()
 {
	  
	TagListTabeView->updateSelectedList();
	QVector<int> &selectRows = TagListTabeView->getSelectedRows();
	m_tagFilterModel->removeEntry(selectRows);
 
	updateTagFilterTableViewer(); 
 }
 
 void CAutoRoutingConf::onTagFilterChanged(int index)
 {
	 dispCurrentTagFilter();
 }
 void CAutoRoutingConf::onFilterNameChanged(QString newName)
 {
	 bool modified_flag = true;

	 if(g_TagFilterList.size()>0){
		int cur_list_no = ui.cbBTagFilter->currentIndex();
		if((cur_list_no>=0) && (cur_list_no<g_TagFilterList.size())){
			QString prev_name = Str2QString(g_TagFilterList[cur_list_no].m_name);
			modified_flag = newName != prev_name;
		}
	 }
	if(modified_flag){
		m_TagFilterModifyStatus = TagFilter_AddNew;
		ui.pBAddFilter->setText("Add ");
		ui.pBDeleteFilter->hide();
	}else{
		m_TagFilterModifyStatus = TagFilter_Modify;
		ui.pBAddFilter->setText("Modify ");
		ui.pBDeleteFilter->show();
	}
 }

 void CAutoRoutingConf::updateRoutingPattern()
 {
		 
#if 0
	CPxTagFilterManager TagFilterMan;
	 
	TagFilterMan.getRoutingPattern(g_RoutingPatternList);
	int PatternID = CPxTagFilterManager::getIDFromList(g_RoutingPatternList, AUTO_ROUTING_NAME);
	if(PatternID<0){
	}else{
		
		std::vector<RoutingPatternEntry>  &entries = m_routingPatternModel->getRoutingPatternEntryList();
		bool ret_b = TagFilterMan.getRoutingPattern(AUTO_ROUTING_NAME,entries);
		if(!ret_b){
		}
		
	}
#else
	m_routingPatternModel->readFromDB();
#endif
	dispCurrentRoutingPattern();
 }

 /////////
 void CAutoRoutingConf::onCheckAutoRouting()
 {
	 if(m_routingPatternModel){
		m_routingPatternModel->setRoutingOnShcedule( 
			ui.chBAutoRouting->checkState() ==  Qt::Checked ) ;
	 }
	
 }
 void CAutoRoutingConf::onARPatApply()
 {
	 m_routingPatternModel->addToDB(true/*isAddNew*/);

 }
 void CAutoRoutingConf::onAddRoutingAE()
 {
	 DB_ID_NameList AE_list_temp;
	CPxRoutingPatternModel::getRemoteAE_IDNameList(AE_list_temp);

	 RoutingPatternEntry new_item;
	 if(g_TagFilterList.size()>0){
		new_item.m_tagFilterID		= g_TagFilterList[0].m_ID;
	 }
	 if(AE_list_temp.size()>0){
		 new_item.m_storeTargetID	= AE_list_temp[0].m_ID;
	 }
	 m_routingPatternModel->addEntry(new_item);
	 
  	 updateRoutingPatternTableViewer();
 
 }
 void CAutoRoutingConf::onDeleteRoutingAE()
 {
#if 1
	 ARPatternTabView->updateSelectedList();
	QVector<int> &selectRows = ARPatternTabView->getSelectedRows();
	m_routingPatternModel->removeEntry(selectRows);
 
	updateRoutingPatternTableViewer(); 
#else
	 bool isRoutingOnShcedule = false;
	 {
		CPxTagFilterManager TagFilterMan;
		isRoutingOnShcedule = TagFilterMan.isRoutingOnSchedule(AUTO_ROUTING_NAME);
	  }
	 {
		 CPxTagFilterManager TagFilterMan;
		 TagFilterMan.deleteRoutingSchedule(AUTO_ROUTING_NAME);
	 }

	  if(1){//test it
		  CPxDB g_db;
		  std::vector<RoutingAEInfo>  oVal;
		 
		for(int i=0;i<g_TagFilterList.size();i++){
			g_db.GetRoutingAEInfos(g_TagFilterList[i].m_ID, oVal);
		}

	 }
	  {
		CPxTagFilterManager TagFilterMan;
		TagFilterMan.addRoutingSchedule(AUTO_ROUTING_NAME,time(0),time(0)+20000);
	  }
#endif
 }

 void CAutoRoutingConf::onRefresh()
 {
	updateTagFilter();
	
	updateRoutingPattern();

 }
 void CAutoRoutingConf::resetTagFilterColumnWidth(int w)
 {
	 if(w <100) return ;

	 float chg_rate = (float)(w-OP_FIELD_W)/(m_refTagFilterViewerWidth-OP_FIELD_W);
	 if(chg_rate<0.6) chg_rate = 0.6f;

	 m_curTagFilterViewerWidth = w;

	 

	 int col_index = 0;
	//Tag
	 
	TagListTabeView->setColumnWidth(col_index,200*chg_rate);
	col_index++;

	//Comparator
	TagListTabeView->setColumnWidth(col_index,100*chg_rate);
	col_index++;

	//Value
	TagListTabeView->setColumnWidth(col_index,110*chg_rate);
	col_index++;

	//Op
	TagListTabeView->setColumnWidth(col_index,OP_FIELD_W);
	col_index++;

 }
 void CAutoRoutingConf::resetRoutingPatternColumnWidth(int w)
 {
	 if(w<100) return ;

	 float chg_rate = (float)w/m_refRoutingPatternViewerWidth;
	 if(chg_rate<0.6) chg_rate = 0.6f;

	 m_curRoutingPatternViewerWidth = w;
	 int col_index = 0;

	//TagFilter
	ARPatternTabView->setColumnWidth(col_index,140*chg_rate);
	col_index++;

	//DestinationAE
	ARPatternTabView->setColumnWidth(col_index,160*chg_rate);
	col_index++;

 }
 
 void CAutoRoutingConf::updateDisplay()
 {
	resetTagFilterColumnWidth(m_curTagFilterViewerWidth);
	resetRoutingPatternColumnWidth(m_curRoutingPatternViewerWidth);
 }

 void CAutoRoutingConf::resizeEvent(QResizeEvent *resize_e)
{
	TabPageComInf::resizeEvent( resize_e);

	const QSize &size = resize_e->size();
	 
	;
	int tag_filer_view_w	= TagListTabeView->size().width();
	int rounting_pat_view_w = ARPatternTabView->size().width();

	resetTagFilterColumnWidth(tag_filer_view_w);
	resetRoutingPatternColumnWidth(rounting_pat_view_w);
}

 //////////////////
void CAutoRoutingConf::pageEntry()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));


	m_tagFilterModel->initDBList();
	m_routingPatternModel->initDBList();

	int tag_filer_view_w	= TagListTabeView->size().width();
	int rounting_pat_view_w = ARPatternTabView->size().width();

	resetTagFilterColumnWidth(tag_filer_view_w);
	resetRoutingPatternColumnWidth(rounting_pat_view_w);

	QApplication::restoreOverrideCursor();
	 
}
void CAutoRoutingConf::pageExit()
{
	int x =0;
}

 
/////////////////
 