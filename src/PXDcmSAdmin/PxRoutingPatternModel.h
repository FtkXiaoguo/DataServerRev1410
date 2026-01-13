/****************************************************************************
**
**
****************************************************************************/

#ifndef PX_ROUTING_PATTERN_MODEL_H
#define PX_ROUTING_PATTERN_MODEL_H


#include <QAbstractTableModel>
#include <QString>
#include <qitemdelegate.h>
#include "PxTagFilterManager.h"

 #include "PxDcmsAEManage.h"

const int ROUTING_PAT_COLS= 2;
//const int ROWS= 2;


 
 
#define Col_TAG_FILTER  0
#define Col_REMOTE_AE  1
 
 
 
class CPxRoutingPatternModel : public QAbstractTableModel
{
    Q_OBJECT
public:
	CPxRoutingPatternModel(QObject *parent);
	~CPxRoutingPatternModel();
	static void initDBList();
	static void getRemoteAE_IDNameList(DB_ID_NameList &list);
	static AEsMap &getStorageAEListt();
	
	void addEntry(const RoutingPatternEntry &item);
	void removeEntry(QVector<int> &selectRows );
	void clearEntry();

	bool addToDB(bool isAddNew);
	bool readFromDB();
 
	
//	static void setupCompareator(std::vector<std::string> list);
//	static void setupTagName(std::vector<std::string> list);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
	//
	virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

	std::string m_tagFilterName;

	std::vector<RoutingPatternEntry> &getRoutingPatternEntryList() {
		return m_RoutingPatternEntryList;
	};
	bool isRoutingOnShcedule() const { return m_isRoutingOnShcedule;};
	void setRoutingOnShcedule(bool onFlag) { m_isRoutingOnShcedule = onFlag;};
private:
	
 //   QString m_gridData[ROWS][COLS];  //holds text entered into QTableView

	std::vector<RoutingPatternEntry> m_RoutingPatternEntryList;
	bool m_isRoutingOnShcedule;
signals:
    void editCompleted(const QString &);
};
 
////////////
class CPxRoutingPatternDelegate: public QItemDelegate
{
public:

explicit CPxRoutingPatternDelegate(QObject *aParent = 0)
    : QItemDelegate(aParent)
{}

~CPxRoutingPatternDelegate()
{}

QWidget *createEditor(QWidget *aParent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const ;
 

void setEditorData(QWidget *editor, const QModelIndex &index) const;
 

void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
 

};

 

#endif // PX_ROUTING_PATTERN_MODEL_H
