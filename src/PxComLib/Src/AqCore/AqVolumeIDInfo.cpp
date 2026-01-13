/***********************************************************************
 * AqVolumeIDInfo.h
 *---------------------------------------------------------------------
 * 
 */

#include "AqVolumeIDInfo.h"
#include "Windows.h"
#include "TRCryptor.h"

using namespace std;

static const char* InputVolumeInfoFile = "InputVolumeInfo.txt";
static const char* OutputVolumeInfoFile = "OutputVolumeInfo.txt";

static const char* StringVersion_1_0 = "Version: 1.0";
static const char* VolumeMark_1_0 = "//// Volume: ";
static const int SizeofVolumeMark_1_0 = strlen("//// Volume: ");

bool AqVolumeIDInfo::WriteVolumeIDInfo(const AqVolumeIDInfoList& iVal, bool iInput, const char* iPath)
{
	FILE* fp;
	AqString filePath;
	const char* filename = (iInput)?InputVolumeInfoFile:OutputVolumeInfoFile;


	if(iPath && iPath[0])
	{
		filePath = iPath;
		filePath = filePath + "/" + filename;
	}
	else
	{
		filePath = filename;
	}
		

	if ((fp = fopen(filePath, "w")) == 0)
	{
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::WriteVolumeIDInfo fail to open file %s \n",
									filePath);
		return false;
	}
	
	fprintf(fp, "%s\n", StringVersion_1_0);
	int i, j;
	for(i=0; i<iVal.Size();i++)
	{
		const AqVolumeIDInfo& volume = iVal[i];
		fprintf(fp, "%s%d\n", VolumeMark_1_0, i);
		fprintf(fp, "%s\n", volume.m_studyUID);
		fprintf(fp, "%s\n", volume.m_seriesUID);
		fprintf(fp, "%s\n", volume.m_dataType);
		fprintf(fp, "%d\n", volume.m_sortOrder);
		fprintf(fp, "%d\n", volume.m_instanceCount);

		if(volume.m_instances.size() != volume.m_instanceCount)
		{
			GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::WriteVolumeIDInfo instance list size error(%d, %d)\n",
				volume.m_instanceCount, volume.m_instances.size());
			fclose(fp);
			return false;
		}

		for(j=0; j<volume.m_instanceCount; j++)
		{
			fprintf(fp, "%s\n", volume.m_instances[j]);
		}
	}
	
	fclose(fp);
	return true;
}

#if 0
bool AqVolumeIDInfo::WriteVolumeIDInfo(const AqVolumeIDInfoList& iVal1, std::vector< const std::vector< const char* >  * >& iVal2, 
									   bool iInput, const char* iPath)
{
	FILE* fp;
	AqString filePath;
	const char* filename = (iInput)?InputVolumeInfoFile:OutputVolumeInfoFile;

	if(iVal1.size() != iVal2.size())
	{
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::WriteVolumeIDInfo input list size error\n");
		return false;
	}

	if(iPath && iPath[0])
	{
		filePath = iPath;
		filePath = filePath + "/" + filename;
	}
	else
	{
		filePath = filename;
	}
		

	if ((fp = fopen(filePath, "w")) == 0)
	{
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::WriteVolumeIDInfo fail to open file %s \n",
									filePath);
		return false;
	}
	
	fprintf(fp, "%s\n", StringVersion_1_0);
	int i, j;
	for(i=0; i<iVal1.size();i++)
	{
		const AqVolumeIDInfo& volume = iVal1[i];
		fprintf(fp, "%s%d\n", VolumeMark_1_0, i);
		fprintf(fp, "%s\n", volume.m_studyUID.GetString());
		fprintf(fp, "%s\n", volume.m_seriesUID.GetString());
		fprintf(fp, "%s\n", volume.m_dataType.GetString());
		fprintf(fp, "%d\n", volume.m_sortOrder);
		fprintf(fp, "%d\n", volume.m_instanceCount);
		
		const std::vector<const char*>* sopList = iVal2[i];
		if(volume.m_instanceCount != sopList->size())
		{
			GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::WriteVolumeIDInfo instance list size error(%d, %d)\n",
				volume.m_instanceCount, sopList->size());
			fclose(fp);
			return false;
		}

		for(j=0; j<volume.m_instanceCount; j++)
		{
			fprintf(fp, "%s\n", sopList->operator[](j));
		}
	}
	
	fclose(fp);
	return true;
}
#endif

static inline char* trimRight(char* buf)
{
	int len = strlen(buf);
	char *p;

	for ( p = buf + len-1; p >= buf && isspace(*p); )
		*p-- = 0;

	return buf;
}

static char* GetNextLine(char* buf, FILE* fp, int bfsize)
{
	while(fgets(buf, bfsize, fp) != 0)
	{
		buf[bfsize] = 0; trimRight(buf);
		
		// return on non-empty line
		if(buf[0])
			return buf;
	}
	fclose(fp);
	return 0;

}

bool AqVolumeIDInfo::LoadVolumeIDInfo(AqVolumeIDInfoList& oVal, bool iInput, const char* iPath)
{
	FILE* fp;
	AqString filePath;
	const char* filename = (iInput)?InputVolumeInfoFile:OutputVolumeInfoFile;


	if(iPath && iPath[0])
	{
		filePath = iPath;
		filePath = filePath + "/" + filename;
	}
	else
	{
		filePath = filename;
	}
		
	if ((fp = fopen(filePath, "r+t")) == 0)
	{
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo fail to open file %s \n",
									filePath);
		return false;
	}
	
	char buf[257];
	if(GetNextLine(buf, fp, 256) == 0)
	{
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo empty file: %s \n",
									filePath);
		return false;
	}

	int vcount, icount;
	AqVolumeIDInfo* pVolumeInfo = 0;
	AqString strbuf;
	int sortOrder;
	if(strcmp(buf, StringVersion_1_0) == 0)
	{
		vcount = 0; icount = 0;
		while(GetNextLine(buf, fp, 256))
		{
			// new volume section start
			if(memcmp(buf, VolumeMark_1_0, SizeofVolumeMark_1_0) == 0)
			{
				//validate the finished one
				if(pVolumeInfo)
				{
					if(	pVolumeInfo->m_instanceCount != pVolumeInfo->m_instances.size())
					{
						fclose(fp);
						GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo "
							"reading instance number miss match at volume: %d\n", vcount);
						return false;
					}
				}

				// get new object to fill data
				//oVal.push_back(samplevInfo);
				//pVolumeInfo = &(oVal[vcount]);
				
				oVal.AddNew();
				pVolumeInfo = &(oVal[vcount]);

				vcount++; // next volume

				// fill header information
				// study UID
				if(GetNextLine(buf, fp, 256) == 0)
				{
					GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo "
						"missing study uid at volume: %d\n", vcount);
					return false;

				}
				pVolumeInfo->m_studyUID = buf;

				// series UID
				if(GetNextLine(buf, fp, 256) == 0)
				{
					GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo "
						"missing series uid at volume: %d\n", vcount);
					return false;

				}
				pVolumeInfo->m_seriesUID = buf;

				//data type
				if(GetNextLine(buf, fp, 256) == 0)
				{
					GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo "
						"missing data type at volume: %d\n", vcount);
					return false;

				}
				pVolumeInfo->m_dataType = buf;

								
				// sort order
				if(GetNextLine(buf, fp, 256) == 0)
				{
					GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo "
						"missing sort order at volume: %d\n", vcount);
					return false;

				}
				sortOrder = atoi(buf);
				if(sortOrder != kAqHeadToFoot && sortOrder != kAqFootToHead)
					sortOrder = kAqUnKnown;
				pVolumeInfo->m_sortOrder = (eVolumeOrder)sortOrder;
				
				// instance number
				if(GetNextLine(buf, fp, 256) == 0)
				{
					GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo "
						"missing instance number at volume: %d\n", vcount);
					return false;

				}
				pVolumeInfo->m_instanceCount = atoi(buf);

			}
			else // get instance UID
			{
				if(pVolumeInfo)
					pVolumeInfo->m_instances.push_back((const char*)buf);
			}

		}
	}
	else
	{
		fclose(fp);
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::LoadVolumeIDInfo unsupport version: %s\n",
									buf);
		return false;
	}

	return true;
}

bool AqVolumeIDInfo::MakeVolumesHash(const AqVolumeIDInfoList& iVolumes, AqString& oVal)
{
	TRCryptor cryptor;
	if(!cryptor.InitMD5())
		return false;
	
	for (int i=0; i<iVolumes.Size(); i++)
	{
		if(!cryptor.AddData(iVolumes[i], true))
			return false;
	}

	return cryptor.GetCountAndHashHex(oVal);
}

bool AqVolumeIDInfo::MakeVolumeID(const AqVolumeIDInfo& iVolume, AqString& oVal)
{
	
	TRCryptor cryptor;
	if(!cryptor.InitMD5())
		return false;
	
	if(!cryptor.AddData(iVolume, true))
		return false;

	return cryptor.GetCountAndHashHex(oVal);
}



bool AqVolumeIDInfo::GetVolumeID(AqString& oVal) const
{
	return MakeVolumeID(*this, oVal);
}


bool AqVolumeIDInfo::SetSortOrder(eVolumeOrder iOrder)
{
	if(m_sortOrder == iOrder)
		return true;
	
	if(m_sortOrder == kAqUnKnown)
	{
		GetAqLogger()->LogMessage("Error: AqVolumeIDInfo::SetSortOrder can not sort unknown sort-type data\n");
		return false;
	}

	m_sortOrder = iOrder;
	int nInstance = m_instances.size();
	if(nInstance < 2)
		return true;

	int i, j, mid = nInstance/2;
	for (i=0, j=nInstance-1; i<mid; i++, j--)
		m_instances[i].Swap(m_instances[j]);

	return true;

}

#if 0
bool AqVolumeIDInfo::AddInstance(std::vector<AqVolumeIDInfo> ioVolumes, const char* iStudyUID, const char* iSeriesUID, 
								 const char* iSOPInstanceUID, const char* iDataType, eVolumeOrder iSortOrder)
{
	int i, vIndex=-1;

	// search the exists volume
	for(i=0; i<ioVolumes.size(); i++)
	{
		if(ioVolumes[i].m_studyUID == iStudyUID && ioVolumes[i].m_seriesUID == iSeriesUID &&
			ioVolumes[i].m_dataType == iDataType && ioVolumes[i].m_sortOrder == iSortOrder )
		{
			vIndex = i;
			break;
		}
	}

	AqVolumeIDInfo* pVolumeInfo;
	if(vIndex < 0)
	{
		vIndex = ioVolumes.size();
		// get new object to fill data
		ioVolumes.push_back(AqVolumeIDInfo());
		pVolumeInfo = &(ioVolumes[vIndex]);
		pVolumeInfo->m_studyUID = iStudyUID;
		pVolumeInfo->m_seriesUID = iSeriesUID;
		pVolumeInfo->m_dataType = iDataType;
		pVolumeInfo->m_sortOrder = iSortOrder;
	}
	else
	{
		pVolumeInfo = &(ioVolumes[vIndex]);
	}
	pVolumeInfo->m_instanceCount++;
	pVolumeInfo->m_instances.push_back(iSOPInstanceUID);

	return true;
}
#endif