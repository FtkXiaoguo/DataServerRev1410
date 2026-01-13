/****************************************************************************
**
**
****************************************************************************/


#include "PxTagFilterModel.h"

#include <qlistview.h>
#include <qcombobox.h>
//#include <qsqlrelationaltablemodel.h>
#include <QLineEdit>

//#include "PxDBUtil.h"
#include "PxTagFilterManager.h"

#include "QtHelper.h"

#include "PxTagFilterManager.h"

DB_ID_NameList g_CompareatorList;
DB_ID_NameList g_TagNameList;

DB_ID_NameList &CPxTagFilterModel::getCompareatorList()
{
	return g_CompareatorList;
}
DB_ID_NameList &CPxTagFilterModel::getTagNameList()
{
	return g_TagNameList;
}

void CPxTagFilterModel::initDBList()
{
	CPxTagFilterManager TagFilterMan;
	 
	TagFilterMan.getCompareatorList(g_CompareatorList);
	TagFilterMan.getTagNameList(g_TagNameList);
	 
}
#if 0
void CPxTagFilterModel::setupCompareator(std::vector<std::string> list)
{
	g_CompareatorLis.clear();
	if(list.size()<1) return;
	int size = list.size();
	g_CompareatorLis.resize(size);
	for(int i=0;i<size;i++){
		g_CompareatorLis[i] = list[i];
	}
}
void CPxTagFilterModel::setupTagName(std::vector<std::string> list)
{
	g_TagNameList.clear();
	if(list.size()<1) return;
	int size = list.size();
	g_TagNameList.resize(size);
	for(int i=0;i<size;i++){
		g_TagNameList[i] = list[i];
	}
}
#endif

CPxTagFilterModel::CPxTagFilterModel(QObject *parent)
    :QAbstractTableModel(parent)
{

//	addNewEntry();
}

//-----------------------------------------------------------------
int CPxTagFilterModel::rowCount(const QModelIndex & /*parent*/) const
{
	int size = m_FilterEntryList.size();
	return size;
//    return ROWS;
}

//-----------------------------------------------------------------
int CPxTagFilterModel::columnCount(const QModelIndex & /*parent*/) const
{
    return COLS;
}

//-----------------------------------------------------------------
QVariant CPxTagFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
 //       return m_gridData[index.row()][index.column()];
		switch(index.column()){
		case Col_DICOMTAG: //DICOM Tag
			{
				return CPxTagFilterManager::getNameFromList(g_TagNameList,m_FilterEntryList[index.row()].m_TagID).c_str();
				 
			}
			break;
		case Col_COMPARATOR: //Comparator
			{
				return CPxTagFilterManager::getNameFromList(g_CompareatorList,m_FilterEntryList[index.row()].m_ComparatorID).c_str();
			}
			break;
		case Col_FILTER_VAL: //Val
			{
				 
				return m_FilterEntryList[index.row()].m_ValStr.c_str();
				 
			}
			break;
		case Col_AND_OP:
			return " AND ";
		default:
			break;
	 
		}
    }
    return QVariant();
}

//-----------------------------------------------------------------
//! [quoting mymodel_e]
bool CPxTagFilterModel::setData(const QModelIndex & index, const QVariant & value, int role)
{

	if (role == Qt::EditRole)
    {
		switch(index.column()){
		case Col_DICOMTAG: //DICOM Tag
			{
				m_FilterEntryList[index.row()].m_TagID =  value.toInt();
				 
			}
			break;
		case Col_COMPARATOR: //Comparator
			{
				m_FilterEntryList[index.row()].m_ComparatorID =  value.toInt();
			}
			break;
		case Col_FILTER_VAL: //Val
			{
				QString str_temp = value.toString();
				m_FilterEntryList[index.row()].m_ValStr =  QString2Str(str_temp);
				 
			}
			break;
		default:
			break;
	 
		}
		 
	}

    return true;
}
//! [quoting mymodel_e]

//-----------------------------------------------------------------
//! [quoting mymodel_f]
Qt::ItemFlags CPxTagFilterModel::flags(const QModelIndex &  index ) const
{
	if(index.column() == Col_AND_OP){
		return Qt::NoItemFlags;
	}else{
		return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
	}
}
//! [quoting mymodel_f]
static QString _header_Data[COLS] = {
	"Tag",
	"Comparator",
	"Value"
	" ",
	}
;  //
QVariant CPxTagFilterModel::headerData(int section, Qt::Orientation orientation,int role ) const
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


void CPxTagFilterModel::addEntry(const TagFilterEntry &item)
{
#if 0
	TagFilterEntry new_item;
	new_item.m_ComparatorID = 1;
	new_item.m_TagID = 2;
	new_item.m_ValStr = "ttt";
#endif
	m_FilterEntryList.push_back(item);

	
}
void CPxTagFilterModel::removeEntry(QVector<int> &selectRows )
{
	std::vector<TagFilterEntry> new_list_temp;
	std::vector<TagFilterEntry>::iterator it;
	int i=0;
	for(it = m_FilterEntryList.begin(); it!=m_FilterEntryList.end();it++){
		if(selectRows.contains(i)){
			
		}else{
			new_list_temp.push_back(*it);
		}
		 
		i++;
	}


	m_FilterEntryList.clear();

	int new_size = new_list_temp.size();
	if(new_size>0){
		m_FilterEntryList.resize(new_size);
		for(int i=0;i<new_size;i++){
			m_FilterEntryList[i] =  new_list_temp[i];
		}
	}
	
}
void CPxTagFilterModel::clearEntry()
{
	 
	m_FilterEntryList.clear();
	 
}

 bool CPxTagFilterModel::addToDB(bool isAddNew)
 {
	 int size = m_FilterEntryList.size();
	 if(size<1) return false;
	 std::vector<TagRule> tag_filter_rules;
	 tag_filter_rules.resize(size);
	 for(int i=0;i<size;i++){
		 tag_filter_rules[i].ComparatorID	= m_FilterEntryList[i].m_ComparatorID;
		 tag_filter_rules[i].DicomTagID		= m_FilterEntryList[i].m_TagID;
		 strcpy(tag_filter_rules[i].m_value	,m_FilterEntryList[i].m_ValStr.c_str());
	 }

	 PxDBUtil::CPxDBUtil DbUtil;
	 return DbUtil.MakeTagFilter(m_tagFilterName,"testdes",tag_filter_rules,isAddNew/*isAddNew*/);
 }

  
  
  ////////////////////
 

QWidget *QSqlRelationalDelegate::createEditor(QWidget *aParent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
#if 0
    const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    if (!childModel)
        return QItemDelegate::createEditor(aParent, option, index);
#endif
    QWidget *ret_widget = 0;
	switch(index.column()){
		case Col_DICOMTAG: //DICOM Tag
			{
				QComboBox *combo = new QComboBox(aParent);
				for(int i=0;i<g_TagNameList.size();i++){
					combo->addItem(g_TagNameList[i].m_name.c_str());
				}
				ret_widget = combo;
			}
			break;
		case Col_COMPARATOR: //Comparator
			{
				QComboBox *combo = new QComboBox(aParent);
				for(int i=0;i<g_CompareatorList.size();i++){
					combo->addItem(g_CompareatorList[i].m_name.c_str());
				}
				ret_widget = combo;
			}
			break;
		case Col_FILTER_VAL: //Val
			{
				ret_widget = new QLineEdit(aParent);
			}
			break;
		default:
			
			break;
	}
	 
#if 0
    combo->setModel(childModel);
    combo->setModelColumn(childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn()));
    combo->installEventFilter(const_cast<QSqlRelationalDelegate *>(this));
#endif
    return ret_widget;
}

void QSqlRelationalDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
 
    const CPxTagFilterModel *sqlModel = qobject_cast<const CPxTagFilterModel *>(index.model());
     
    if (!sqlModel) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }
  
	switch(index.column()){
		case Col_DICOMTAG: //DICOM Tag
			 
		case Col_COMPARATOR: //Comparator
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
		case Col_FILTER_VAL: //Val
			{
				QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
				if ( !lineEdit) {
					QItemDelegate::setEditorData(editor, index);
					return;
				}else{
					lineEdit->setText(sqlModel->data(index).toString());
				}
				 
			}

		default:
			break;
	}

	 
}

void QSqlRelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

	CPxTagFilterModel *sqlModel = qobject_cast<CPxTagFilterModel *>(model);
    
     
    if (!sqlModel) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }
   
   switch(index.column()){
		case Col_DICOMTAG: //DICOM Tag
			{
				QComboBox *combo = qobject_cast<QComboBox *>(editor);
				if (!combo) {
					QItemDelegate::setModelData(editor, model, index);
					return;
				}else{
					sqlModel->setData(index,
						CPxTagFilterManager::getIDFromList(g_TagNameList,QString2Str( combo->currentText())),
						Qt::EditRole);
				}

	 
			}
			break;
		case Col_COMPARATOR: //Comparator
			{
				QComboBox *combo = qobject_cast<QComboBox *>(editor);
				 
				if (!combo) {
					QItemDelegate::setModelData(editor, model, index);
					return;
				}else{
					sqlModel->setData(index,
						CPxTagFilterManager::getIDFromList(g_CompareatorList,QString2Str( combo->currentText())),
						Qt::EditRole);
				}
						
			}
			break;
		case Col_FILTER_VAL: //Val
			{
				QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
				if ( !lineEdit) {
					QItemDelegate::setEditorData(editor, index);
					return;
				}else{
					sqlModel->setData(index,
						lineEdit->text(),
						Qt::EditRole);
					 
				}
				 
			}
			break;
		default:
			break;
	}
#if 0
    int currentItem = combo->currentIndex();
    int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
    int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
    sqlModel->setData(index,
            childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
            Qt::DisplayRole);
    sqlModel->setData(index,
            childModel->data(childModel->index(currentItem, childEditIndex), Qt::EditRole),
            Qt::EditRole);
#endif
}

