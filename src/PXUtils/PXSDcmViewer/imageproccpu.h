#ifndef IMAGEPRC_CPU_H
#define IMAGEPRC_CPU_H
 
 #include "imageProcBase.h"


class CImageFilterNullCPU : public  CImageFilterNull
{
public:
	CImageFilterNullCPU() ;
	~CImageFilterNullCPU() ;
	virtual bool doFilter() ;
protected:
	 
};

class CImageFirFilterCPU : public  CImageFirFilter
{
public:
	CImageFirFilterCPU();
	~CImageFirFilterCPU();

	virtual bool doFilter ();
protected:
	
};
//////
class CImageBiLateralFilterCPU : public  CImageBiLateralFilter
{
public:
	CImageBiLateralFilterCPU();
	~CImageBiLateralFilterCPU();

	 
	virtual bool doFilter ();
protected:
	
};

class CImageProcCPU : public CImageProcBase
{
	 
public:
    CImageProcCPU();
	~CImageProcCPU();
	virtual bool createFilter(const std::string &name, int type);
	virtual void selCurrentFilter(const std::string &name) ;
	
protected:
 
};
 



#endif //IMAGEPRC_CPU_H


