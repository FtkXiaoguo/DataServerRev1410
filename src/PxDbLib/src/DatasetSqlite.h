/***********************************************************************
 *-------------------------------------------------------------------
 */

#ifndef	__DATASQLITE_h__
#define	__DATASQLITE_h__

 
#include <string>
#include <vector>
#include <map>
#if 0
class QAFieldData
{
public:
	std::string ColName;
	std::string ColVal;
 
};
#endif
//typedef std::map<std::string ,std::string > QARecordData;
typedef std::vector<std::string > QARecordData; ////2012/05/22 K.Ko

//typedef std::vector<QAFieldData> QARecordData;
typedef std::vector<QARecordData> QADataset;

 
class AQDataSetSqlite
{
public:
	int m_curRow;
	int m_curCol;
	QADataset m_dataset;
	 
	std::vector<std::string > m_colNames;
};

#endif //__DATASQLITE_h__