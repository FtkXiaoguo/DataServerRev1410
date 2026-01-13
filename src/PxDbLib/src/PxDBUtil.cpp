/***********************************************************************
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */
//start from DBToolsWeb.cpp 1.113
#include "PxDBUtil.h"

#include "AqCore/TRPlatform.h"

#include "CheckMemoryLeak.h"

 
using namespace std;

using namespace PxDBUtil;

CPxDBUtil::CPxDBUtil()
{
}

CPxDBUtil::~CPxDBUtil()
{
}

bool CPxDBUtil::addTagName(unsigned long iTag,const std::string &name)//#49
{
	bool ret_b = true;
 
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("INSERT DicomTag (Tag,TagString) VALUES(%d,'%s')", 
		iTag, name.c_str());

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		ret_b = false;
	} 

 	SQLExecuteEnd(sqa);	

	return ret_b;

}
bool CPxDBUtil::deleteTagName(unsigned long iTag )//#49
{
	bool ret_b = true;
	
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("DELETE FROM DicomTag WHERE Tag=%d ", 
		iTag );
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		ret_b = false;
	} 

 	SQLExecuteEnd(sqa);	


	return ret_b;
}
bool CPxDBUtil::getTagListName(std::vector<DB_ID_NameEx> &idname_list)
{
	bool ret_b = true;
	AqString iSQLStr = "SELECT * FROM DicomTag";

	int size;
	int retcd;
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(iSQLStr);

	if ((retcd = SQLExecuteBegin(sqa)) != kOK){
		ret_b = false;
	}

	if(ret_b){
		if ((size = sqa.GetRecordCount()) < 1){
			ret_b = false;
		}
		if(ret_b){
			if ((retcd = sqa.MoveFirst()) != kOK) {
				ret_b = false;
			}
		}
	}
	idname_list.clear();
	char _str_buf[1024];
	if(ret_b){
		idname_list.resize(size);
		for(int i=0;i<size;i++){
			DB_ID_NameEx new_item;

			SQL_GET_INT(new_item.m_ID, sqa);
			SQL_GET_INT(new_item.m_TAG, sqa);
			SQL_GET_STR(_str_buf, sqa);
			new_item.m_name = _str_buf;
			idname_list[i] = new_item;

			if ((retcd = sqa.MoveNext()) != kOK) {
				ret_b = false;
				break;
			}
		}
	}

	SQLExecuteEnd(sqa);	

	return ret_b;
}
bool CPxDBUtil::getIDListName(IdListName TableName,std::vector<DB_ID_Name> &idname_list)
{
	bool ret_b = false;

	 AqString iSQLStr = "";

	 ret_b = true;
    switch (TableName){
        case IdList_LocalAE:
            iSQLStr = "SELECT ID, AEName + '/'+ AETitle  FROM LocalAE ORDER BY AEName";
            break;
            
        case IdList_RemoteAE:
            iSQLStr = "SELECT ID, AEName + '/'+ AETitle  FROM RemoteAE ORDER BY AEName";
            break;
        case IdList_UserGroup:
            iSQLStr = "SELECT UserGroupID, Name FROM UserGroup ORDER BY Name";
            break;
            
        case IdList_StoreTargetAE:
            iSQLStr = "SELECT ID, AEName FROM RemoteAE INNER JOIN StoreTargetAE ON storeTargetAE.AETitleID = RemoteAE.ID ORDER BY AEName";
            break;
        case IdList_QRSourceAE:
            iSQLStr = "SELECT ID, AEName FROM RemoteAE INNER JOIN QRSourceAE ON QRSourceAE.AETitleID = RemoteAE.ID ORDER BY AEName";
            break;
        case IdList_RoutingTargetGroup:
            iSQLStr = "SELECT ID, Name FROM RoutingTargetGroup ORDER BY Name";
            break;
        case IdList_RoutingPattern:
            iSQLStr = "SELECT ID, Name FROM RoutingPattern WHERE name <>'APS outputs routing pattern' ORDER BY Name";
            break;
        case IdList_Schedule:
            iSQLStr = "SELECT ID, Name FROM Schedule ORDER BY Name";
            break;
        case IdList_TagFilter:
            iSQLStr = "SELECT ID, Name FROM TagFilter WHERE name not like '%output filter' AND Description <>'Do not delete please' ORDER BY Name ";
            break;
        case IdList_DicomTag:
            iSQLStr = "SELECT ID, TagString FROM DicomTag";
            break;
        case IdList_Comparator:
            iSQLStr = "SELECT ID, OpString FROM Comparator";
            break;
        case IdList_Printer:
            iSQLStr = "SELECT ID, Name FROM Printer";
            break;
        case IdList_Organization:
            iSQLStr = "SELECT OrganizationID, Name FROM Organization";
            break;
        case IdList_Domain:
//                iSQLStr = "SELECT DomainID, Name FROM DomainT WHERE Type = " + Domain.TYPE_ADIS;
			ret_b = false;
            break;
        case IdList_DataProcessJob:
            iSQLStr = "SELECT ID, JobName FROM DataProcessJob Order by JobName";
            break;
        case IdList_DataProcessor:
            iSQLStr = "SELECT ID, ProcessName FROM DataProcessor Order by ProcessName";
            break;
        default:
			ret_b = false;
			break;
             
    }
	if(!ret_b) {
		return ret_b;
	}

	int size;
	int retcd;
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(iSQLStr);

	if ((retcd = SQLExecuteBegin(sqa)) != kOK){
		ret_b = false;
	}

	if(ret_b){
		if ((size = sqa.GetRecordCount()) < 1){
			ret_b = false;
		}
		if(ret_b){
			if ((retcd = sqa.MoveFirst()) != kOK) {
				ret_b = false;
			}
		}
	}
	idname_list.clear();
	char _str_buf[1024];
	if(ret_b){
		idname_list.resize(size);
		for(int i=0;i<size;i++){
			DB_ID_Name new_item;

			SQL_GET_INT(new_item.m_ID, sqa);
			SQL_GET_STR(_str_buf, sqa);
			new_item.m_name = _str_buf;
			idname_list[i] = new_item;

			if ((retcd = sqa.MoveNext()) != kOK) {
				ret_b = false;
				break;
			}
		}
	}

	SQLExecuteEnd(sqa);	

	return ret_b;
}

bool CPxDBUtil::insertIDListName(IdListName TableName,std::string nameVal)
{
	bool ret_b = false;

	 AqString iSQLStr = "";

	 ret_b = true;

	 switch (TableName){
        case IdList_LocalAE:
            iSQLStr = "SELECT ID, AEName + '/'+ AETitle  FROM LocalAE ORDER BY AEName";
            break;
            
        case IdList_RemoteAE:
            iSQLStr = "SELECT ID, AEName + '/'+ AETitle  FROM RemoteAE ORDER BY AEName";
            break;
        case IdList_UserGroup:
            iSQLStr = "SELECT UserGroupID, Name FROM UserGroup ORDER BY Name";
            break;
            
        case IdList_StoreTargetAE:
            iSQLStr = "SELECT ID, AEName FROM RemoteAE INNER JOIN StoreTargetAE ON storeTargetAE.AETitleID = RemoteAE.ID ORDER BY AEName";
            break;
        case IdList_QRSourceAE:
            iSQLStr = "SELECT ID, AEName FROM RemoteAE INNER JOIN QRSourceAE ON QRSourceAE.AETitleID = RemoteAE.ID ORDER BY AEName";
            break;
        case IdList_RoutingTargetGroup:
            iSQLStr = "SELECT ID, Name FROM RoutingTargetGroup ORDER BY Name";
            break;
        case IdList_RoutingPattern:
            iSQLStr = "SELECT ID, Name FROM RoutingPattern WHERE name <>'APS outputs routing pattern' ORDER BY Name";
            break;
        case IdList_Schedule:
            iSQLStr = "SELECT ID, Name FROM Schedule ORDER BY Name";
            break;
        case IdList_TagFilter:
            iSQLStr = "INSERT TagFilter (Name) VALUES('%s')";
            break;
        case IdList_DicomTag:
            iSQLStr = "SELECT ID, TagString FROM DicomTag";
            break;
        case IdList_Comparator:
            iSQLStr = "SELECT ID, OpString FROM Comparator";
            break;
        case IdList_Printer:
            iSQLStr = "SELECT ID, Name FROM Printer";
            break;
        case IdList_Organization:
            iSQLStr = "SELECT OrganizationID, Name FROM Organization";
            break;
        case IdList_Domain:
//                iSQLStr = "SELECT DomainID, Name FROM DomainT WHERE Type = " + Domain.TYPE_ADIS;
			ret_b = false;
            break;
        case IdList_DataProcessJob:
            iSQLStr = "SELECT ID, JobName FROM DataProcessJob Order by JobName";
            break;
        case IdList_DataProcessor:
            iSQLStr = "SELECT ID, ProcessName FROM DataProcessor Order by ProcessName";
            break;
        default:
			ret_b = false;
			break;
             
    }
	if(!ret_b) {
		return ret_b;
	}

	int retcd;
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText(iSQLStr,nameVal.c_str());

	if ((retcd = SQLExecuteBegin(sqa)) != kOK){
		ret_b = false;
	}
	 
	SQLExecuteEnd(sqa);	

	return ret_b;
}
	
bool CPxDBUtil::deleteIDListName(IdListName TableName,std::string nameVal)
{
	bool ret_b = false;

	 AqString iSQLStr = "";

	 ret_b = true;

	return ret_b;
}
bool CPxDBUtil::MakeTagFilter(const std::string &name, const std::string &description,const std::vector<TagRule> &rules,bool isAddNew)
{
	bool ret_b = false;

	 AqString iSQLStr = "";

	 ret_b = true;

	 SQA sqa(getLocalDBType());
	 try {
		 // Try to insert into or modify tagFilter table and get the ID 
		
		AqString selectStr;
		selectStr.Format("SELECT ID FROM TagFilter WHERE Name = '%s'", name.c_str());
		if(isAddNew)
			sqa.FormatCommandText("IF NOT EXISTS (%s) INSERT INTO TagFilter (Name, Description) values('%s', '%s')", selectStr, name.c_str(),  description.c_str());
		else {
			sqa.FormatCommandText(" UPDATE TagFilter SET Description = '%s' WHERE Name = '%s'", description.c_str(), name.c_str());
		}

		int retcd = SQLExecuteBegin(sqa);  
		if(retcd != kOK) {
			 throw(-1);
		}
		//
		sqa.FormatCommandText("%s",selectStr);
		retcd = SQLExecuteBegin(sqa);  
		if(retcd != kOK || sqa.MoveFirst()!= kOK) {
			throw(-1);;
		}
		int filterID = sqa.getDataInt();

		// Clear entries in TagFilterRules if it is modification
		if(!isAddNew) {
			sqa.FormatCommandText("DELETE FROM TagFilterRules WHERE TagFilterID = %d",filterID);
			retcd = SQLExecuteBegin(sqa); 
			if(retcd != kOK) {
				throw(-1);
			};
		}

		// loop (to insert TagRule and get ID for that rule, insert to TagFilterRules )
		int ruleID = -1;
		for(int i=0; i<rules.size(); i++)
		{
			selectStr.Format(" SELECT ID FROM TagRule WHERE DicomTagID = %d AND ComparatorID = %d AND value = '%s'", rules[i].DicomTagID, rules[i].ComparatorID, rules[i].m_value);
			sqa.FormatCommandText("IF NOT EXISTS (%s) INSERT INTO TagRule (DicomTagID, ComparatorID, Value) VALUES( %d, %d, '%s')",selectStr, rules[i].DicomTagID, rules[i].ComparatorID, rules[i].m_value);
			retcd = SQLExecuteBegin(sqa);  
			if(retcd != kOK) {
				throw(-1);
			};
			sqa.FormatCommandText("%s", selectStr);
			retcd = SQLExecuteBegin(sqa); 
			if(retcd != kOK || sqa.MoveFirst()!= kOK) {
				throw(-1);
			} 
			ruleID = sqa.getDataInt();
			sqa.FormatCommandText("IF NOT EXISTS (SELECT TagFilterID FROM tagFilterRules WHERE TagFilterID = %d AND TagRuleID = %d) "\
				" INSERT INTO tagFilterRules (TagFilterID, TagRuleID) VALUES( %d, %d)",filterID, ruleID, filterID, ruleID);
			retcd = SQLExecuteBegin(sqa);  
			if(retcd != kOK) {
				throw(-1);
			} 
		}	
		// if it is update, clean up tagrule table. status check is not needed
		if(!isAddNew) {
			sqa.FormatCommandText("%s", "Delete from tagRule where ID NOT in (SELECT TagRuleID from tagFilterRules )");
			SQLExecuteBegin(sqa); 
		}
	}catch(int errorID){
		ret_b = false;
	}
	SQLExecuteEnd(sqa);
	return ret_b;
}

bool CPxDBUtil::getTagFilterRulesWithTagName(std::vector<TagFilterRuleTagName>& oVal, const char* iWhereFilter)
{
	AqString	strSQL = "SELECT F.TagFilterID, D.TagString, C.ID, R.Value FROM TagRule R "\
	" JOIN DicomTag D ON R.DicomTagID = D.ID "\
	" JOIN Comparator C ON R.ComparatorID = C.ID "\
	" JOIN TagFilterRules F ON F.TagRuleID = R.ID "\
//	" ORDER BY F.TagFilterID "; //2012/05/09 K.Ko for iWhereFilter
	" ";
	if(iWhereFilter) strSQL += iWhereFilter;

	strSQL += " ORDER BY F.TagFilterID "; //2012/05/09 K.Ko for iWhereFilter
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDBUtil::getTagFilterRulesWithTagName with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); 
	if(retcd != kOK) {
		return false;
	}
	int size = sqa.GetRecordCount(); 
	if(size < 1) {
		return true;
	}
	oVal.resize(size);

	TagFilterRuleTagName* pRule;
	int index = 0;
	retcd = sqa.MoveFirst(); 
	if(retcd != kOK) {
		return false;
	}
	char tag_name_buff[256];
	char val_str_buff[1024];
	while( retcd == kOK && index < size )
	{
		pRule = &(oVal[index++]);

		SQL_GET_INT(pRule->m_filterID,sqa);

		tag_name_buff[0] = 0;
		SQL_GET_STR(tag_name_buff,sqa);//pRule->m_tag,sqa);
		pRule->m_tagName = tag_name_buff;
		SQL_GET_INT(pRule->m_comparatorID,sqa);

		val_str_buff[0] = 0;
		SQL_GET_STR(val_str_buff,sqa);//pRule->m_value,sqa);
		pRule->m_valStr	= val_str_buff;
			
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	 
	
	return true;
}
bool CPxDBUtil::deleteTagFilter(const std::string &name)
{
	AqString strSQL1;
	strSQL1.Format(
				"DELETE TagRule From TagRule "
				" JOIN TagFilterRules as A On A.TagRuleID = TagRule.ID "
				" JOIN TagFilter  as B On A.TagFilterID = B.ID "
				" WHERE B.Name = '%s'",
				name.c_str());

 
	//
	AqString strSQL2;
	strSQL2.Format(
				"DELETE TagFilterRules From TagFilterRules "
				" JOIN TagFilter  as A On TagFilterRules.TagFilterID = A.ID "
				" WHERE A.Name = '%s'",
				name.c_str());
	//
	AqString strSQL3;
	strSQL3.Format(
				"DELETE TagFilter From TagFilter "
				" WHERE Name = '%s'",
				name.c_str());
 

	AqString strSQL = strSQL1 + strSQL2 + strSQL3;
 

	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	 
	if(retcd != kOK) {
		return false;
	}

	SQLExecuteEnd(sqa);
	return true;
}

bool CPxDBUtil::modifyRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries)
{
#if 0
        String name = DoubleSingleQuotation(iRP.getName());
        String sqlStr = "SELECT ID FROM RoutingPattern WHERE name = '" + name +"' " ;
        int patternID = NativeGetIntValue(sqlStr);
        if(patternID <0) throw new IOException("Failed to get ID for pattern name: " + name );
        StringBuffer modifyEntry = new StringBuffer();
        modifyEntry.append("DELETE FROM TagBasedRoutingPatternEntry  WHERE RoutingPatternID = " + patternID);
        modifyEntry.append(MakeInsertSQLStr(patternID,  iRP.getRoutingPatternEntries()));
        return SQLExecute(modifyEntry.toString());
#endif
		return true;
}
    //--------------------------------------------------------------------------
    //
bool CPxDBUtil::insertRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries)
 {
        
	AqString strSQL_ID;
	strSQL_ID.Format(
			"SELECT ID FROM RoutingPattern WHERE name = '%s'",
			name.c_str());

	AqString insert2RoutingPattern;
	insert2RoutingPattern.Format(" INSERT INTO RoutingPattern (Name) values('%s') ",
			name.c_str());
	insert2RoutingPattern = " IF NOT EXISTS (" + strSQL_ID +")" + insert2RoutingPattern;
 
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(insert2RoutingPattern);
	int retcd = SQLExecuteBegin(sqa);

	if(retcd != kOK) {
		return false;
	}
	SQLExecuteEnd(sqa);
	//
	// get patternID
    int patternID ;
	retcd= SQLGetInt(strSQL_ID,patternID);
	if(retcd != kOK) {
		return false;
	}
    
    if(patternID < 0 ) {
		AqString deleteSQLStr;
		deleteSQLStr.Format(" DELETE FROM RoutingPattern WHERE Name = '%d' ",
					name.c_str());
		sqa.SetCommandText(deleteSQLStr);
        retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) {
			return false;
		}
		SQLExecuteEnd(sqa);
	}else{
		 AqString insert2RountingPatternEntrySQLStr;
         insert2RountingPatternEntrySQLStr.Format(
			 " Delete FROM TagBasedRoutingPatternEntry  WHERE RoutingPatternID = %d",
					patternID );
		 AqString insertEntrySQLStr;
		 int entry_size = entries.size();
		 for(int i=0;i<entry_size;i++){
			insertEntrySQLStr.Format(" IF NOT EXISTS (SELECT RoutingPatternID FROM TagBasedRoutingPatternEntry "
									"WHERE "
									" RoutingPatternID = %d "
									" AND TagFilterID = %d "
									" AND StoreTargetID = %d "
									" AND CompressionMethod = %d "
									" AND compressionFactor = %d "
									")"
									" INSERT INTO TagBasedRoutingPatternEntry "
									" ("
									"RoutingPatternID, "
									"TagFilterID, "
									"storeTargetID, " 
									"compressionMethod, "
									"compressionFactor "
									")"
									" Values( "
									"%d,"		//RoutingPatternID
									"%d,"		//TagFilterID
									"%d,"		//storeTargetID
									"%d,"		//compressionMethod
									"%d"		//compressionFactor
									") ",
									//WHERE
									patternID,//entries[i].m_routingPatternID,
									entries[i].m_tagFilterID,
									entries[i].m_storeTargetID,
									entries[i].m_compressionMethod,
									entries[i].m_compressionFactor,
									//Values
									patternID,//entries[i].m_routingPatternID,
									entries[i].m_tagFilterID,
									entries[i].m_storeTargetID,
									entries[i].m_compressionMethod,
									entries[i].m_compressionFactor
									);
             insert2RountingPatternEntrySQLStr = insert2RountingPatternEntrySQLStr + insertEntrySQLStr;
		 }
                 
        sqa.SetCommandText(insert2RountingPatternEntrySQLStr);
        retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) {
			return false;
		}   
		SQLExecuteEnd(sqa);
	}
#if 0
        String name = DoubleSingleQuotation(iRP.getName());
        String sqlStr = "SELECT ID FROM RoutingPattern WHERE name = '" + name +"' " ;
        StringBuffer insert2RoutingPattern =  new StringBuffer();
        insert2RoutingPattern.append(" IF NOT EXISTS (" + sqlStr + ")");
        insert2RoutingPattern.append(" INSERT INTO RoutingPattern (Name) values('" + name +"')");
        
        if(SQLExecute(insert2RoutingPattern.toString())){
            // get patternID
            int patternID = NativeGetIntValue(sqlStr);
            String deleteSQLStr = " DELETE FROM RoutingPattern WHERE Name = '" + name +"' ";
            if(patternID < 0 ) {
                SQLExecute(deleteSQLStr);
                return false;
            }
            else {
                
                StringBuffer insert2RountingPatternEntrySQLStr =  new StringBuffer();
                insert2RountingPatternEntrySQLStr.append(" Delete FROM TagBasedRoutingPatternEntry  WHERE RoutingPatternID = " + patternID );
                insert2RountingPatternEntrySQLStr.append(MakeInsertSQLStr(patternID,  iRP.getRoutingPatternEntries()));
                boolean ret = SQLExecute(insert2RountingPatternEntrySQLStr.toString());
                initAqnetDefault7x24Schedule();
                if (ret == false || m_aqnetDefault7x24Schedule <0 ){
                    SQLExecute(deleteSQLStr);
                    return false;
                }
                
                StringBuffer pSSQLStr = new StringBuffer();
                pSSQLStr.append("IF NOT EXISTS (SELECT * FROM RoutingSchedule WHERE ScheduleID = ");
                pSSQLStr.append(m_aqnetDefault7x24Schedule +" AND RoutingPatternID = " +  patternID );
                pSSQLStr.append(") insert into RoutingSchedule (ScheduleID, RoutingPatternID, repeat) Values (");
                pSSQLStr.append(m_aqnetDefault7x24Schedule + ", " + patternID +", 1)" );
                ret = SQLExecute(pSSQLStr.toString());
                if (ret == false){
                    SQLExecute(deleteSQLStr);
                    return false;
                }
                return true;
            }
        }
        else return false;
#endif
		return true;
  
}
    

bool CPxDBUtil::getRoutingPattern(const std::string &name,std::vector<RoutingPatternEntry> &entries)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDBUtil::getRoutingPattern start\n");
	
	entries.clear();  

	 
	AqString	strSQL = "";

	strSQL.Format(
				"SELECT "
				"RoutingPatternID, "
				"TagFilterID, "
				"storeTargetID, " 
				"compressionMethod, "
				"compressionFactor "
				"FROM TagBasedRoutingPatternEntry "
				"JOIN RoutingPattern ON "
				"TagBasedRoutingPatternEntry.RoutingPatternID = RoutingPattern.ID "
				"WHERE RoutingPattern.Name = '%s' ",
				name.c_str());
				  
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	if (retcd != kOK) {
		return false;
	}

	int size = sqa.GetRecordCount(); 
	if (size >0)
	{
		entries.resize(size);

		retcd = sqa.MoveFirst(); 
		if(retcd != kOK)  {
			return false;
		}
		int index = 0;
		while( retcd == kOK )
		{
			RoutingPatternEntry *pEntry = &(entries[index++]);

			SQL_GET_INT(pEntry->m_routingPatternID, sqa);
			SQL_GET_INT(pEntry->m_tagFilterID, sqa);
			SQL_GET_INT(pEntry->m_storeTargetID, sqa);
			
			SQL_GET_INT(pEntry->m_compressionMethod, sqa);
			SQL_GET_INT(pEntry->m_compressionFactor, sqa);
			 
			retcd = sqa.MoveNext();
		}
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DEBUG: -CPxDBUtil::getRoutingPattern end\n");
	return true;

}
#ifndef TimeStrFormat
#define TimeStrFormat "%04d/%02d/%02d %02d:%02d:%02d"
#endif

bool CPxDBUtil::addRoutingSchedule(const std::string &name,long startDate,long endDate)
{

	AqString strSQL_ID;
	strSQL_ID.Format(
			"SELECT ID FROM RoutingPattern WHERE name = '%s'",
			name.c_str());
 
	//
	// get patternID
    int patternID ;
	int retcd= SQLGetInt(strSQL_ID,patternID);
	if(retcd != kOK) {
		return false;
	}
	if(patternID < 0 ) {
		return false;
	}

	AqString  strSQL;

	/*
	* Šù‚É‘¶Ý‚µ‚½ê‡‚Íæ‚Éíœ
	*/
	strSQL.Format(
			 " Delete FROM RoutingSchedule  WHERE RoutingPatternID = %d ",
					patternID ); 

	strSQL += " INSERT INTO RoutingSchedule ";
	AqString  strSQL_sub;
	if(startDate<0){
		strSQL_sub.Format(
						" ( ScheduleID ,RoutingPatternID  ) Values(1, %d) ",
						patternID); 
	}else{
		if(endDate<0){
			time_t time_start = startDate;
			struct tm tm_start_temp = *localtime(&time_start);
			strSQL_sub.Format(
						" ( ScheduleID ,RoutingPatternID , StartDate ) Values(1, %d ,'"
						TimeStrFormat
						"')",
						patternID,
						///////
						tm_start_temp.tm_year + 1900,
						tm_start_temp.tm_mon + 1,
						tm_start_temp.tm_mday,
						tm_start_temp.tm_hour,
						tm_start_temp.tm_min,
						tm_start_temp.tm_sec
						/////////
						); 
		}else{
			time_t time_start = startDate;
			struct tm tm_start_temp = *localtime(&time_start);
			//
			time_t time_end = endDate;
			struct tm tm_end_temp = *localtime(&time_end);

			strSQL_sub.Format(
						" ( ScheduleID ,RoutingPatternID , StartDate, EndDate) Values(1, %d ,'"
						TimeStrFormat
						"','"
						TimeStrFormat
						"')",
						patternID,
						///////
						tm_start_temp.tm_year + 1900,
						tm_start_temp.tm_mon + 1,
						tm_start_temp.tm_mday,
						tm_start_temp.tm_hour,
						tm_start_temp.tm_min,
						tm_start_temp.tm_sec,
						////////
						tm_end_temp.tm_year + 1900,
						tm_end_temp.tm_mon + 1,
						tm_end_temp.tm_mday,
						tm_end_temp.tm_hour,
						tm_end_temp.tm_min,
						tm_end_temp.tm_sec
						/////////
						); 
		}
	}
	strSQL += strSQL_sub;

	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	retcd = SQLExecuteBegin(sqa);

	if(retcd != kOK) {
		return false;
	}
	SQLExecuteEnd(sqa);

	return true;

}
bool CPxDBUtil::deleteRoutingSchedule(const std::string &name)
{
	AqString strSQL;
	strSQL.Format(
			 " DELETE RoutingSchedule FROM RoutingSchedule "
			 "JOIN RoutingPattern ON RoutingPattern.ID = RoutingPatternID "
			 " WHERE RoutingPattern.Name = '%s'",
				name.c_str());
	//
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);

	if(retcd != kOK) {
		return false;
	}
	SQLExecuteEnd(sqa);

	return true;
}
bool CPxDBUtil::isRoutingOnSchedule(const std::string &name)
{
	AqString strSQL_ID;
	strSQL_ID.Format(
			 " SELECT RoutingPatternID FROM RoutingSchedule "
			 "JOIN RoutingPattern ON RoutingPattern.ID = RoutingPatternID "
			 " WHERE RoutingPattern.Name = '%s'",
				name.c_str());
	//
	// get patternID
    int patternID =-1;
	int retcd= SQLGetInt(strSQL_ID,patternID);
	if(retcd != kOK) {
		return false;
	}
	if(patternID < 0 ) {
		return false;
	}else{
		return true;
	}
}