/***********************************************************************
 * $Id: TRCompressedCacheWriter.cpp 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This file implements the member functions of the 
 *	    TRCompressedCacheWriter Object.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#include "TRCompressedCacheWriter.h"

#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

#include <fstream>
//#include <strstrea.h>
#include <strstream>
#include <iostream>

#ifndef AWARE_ONLY
#include "j2kErr.h"
#include "j2kEncoder.h"
#endif

//#include "AppComDataConversion.h"
#include "rtvsutil.h"


using namespace std;

TRCompressedCacheWriter::ProcessMap TRCompressedCacheWriter::c_pMap;

// create TRCompressedCacheWriter instance, then turn it to One shot Tread manager
//called in SeriesDirMonitor as TRCompressedCacheWriter::Start(m_cacheDir, true);

int TRCompressedCacheWriter::Start(const char* iCacheDirectory, bool threading, float iCompressionRatio)
{
	if(!threading)
	{
		TRCompressedCacheWriter compressedCacheWriter(iCacheDirectory, iCompressionRatio);
		return compressedCacheWriter.Process();
	}
	else
	{
		TRCompressedCacheWriter* p = new TRCompressedCacheWriter(iCacheDirectory, iCompressionRatio);
		
		if(Stop(iCacheDirectory) != kSuccess || !c_pMap.Add(iCacheDirectory, p))
		{
			delete p;
			return kCancelled;
		}
		p->m_inMap = true;
		RTVOneShotThreadManager::theManager().AddRequest(p);
		return kSuccess;
	}

}


//called in SeriesDirMonitor as TRCompressedCacheWriter::Stop(m_cacheDir);

int TRCompressedCacheWriter::Stop(const char* iCacheDirectory)
{
	TRCompressedCacheWriter* proc = c_pMap.Get(iCacheDirectory);
	if(!proc) return kSuccess;
	
	if(RTVOneShotThreadManager::theManager().RemoveRequest(proc, 5000))
	{
		c_pMap.Remove(iCacheDirectory);
		return kSuccess;
	}
	return kCancelled;
}

//-----------------------------------------------------------------------------

TRCompressedCacheWriter::TRCompressedCacheWriter(const char* iCacheDirectory, float iCompressionFactor):
	m_cacheDirectory(iCacheDirectory), m_compressionFactor(iCompressionFactor), m_status(kSuccess)
{
	m_processorName = "TRCompressedCacheWriter";
	m_inMap = false;
	// Make sure we have no trailing '/'
	int slen = m_cacheDirectory.length()-1;
	char tail = m_cacheDirectory[slen];
	if (tail == '/' || tail == '\\')
		m_cacheDirectory.resize(slen);


	m_srcDataFileName                = m_cacheDirectory + "/" + kCacheDataFileName;
	
	m_level0CacheDesctiptionFileName = m_cacheDirectory + "/"+ kJ2kCacheL0DescriptionFileName;
	m_level0CacheDataFileName        = m_cacheDirectory + "/"+ kJ2kCacheL0DataFileName;
 
	m_level1CacheDesctiptionFileName = m_cacheDirectory + "/"+ kJ2kCacheL1DescriptionFileName;
	m_level1CacheDataFileName        = m_cacheDirectory + "/"+ kJ2kCacheL1DataFileName;
 

}

//-----------------------------------------------------------------------------

TRCompressedCacheWriter::~TRCompressedCacheWriter()
{
	if(m_inMap)
		c_pMap.Remove(m_cacheDirectory);

}

//-----------------------------------------------------------------------------
int TRCompressedCacheWriter::DeleteLevel0CacheFiles()
{
		// Make sure that the .j2k are not present. If they are then delete and recreate them
	if (::access(m_level0CacheDesctiptionFileName.c_str(), 0) == 0)
	{
		if (::remove (m_level0CacheDesctiptionFileName.c_str()) != 0)
		{
			return (m_status = kErrCouldNotDeleteL0CompressedCache);
		}
	}	
	
	if (::access(m_level0CacheDataFileName.c_str(), 0) == 0)
	{
		if (::remove (m_level0CacheDataFileName.c_str()) != 0)
		{
			return (m_status = kErrCouldNotDeleteL0CompressedCache);
		}
	}

	return kSuccess;
}

//-----------------------------------------------------------------------------

int TRCompressedCacheWriter::Process()
{
	if(TerminationRequested()) // fullfill cancell requested before start thread
		return (m_status = kCancelled);

	m_status = kRunning;

	// -- 07/18/2003 ThreadProcess should never set these things
//	m_processStatus = kRunning;

	// First Check to see if the directory exists
	if (::access(m_cacheDirectory.c_str(), 2) != 0)
	{
		RequestTermination();
		return (m_status = kErrCannotAccessDirectory);
	}


	m_status = DeleteLevel0CacheFiles();

	if (m_status != kSuccess) return m_status;

	// Read the Cache.description
	m_status = m_cacheReader.ReadCacheDescription (m_cacheDirectory.c_str(), false);

	if (m_status == kSuccess)
	{
		// Write the L0 Cache
		m_status = WriteCompresssedCache ();

		if (m_status != kSuccess) return m_status;
	}

	return m_status;
}
//-----------------------------------------------------------------------------
class TRMemoryGaurd
{
	public:
		TRMemoryGaurd ()
		{
			m_memory = 0;
			m_numberOfBytesAllocated = 0;
		}

		~TRMemoryGaurd ()
		{
			if (m_memory) delete [] m_memory;
		
			m_memory = 0;
			m_numberOfBytesAllocated = 0;		
		}

		int Allocate (unsigned int iBytesToAllocate)
		{
			if (iBytesToAllocate > m_numberOfBytesAllocated)
			{
				if (m_memory) delete [] m_memory;

				m_numberOfBytesAllocated = iBytesToAllocate;
				m_memory = new unsigned char [m_numberOfBytesAllocated];
		
			}

			return kSuccess;
		}

	 unsigned char* GetData () {return m_memory;}
	 int GetNumberOfBytesAllocated () {return m_numberOfBytesAllocated;}

	protected:
		unsigned char* m_memory;
		unsigned int   m_numberOfBytesAllocated;

};



//-----------------------------------------------------------------------------

// -- JULY-22-2003 CompressedCache right now is used strictly for WAN.
// For WAN mode, we really want each image to look good. Thus scaling using
// global min,max is not the right thing for that. Isolate all global minmax
// handling using kUserGlobalMinMax

#define kUseGlobalMinMax   0



int TRCompressedCacheWriter::WriteCompresssedCache ()
{


#ifndef AWARE_ONLY
	j2kEncoder encoder;
	encoder.setBigEndian(false);
	encoder.setFastMode(true);
	encoder.setCompressionRatio(1.0/m_compressionFactor);
#endif

	int status = kSuccess;
#if 0

	std::vector <iRTVPtrToSliceInformation>& allSlices = m_cacheReader.GetAllSlices();
	int numberOfSlices = allSlices.size();

	ifstream srcDataFile;
	ofstream dstL0DataFile;
	ofstream dstL0DescriptionFile;
	

	srcDataFile.open          (m_srcDataFileName.c_str(),ios::binary |  ios::nocreate);
	dstL0DescriptionFile.open (m_level0CacheDesctiptionFileName.c_str(), ios::app | ios::binary);
	dstL0DataFile.open        (m_level0CacheDataFileName.c_str(), ios::app  | ios::binary);



	// Make sure all the relevant files are open
	if (!dstL0DescriptionFile.is_open ())
	{
		return -1;
	}
	if (!dstL0DataFile.is_open ())
	{
		return -1;
	}

	int i = 0;


	// Allocate memory to hold the buffer
	TRMemoryGaurd srcSlice;
	TRMemoryGaurd dstSlice;
	

	int bitsPerPixel   = allSlices[0].GetBitsAllocated ();
	int bytesPerPixel  = (bitsPerPixel + 7)/8;
	int sizeX		   = allSlices[0].GetSizeX ();
	int sizeY		   = allSlices[0].GetSizeY ();

	// This is for right now as suggested by Robert (M)
	if (bitsPerPixel > 12) bitsPerPixel = 12;

	unsigned int     sizeOfCompressedData = sizeX*sizeY*bytesPerPixel;
	srcSlice.Allocate (sizeOfCompressedData);
	dstSlice.Allocate (sizeOfCompressedData);

	// If this is MR we will need to shift the dataset by a scale and offset
	// In the compressed data case it is done on the entire series

	int globalMinVV = 0;
	int globalMaxVV = 0;

	double a = 1.0;
	double b = 1.0;
	double result = 0.0;
	double range  = 4095.0;

	bool isMR = false;
	std::string sopClassUID = allSlices[0].GetSOPClassUID();


	if (IsSOPClassUIDMR(sopClassUID))
	{
		isMR = true;
#if !kUseGlobalMinMaX		// -- JULY-22-2003		
		// Go through all the slice and find out what the global min and max are
		int min = allSlices[0].GetSliceMinVoxelValue ();
		int max = allSlices[0].GetSliceMaxVoxelValue ();
		
		globalMinVV = min;
		globalMaxVV = max;
		
		for (i = 1; i < numberOfSlices; i++)
		{
			
			min = allSlices[0].GetSliceMinVoxelValue ();
			max = allSlices[0].GetSliceMaxVoxelValue ();
			
			if (min < globalMinVV) globalMinVV = min;
			if (max > globalMaxVV) globalMaxVV = max;
		}
		
		// a(min) + b = 0
		// a(max) + b = 1<<m_bitsUsedPerPixel) -1;
		a = 1.0;
		b = 1.0;
		result = 0.0;
		range  = globalMaxVV - globalMinVV +1;
		
		if (range > 4095.0) range = 4095.0;
		
		a = range / (double)(globalMaxVV - globalMinVV);
		b = -1.0 * a * ((double)globalMinVV);
#endif // if !kUseGlobalMinMaX
	}

	
	unsigned long    currentFileOffset    = 0;
	// Main loop
	for (i = 0; i < numberOfSlices; i++)
	{
		if(TerminationRequested())// cancel writing
		{
			status = kCancelled;
			break;
		}
		


#ifdef _DEBUG
		cerr << "Writing compressedCache. Slice " << i+1 << "instance " << allSlices[i].GetImageNumber() << endl;
#endif

		// Read the data from the SrcFile and compress using j2K and write out

		__int64 bytesToSkip = allSlices[i].GetStartOfData();
		int     bytesToRead = allSlices[i].GetSizeOfData();

		srcSlice.Allocate (bytesToRead);
		sizeOfCompressedData = bytesToRead;
		dstSlice.Allocate (sizeOfCompressedData);

		srcDataFile.seekg (bytesToSkip, ios::beg);
		srcDataFile.read (((char*)srcSlice.GetData()), bytesToRead);

		// -- JULY-22-2003 we need to convert in consistent way
		AppComDataConverter dc;
		float slope = 1.0f, offset = 0.0;
		int iWindowWidth = 0, iWindowCenter = 0 ;
		int ww = allSlices[i].GetVOIWindowWidth();
		int wc = allSlices[i].GetVOIWindowCenter();
	
		if (srcDataFile.good ()) 
		{
			unsigned char *src = srcSlice.GetData();

			if (isMR)
			{

				int totalSize = sizeX * sizeY;
				short* pData = (short*)srcSlice.GetData();
#if kUseGlobalMinMaX
				for (int j = 0; j < totalSize; j++)
				{
					result = a*(double)pData[j] + b;
					pData[j] = (short)result;
				}
#else
				// -- JULY-22-2003 use common converter for consistency
				dc.FindMinMax(pData, totalSize, true,globalMinVV, globalMaxVV);
			 	dc.SetVOIWindowWidth (ww);
			 	dc.SetVOIWindowCenter (wc);
		 		dc.RescaleMR (pData, pData, totalSize, globalMinVV, globalMaxVV);
#endif
			}

			/* scale the voi lut */
			dc.GetPixelToVoxelMapping(slope, offset); 
			iWindowWidth = int(ww * slope + 0.5f);
			iWindowCenter = int((wc * slope + offset)*1.0001f);

			iWindowWidth = iRTVSClamp(iWindowWidth,0,4096);
			iWindowCenter = iRTVSClamp(iWindowCenter,-200,5000);

			sizeOfCompressedData = dstSlice.GetNumberOfBytesAllocated ();

#ifndef AWARE_ONLY
			if(encoder.compress(&src, sizeX, sizeY, 1, 
				                bitsPerPixel, dstSlice.GetData(), 
								sizeOfCompressedData) != J2K_OK)
#endif
			{
				status = kErrJ2kCompressionFailed;
				break;
			}
		}

	

		// Write the Cache Description and the Cache Data
		dstL0DescriptionFile << "<" << kSliceDescriptionStr << " V0.9>" << "\n"; // open slice description
		
		dstL0DescriptionFile << kSOPInstanceUIDStr << " = " << allSlices[i].GetSOPInstanceUID() << "\n";
		dstL0DescriptionFile << kStartOfDataInDataFileStr << " = " << currentFileOffset  << "\n";
		dstL0DescriptionFile << kSizeOfDataInBytesStr << " = " << sizeOfCompressedData  << "\n";
		
		dstL0DescriptionFile << kSliceCompressionFactorStr << " = " << m_compressionFactor  << "\n";
		dstL0DescriptionFile << kInputCompressionSizeXStr << " = " << sizeX  << "\n";
		dstL0DescriptionFile << kInputCompressionSizeYStr << " = " << sizeY  << "\n";

		// -- JULY-22-2003 we need these
		if (iWindowWidth != 0 || iWindowCenter != 0)
		{
			dstL0DescriptionFile << kVOIWindowWidthStr  << " = " << iWindowWidth  << "\n";
			dstL0DescriptionFile << kVOIWindowCenterStr << " = " << iWindowCenter  << "\n";
		}
		
		if (isMR)
		{
			dstL0DescriptionFile << kGlobalMinVoxelValueStr << " = " << globalMinVV  << "\n";
			dstL0DescriptionFile << kGlobalMaxVoxelValueStr << " = " << globalMaxVV  << "\n";
		}

		dstL0DescriptionFile << "</" << kSliceDescriptionStr << ">" << "\n";     // close slice description


		if (!dstL0DescriptionFile.good ())
		{
			status = kErrFileStateIsNotGood;
			break;
		}

		 dstL0DataFile.write (dstSlice.GetData(), sizeOfCompressedData);

		if (!dstL0DataFile.good ())
		{
			status = kErrFileStateIsNotGood;
			break;
		}

		currentFileOffset += sizeOfCompressedData;

	}

	dstL0DataFile.flush();
	dstL0DescriptionFile.flush();

	// Close all the files 
	if (srcDataFile.is_open())          srcDataFile.close();
	if (dstL0DescriptionFile.is_open()) dstL0DescriptionFile.close();
	if (dstL0DataFile.is_open())        dstL0DataFile.close();
	
	m_status = status;
	if (status != kSuccess) 
	{
		DeleteLevel0CacheFiles ();
	}

#endif
	return status;
}

//-----------------------------------------------------------------------------