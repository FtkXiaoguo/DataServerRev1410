/***********************************************************************
 * AqVolumeIDInfo.h
 *---------------------------------------------------------------------
 * 
 */

#pragma warning (disable: 4786)

#ifndef AqVolumeIDInfo_H
#define AqVolumeIDInfo_H 

#include "AqCore.h"
#include "AqString.h"
#include <vector>


class AqVolumeIDInfoList;


// this class is to capture information regarding volume
class AqVolumeIDInfo : public AqStringList
{
public:

	enum eVolumeOrder
	{
		kAqUnKnown = 0,
		kAqHeadToFoot, 
		kAqFootToHead
	} ;

	static bool WriteVolumeIDInfo(const AqVolumeIDInfoList& iVal, bool iInput, const char* iPath);
	static bool LoadVolumeIDInfo(AqVolumeIDInfoList& oVal, bool iInput, const char* iPath);


	static bool WriteInputVolumeIDInfo(const AqVolumeIDInfoList& iVal, const char* iPath=0) {return WriteVolumeIDInfo(iVal, true, iPath);}
	static bool LoadInputVolumeIDInfo(AqVolumeIDInfoList& oVal, const char* iPath=0) {return LoadVolumeIDInfo(oVal, true, iPath);}

	static bool WriteOutputVolumeIDInfo(const AqVolumeIDInfoList& iVal, const char* iPath=0) {return WriteVolumeIDInfo(iVal, false, iPath);}
	static bool LoadOutputVolumeIDInfo(AqVolumeIDInfoList& oVal, const char* iPath=0) {return LoadVolumeIDInfo(oVal, false, iPath);}

	static bool MakeVolumeID(const AqVolumeIDInfo& iVolumes, AqString& oVal);
	static bool MakeVolumesHash(const AqVolumeIDInfoList& iVolumes, AqString& oVal);

	//static bool WriteVolumeIDInfo(const AqVolumeIDInfoList& iVal1, std::vector< const std::vector< const char* > * >& iVal2, 
	//								   bool iInput, const char* iPath);

	//static bool AddInstance(AqVolumeIDInfoList& ioVolumes, const char* iStudyUID, const char* iSeriesUID, 
	//		const char* iSOPInstanceUID, const char* iDataType, eVolumeOrder iSortOrder = kAqHeadToFoot);
	
	AqVolumeIDInfo(void) { Clear();};
	AqVolumeIDInfo(const AqVolumeIDInfo & iVolumeIDInfo) { *this = iVolumeIDInfo;}

	virtual ~AqVolumeIDInfo(void) {};


	virtual void Resize(unsigned int iSize) {m_instanceCount = iSize; m_instances.resize(iSize);}
	
	const AqVolumeIDInfo & operator = (const AqVolumeIDInfo & iVolumeIDInfo)
	{
		m_studyUID = iVolumeIDInfo.m_studyUID;
		m_seriesUID = iVolumeIDInfo.m_seriesUID;
		m_dataType = iVolumeIDInfo.m_dataType;
		m_sortOrder = iVolumeIDInfo.m_sortOrder;
		m_instanceCount = iVolumeIDInfo.m_instanceCount;
		m_instances = iVolumeIDInfo.m_instances;

		return *this;
	}

	bool operator == (const AqVolumeIDInfo & iVolumeIDInfo )
	{
		return 
			( m_studyUID == iVolumeIDInfo.m_studyUID &&
				m_seriesUID == iVolumeIDInfo.m_seriesUID &&
				m_dataType  == iVolumeIDInfo.m_dataType  &&
				m_sortOrder  == iVolumeIDInfo.m_sortOrder  &&
				m_instanceCount == iVolumeIDInfo.m_instanceCount
			);

	};

	bool operator != (const AqVolumeIDInfo & iVolumeIDInfo )
	{
		return !(*this == iVolumeIDInfo);
	};


	void Clear()
	{
		m_studyUID = "";
		m_seriesUID = "";
		m_dataType  = "";
		m_sortOrder = kAqUnKnown;
		m_instanceCount = 0;
		m_instances.clear();
	}

	bool SetSortOrder(eVolumeOrder iOrder);

	bool GetVolumeID(AqString& oVal) const;

	


	// information for volume
	AqString				m_studyUID;
	AqString				m_seriesUID;
	AqString				m_dataType;
	eVolumeOrder			m_sortOrder;
	int						m_instanceCount; // number of instance in volume

	// can be empty list, it means all instance in a series
	//std::vector<AqString>	m_instances;  in AqStringList
};


class AqVolumeIDInfoList
{
public:
	AqVolumeIDInfoList() {};
	virtual ~AqVolumeIDInfoList() {};

	virtual int Size() const = 0;
	virtual void Clear() = 0;
	
	virtual bool AddNew() = 0;
	virtual AqVolumeIDInfo& GetAt(unsigned int index) const = 0;
	virtual AqVolumeIDInfo& operator [](unsigned int index) const = 0;
};


class AqVolumeIDInfoVector : public AqVolumeIDInfoList
{
public:
	AqVolumeIDInfoVector() {};
	AqVolumeIDInfoVector(const AqVolumeIDInfoList& iVolumeIDInfoList) { *this = iVolumeIDInfoList;}
	AqVolumeIDInfoVector(const AqVolumeIDInfoVector& iVolumeIDInfoVector) { *this = iVolumeIDInfoVector;}

	virtual ~AqVolumeIDInfoVector() { Clear(); }

	virtual int Size() const {return m_volumeIDs.size();}
	virtual void Clear() 
	{
		for(int i=0; i<m_volumeIDs.size(); i++)
		{
			AqVolumeIDInfo* p = m_volumeIDs[i];
			if(p)
				delete p;
		}
		m_volumeIDs.clear();
	}
	
	virtual bool AddNew() { m_volumeIDs.push_back(new AqVolumeIDInfo()); return true;}
	virtual AqVolumeIDInfo& GetAt(unsigned int index) const {return *(m_volumeIDs[index]);}
	virtual AqVolumeIDInfo& operator [](unsigned int index) const {return *(m_volumeIDs[index]);}

	virtual void AddOne(const AqVolumeIDInfo& iVInfo) { m_volumeIDs.push_back(new AqVolumeIDInfo(iVInfo)); }

	const AqVolumeIDInfoVector& operator = (const AqVolumeIDInfoVector& iVolumeIDInfoVector) 
	{
		Clear();
		for(int i=0; i<iVolumeIDInfoVector.Size(); i++)
		{
			m_volumeIDs.push_back(new AqVolumeIDInfo(iVolumeIDInfoVector[i]));
		}
		return *this;
	}

	const AqVolumeIDInfoVector& operator = (const AqVolumeIDInfoList& iVolumeIDInfoList) 
	{
		Clear();
		for(int i=0; i<iVolumeIDInfoList.Size(); i++)
		{
			m_volumeIDs.push_back(new AqVolumeIDInfo(iVolumeIDInfoList[i]));
		}
		return *this;
	}

protected:
	std::vector<AqVolumeIDInfo*> m_volumeIDs;

};

#endif