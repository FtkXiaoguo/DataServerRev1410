/****************************************************************************
**
**
****************************************************************************/


#include "PxRoutingPatternModel.h"

#include <qlistview.h>
#include <qcombobox.h>
//#include <qsqlrelationaltablemodel.h>
#include <QLineEdit>

//#include "PxDBUtil.h"
#include "PxTagFilterManager.h"

#include "QtHelper.h"

#include "PxTagFilterManager.h"

#include "PxDcmsAEManage.h"

static AEsMap g_StorageAEList;
extern DB_ID_NameList g_TagFilterList;

AEsMap &CPxRoutingPatternModel::getStorageAEListt()
{
	return g_StorageAEList;
}

void CPxRoutingPatternModel::getRemoteAE_IDNameList(DB_ID_NameList &list)
{
	list.clear();
	int size = g_StorageAEList.size();
	if(size<1) return;
	 
	AEsMap::iterator it = g_StorageAEList.begin();
	while(it!=g_StorageAEList.end()){
		PxDBUtil::DB_ID_Name new_item;
		new_item.m_ID	= it->second.m_AEID;
		new_item.m_name	= it->second.m_AEName;
		list.push_back(new_item);
		it++;
	}
	
}

void CPxRoutingPatternModel::initDBList()
{
	CPxDcmsAEManageRemote  RemoteAEManage;
	 g_StorageAEList.clear();
	 RemoteAEManage.queryAEs(g_StorageAEList ,true /*storageOnly*/ );
	

	CPxTagFilterManager TagFilterMan;
	 
//	TagFilterMan.getCompareatorList(g_CompareatorList);
//	TagFilterMan.getTagNameList(g_TagNameList);
	 
	

}


CPxRoutingPatternModel::CPxRoutingPatternModel(QObject *parent)
    :QAbstractTableModel(parent)
{
	 
//	addNewEntry();
}
CPxRoutingPatternModel::~CPxRoutingPatternModel()
{
	 
	 
}
//-----------------------------------------------------------------
int CPxRoutingPatternModel::rowCount(const QModelIndex & /*parent*/) const
{
	int size = m_RoutingPatternEntryList.size();
	return size;
//    return ROWS;
}

//-----------------------------------------------------------------
int CPxRoutingPatternModel::columnCount(const QModelIndex & /*parent*/) const
{
    return ROUTING_PAT_COLS;
}

//-----------------------------------------------------------------
QVariant CPxRoutingPatternModel::data(const QModelIndex &index, int role) const
{
 
    if (role == Qt::DisplayRole)
    {
 //       return m_gridData[index.row()][index.column()];
		switch(index.column()){
		case Col_TAG_FILTER:  
			{
	 			return CPxTagFilterManager::getNameFromList(g_TagFilterList,m_RoutingPatternEntryList[index.row()].m_tagFilterID).c_str();
				 
			}
			break;
		case Col_REMOTE_AE:  
			{
				DB_ID_NameList AE_list_temp;
				CPxRoutingPatternModel::getRemoteAE_IDNameList(AE_list_temp);
	 			return CPxTagFilterManager::getNameFromList(AE_list_temp,m_RoutingPatternEntryList[index.row()].m_storeTargetID).c_str();
			}
			break;
		
		default:
			break;
	 
		}
    }
 
    return QVariant();
}

//-----------------------------------------------------------------
//! [quoting mymodel_e]
bool CPxRoutingPatternModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
#if 1
 
	if (role == Qt::EditRole)
    {
		switch(index.column()){
		case Col_TAG_FILTER: 
			{
				m_RoutingPatternEntryList[index.row()].m_tagFilterID =  value.toInt();
				 
			}
			break;
		case Col_REMOTE_AE:  
			{
				m_RoutingPatternEntryList[index.row()].m_storeTargetID =  value.toInt();
			}
			break;
		 
		default:
			break;
	 
		}
		 
	}
#endif
    return true;
}
//! [quoting mymodel_e]

//-----------------------------------------------------------------
//! [quoting mymodel_f]
Qt::ItemFlags CPxRoutingPatternModel::flags(const QModelIndex &  index ) const
{
	 
	return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
	 
}
//! [quoting mymodel_f]
static QString _header_Data[ROUTING_PAT_COLS] = {
	"TagFilter",
	"DestinationAE",
	}
;  //
QVariant CPxRoutingPatternModel::headerData(int section, Qt::Orientation orientation,int role ) const
{
	if ((role == Qt::DisplayRole) ) 
    {
		if( orientation==Qt::Horizontal){
			return _header_Data[section];
		}else{
			return section;
		}
    }
    return QVariant();
}


void CPxRoutingPatternModel::addEntry(const RoutingPatternEntry &item)
{
 
	m_RoutingPatternEntryList.push_back(item);
 
}
 
void CPxRoutingPatternModel::removeEntry(QVector<int> &selectRows )
{
	std::vector<RoutingPatternEntry> new_list_temp;
	std::vector<RoutingPatternEntry>::iterator it;
	int i=0;
	for(it = m_RoutingPatternEntryList.begin(); it!=m_RoutingPatternEntryList.end();it++){
		if(selectRows.contains(i)){
			
		}else{
			new_list_temp.push_back(*it);
		}
		 
		i++;
	}


	m_RoutingPatternEntryList.clear();

	int new_size = new_list_temp.size();
	if(new_size>0){
		m_RoutingPatternEntryList.resize(new_size);
		for(int i=0;i<new_size;i++){
			m_RoutingPatternEntryList[i] =  new_list_temp[i];
		}
	}
	
}
void CPxRoutingPatternModel::clearEntry()
{
	 
	m_RoutingPatternEntryList.clear();
	 
}

 bool CPxRoutingPatternModel::addToDB(bool isAddNew)
 {
	 bool ret_b = false;
	 CPxTagFilterManager TagFilterMan;
	 if(isAddNew) {
		ret_b = TagFilterMan.insertRoutingPattern(AUTO_ROUTING_NAME,m_RoutingPatternEntryList);
	 }else{
		ret_b = TagFilterMan.insertRoutingPattern(AUTO_ROUTING_NAME,m_RoutingPatternEntryList);
	 }
 
	 if(ret_b){
		 if(m_isRoutingOnShcedule){
			 ret_b =TagFilterMan.addRoutingSchedule(AUTO_ROUTING_NAME);
  
		 }else{
			ret_b =TagFilterMan.deleteRoutingSchedule(AUTO_ROUTING_NAME);
		 }
	 }

	 return ret_b;
 }
bool CPxRoutingPatternModel::readFromDB()
{
	m_RoutingPatternEntryList.clear();
	CPxTagFilterManager TagFilterMan;
	 bool ret_b = TagFilterMan.getRoutingPattern(AUTO_ROUTING_NAME,m_RoutingPatternEntryList);
	 if(!ret_b){
		 return ret_b;
	 }
	 m_isRoutingOnShcedule = TagFilterMan.isRoutingOnSchedule(AUTO_ROUTING_NAME);
	 return ret_b;
}
  ////////////////////
 

QWidget *CPxRoutingPatternDelegate::createEditor(QWidget *aParent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{

    QWidget *ret_widget = 0;
 
	switch(index.column()){
		case Col_TAG_FILTER: 
			{
				QComboBox *combo = new QComboBox(aParent);
				for(int i=0;i<g_TagFilterList.size();i++){
					combo->addItem(g_TagFilterList[i].m_name.c_str());
				}
				ret_widget = combo;
			}
			break;
		case Col_REMOTE_AE: //Comparator
			{
				DB_ID_NameList AE_list_temp;
				CPxRoutingPatternModel::getRemoteAE_IDNameList(AE_list_temp);
				QComboBox *combo = new QComboBox(aParent);
				for(int i=0;i<AE_list_temp.size();i++){
					combo->addItem(AE_list_temp[i].m_name.c_str());
				}
				ret_widget = combo;
			}
			break;
		
		default:
			
			break;
	}
	 
 
    return ret_widget;
}

void CPxRoutingPatternDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
 
    const CPxRoutingPatternModel *sqlModel = qobject_cast<const CPxRoutingPatternModel *>(index.model());
     
    if (!sqlModel) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }
  
	switch(index.column()){
		case Col_TAG_FILTER: 
			 
		case Col_REMOTE_AE:  
			{
				QComboBox *combo = qobject_cast<QComboBox *>(editor);
				if ( !combo) {
					QItemDelegate::setEditorData(editor, index);
					return;
				}else{
			 		combo->setCurrentIndex(combo->findText(sqlModel->data(index).toString()));
				}
			 
						
			}
			break;
		 

		default:
			break;
	}

	 
}

void CPxRoutingPatternDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

 
	CPxRoutingPatternModel *sqlModel = qobject_cast<CPxRoutingPatternModel *>(model);
    
     
    if (!sqlModel) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }
   
   switch(index.column()){
		case Col_TAG_FILTER:  
			{
				QComboBox *combo = qobject_cast<QComboBox *>(editor);
				if (!combo) {
					QItemDelegate::setModelData(editor, model, index);
					return;
				}else{
					sqlModel->setData(index,
 						CPxTagFilterManager::getIDFromList(g_TagFilterList,QString2Str( combo->currentText())),
						Qt::EditRole);
				}

	 
			}
			break;
		case Col_REMOTE_AE:  
			{
				QComboBox *combo = qobject_cast<QComboBox *>(editor);
				 
				if (!combo) {
					QItemDelegate::setModelData(editor, model, index);
					return;
				}else{
					DB_ID_NameList AE_list_temp;
					CPxRoutingPatternModel::getRemoteAE_IDNameList(AE_list_temp);
					sqlModel->setData(index,
	 					CPxTagFilterManager::getIDFromList(AE_list_temp,QString2Str( combo->currentText())),
						Qt::EditRole);
				}
						
			}
		 
			break;
		default:
			break;
	}
 
 
}
 