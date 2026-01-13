/***********************************************************************
 * rtvloadoption.h
 *---------------------------------------------------------------------
 *		Copyright, Mitsubishi Electric Information Technology Center 
 *					America, Inc., 2000, All rights reserved.
 *
 *	PURPOSE:
 *		A generic name-value pair reader and parser. Useful for
 *      configuration parsing and generations.
 *
 *	AUTHOR(S):  T.C. Zhao, December 2000.
 *
 *   
 *-------------------------------------------------------------------
 */


#ifndef RTVLOADOPTION_H_
#define RTVLOADOPTION_H_

#include <stdio.h>
#include "AqCore/AqCore.h"
#include "AqCore/TRLogger.h"

class iRTVOptionsInternal;

class iRTVOptions
{
public:
	enum 
	{ 
		kOK = 0,
		kBadArgument = -100, 
		kErrAlloc, 
		kErrFormat, 
		kNoFile,
		kErrOpen, 
		kErrRead, 
		kErrWrite,
		kSystemFile

	};
	
	//keys to control the shared file access 
	enum fileSharing
	{
		kLoadOptionSharedReadOnly,
		kLoadOptionSharedWriteOnly,
		kLoadOptionNoSharedReadWrite,
	};

public:
	iRTVOptions(int iCaseSensitive=0);
	~iRTVOptions(void);

	// basic object requirement. Note the copy constructor and operator= do
	// shallow copies that only take care of memory but the internal name-value
	// pairs are NOT copied. This behavior in most cases is the correct
	// behavior as the address of iValue usually should not be copied.
	iRTVOptions(const iRTVOptions& iOpt);
	iRTVOptions& operator=(const iRTVOptions& iOpt);

	// using Add() to specify the name-value pair. Use iMask to specify
	// a mask so you can use GetBits() to find out which name-value pair is set
	//	template <class T> void	Add(const char* iName, T iValue);
	void	Add(const char* iName, int*			 iValue, unsigned int iMask=0);
	void	Add(const char* iName, unsigned int* iValue, unsigned int iMask=0);
	void	Add(const char* iName, long*		 iValue, unsigned int iMask=0);
	void	Add(const char* iName, unsigned long* iValue, unsigned int iMask=0);
	void	Add(const char* iName, short*		 iValue, unsigned int iMask=0);
	void	Add(const char* iName, unsigned short*iValue,unsigned int iMask=0);
	void	Add(const char* iName, float*		 iValue, unsigned int iMask=0);
	void	Add(const char* iName, double*		 iValue, unsigned int iMask=0);
	void	Add(const char* iName, char*		 iValue, int iLen=0,unsigned int iMask=0);
	void	Add(const char* iName, unsigned char*iValue, int iLen=0,unsigned int iMask=0);
	void	Add(const char*	iName, bool*		 iValue, unsigned int iMask=0);	

	// GetBits() returns the bits that are set by Load().
	unsigned int	GetBits(void) const { return m_bits;}

	// remove an option.
	int		Remove(const char* iName);

	// Get number of options
	int		GetCount(void) const; 

	const char* GetOptionName(int iC) const;
	const char*	GetOptionValue(const char *iOption);

	// Clear all settings
	void	Clear(void);

	int		SetLogLevel(int iLevel);

	// save and load from a file
	int		Load  (const char*	iFileName, int iLock=0, bool iGenKeys = false);
	int		ReadHeader (FILE *fp, const char* startMark, const char* stopMark);
	int		Save  (const char*	iFileName, const char* iOptionName=0, int iLock=0);
	int		Save  (FILE* fp, const char* prefix="");
	int		Save  (TRLogger* ipLogger, const char* prefix="");

	const char*	GetLoadedData(int* oLen=0) 
	{ 
		if (oLen)
			*oLen = m_length;
		return m_buf;
	}

	void	CleanLoadedData(void);

	// save all the options into a string of form
	// A = v\n B = v\n etc
	const char*	BuildString(const char* iPrefix="");

	int		WriteHeader (FILE *fp, const char* startMark, const char* stopMark);


	//multi-node case - chetan
	//int		LoadWithLock  (const char*	iFileName, bool iGenKeys = false, int iSharingOption = kLoadOptionSharedReadOnly, const char* iConstraint = "");

	// Added jwu 03/20/04 for AQNet_GATE
	//chetan - added option (iSharingOption) to control file access
	//default is set to shared read only
	int		LoadWithLock  (const char*	iFileName, bool iGenKeys = false, int iSharingOption = kLoadOptionSharedReadOnly);
	int		SaveWithLock  (const char*	iFileName, bool iCreateIfNotThere = false, const char* iOptionName=0);
	// End jwu 03/20/04

	// save to and load from external sources: return true if parsed correctly.
	int		Parse(const char* iBytes, int iGenKeys=0);
	int		ParseLine(const char *iLine);
	char*	ConvertS(int iOption);
	char*	ConvertS(const char* iOption);

	// misc. options
	int		SetCommentChar(int iC);
	int		SetSeperator(int iS);
	int		GetCommentChar(void)   const { return m_commentChar;}
	int		GetSeperatorChar(void) const { return m_seperatorChar;}
	int		GetVerbose(void)       const { return m_verbose;}
	int		IsCaseSensitive(void)  const { return m_isCaseSensitive;}

private:
	int						m_verbose;
	int						m_commentChar;
	int						m_seperatorChar;
	int						m_isCaseSensitive;
	iRTVOptionsInternal*	m_implementation;
	unsigned int			m_bits;
	char*					m_buf;
	int						m_length;
};

// instentiate the template function explicitly
//template void  iRTVOptions::AddOption(const char*, short*);
//template void  iRTVOptions::AddOption(const char*, int*);



#endif

