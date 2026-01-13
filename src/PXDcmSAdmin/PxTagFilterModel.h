/****************************************************************************
**
**
****************************************************************************/

#ifndef PXTAGFILTER_MODEL_H
#define PXTAGFILTER_MODEL_H


#include <QAbstractTableModel>
#include <QString>
#include <qitemdelegate.h>
#include "PxTagFilterManager.h"

const int COLS= 4;
//const int ROWS= 2;


class TagFilterEntry
{
public:
	int m_TagID;
	int m_ComparatorID;
	std::string m_ValStr;
 
};
#define Col_DICOMTAG 0
#define Col_COMPARATOR 1
#define Col_FILTER_VAL 2
#define Col_AND_OP 3

class CPxTagFilterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
	static void initDBList();
	static DB_ID_NameList &getCompareatorList();
	static DB_ID_NameList &getTagNameList();

	void addEntry(const TagFilterEntry &item);
 	void removeEntry(QVector<int> &selectRows );
	void clearEntry();

	bool addToDB(bool isAddNew);
 
//	static void setupCompareator(std::vector<std::string> list);
//	static void setupTagName(std::vector<std::string> list);
    CPxTagFilterModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
	//
	virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

	std::string m_tagFilterName;

	 
private:
 //   QString m_gridData[ROWS][COLS];  //holds text entered into QTableView

	std::vector<TagFilterEntry> m_FilterEntryList;
	
signals:
    void editCompleted(const QString &);
};
 
class QSqlRelationalDelegate: public QItemDelegate
{
public:

explicit QSqlRelationalDelegate(QObject *aParent = 0)
    : QItemDelegate(aParent)
{}

~QSqlRelationalDelegate()
{}

QWidget *createEditor(QWidget *aParent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const ;
 

void setEditorData(QWidget *editor, const QModelIndex &index) const;
 

void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;


};

 
#endif // PXTAGFILTER_MODEL_H
