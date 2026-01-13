//#include "StdAfx.h"
#include "StlReadWrite.h"
 
#include "math.h"

CStlReadWrite::CStlReadWrite(void)
{
}

CStlReadWrite::~CStlReadWrite(void)
{
}


CStlReadWrite::FileType CStlReadWrite::DetectFileType(const char *filename,unsigned long length, double percent_bin)
{
	if (!filename || percent_bin < 0)
    {
		return FileTypeUnknown;
    }

	 FILE *fp;
  fp = fopen(filename, "rb");
  if (!fp)
    {
    return CStlReadWrite::FileTypeUnknown;
    }

  // Allocate buffer and read bytes

  unsigned char *buffer = new unsigned char [length];
  size_t read_length = fread(buffer, 1, length, fp);
  fclose(fp);
  if (read_length == 0)
    {
    return CStlReadWrite::FileTypeUnknown;
    }

  // Loop over contents and count

  size_t text_count = 0;
 
  const unsigned char *ptr = buffer;
  const unsigned char *buffer_end = buffer + read_length;

  while (ptr != buffer_end)
    {
    if ((*ptr >= 0x20 && *ptr <= 0x7F) || 
        *ptr == '\n' ||
        *ptr == '\r' ||
        *ptr == '\t')
      {
      text_count++;
      }
    ptr++;
    }

  delete [] buffer;

  double current_percent_bin =  
    (static_cast<double>(read_length - text_count) /
     static_cast<double>(read_length));

  if (current_percent_bin >= percent_bin)
    {
    return CStlReadWrite::FileTypeBinary;
    }

  return CStlReadWrite::FileTypeText;

}
bool CStlReadWrite::readStl(const char *fileName)
{

	if(checkPxFormat(fileName)){
		return readStlPXBinary(fileName);
	};

	CStlReadWrite::FileType fileType = DetectFileType(fileName);
	if(fileType == CStlReadWrite::FileTypeUnknown){
		return false;
	}

	m_polygonPts.empty();

	if(fileType == FileTypeBinary){
		return readStlBinary(fileName);
	}else{
		return readStlText(fileName);
	}
}
//from : vtkSTLReader::ReadBinarySTL
bool CStlReadWrite::readStlBinary(const char *fileName)
{
	int i, numTris;
  unsigned long   ulint;
  unsigned short  ibuff2;
  char    header[81];
  typedef struct  {float  n[3], v1[3], v2[3], v3[3];} facet_t;
  facet_t facet;

  m_polygonPts.clear();
  CStlPolygon new_polygon;
	try{
		FILE *fp = fopen(fileName,"rb");
		if(fp == 0) return false;

		fread (header, 1, 80, fp);
		fread (&ulint, 1, 4, fp);
		//vtkByteSwap::Swap4LE(&ulint);

	if ( (numTris = (int) ulint) > 0 )
    {
	//	for ( i=0; fread(&facet,48,1,fp) > 0; i++ )
		for ( i=0; fread(&new_polygon,48,1,fp) > 0; i++ )
		{
			fread(&ibuff2,2,1,fp); //read extra junk

	//		vtkByteSwap::Swap4LE (facet.n);
	//		vtkByteSwap::Swap4LE (facet.n+1);
	//		vtkByteSwap::Swap4LE (facet.n+2);

	//		vtkByteSwap::Swap4LE (facet.v1);
	//		vtkByteSwap::Swap4LE (facet.v1+1);
	//		vtkByteSwap::Swap4LE (facet.v1+2);
	 

	//		vtkByteSwap::Swap4LE (facet.v2);
	//		vtkByteSwap::Swap4LE (facet.v2+1);
	//		vtkByteSwap::Swap4LE (facet.v2+2);
	 

	//		vtkByteSwap::Swap4LE (facet.v3);
	//		vtkByteSwap::Swap4LE (facet.v3+1);
	//		vtkByteSwap::Swap4LE (facet.v3+2);
	 

			m_polygonPts.push_back(new_polygon);

			if ( (i % 5000) == 0 && i != 0 )
			 {
			  ;
			}
		}

      
    }

	fclose(fp);
		 
	 }catch(...){
		return false;
	}

	return true;
}

//from : vtkSTLReader::ReadASCIISTL
bool CStlReadWrite::readStlText(const char *fileName)
{
#define READ_LINE_LEN (1024)
	char line[READ_LINE_LEN];
	float x[3];
	int done;

	int currentSolid = 0;
	
	m_polygonPts.clear();

	CStlPolygon new_polygon;
	try{
		FILE *fp = fopen(fileName,"rt");
		if(fp == 0) return false;
		 fgets (line, READ_LINE_LEN, fp);

		done = (fscanf(fp,"%s %*s %f %f %f\n", line, x, x+1, x+2)==EOF);
		if ((strcmp(line, "COLOR") == 0) || (strcmp(line, "color") == 0))
		{
			done = (fscanf(fp,"%s %*s %f %f %f\n", line, x, x+1, x+2)==EOF);
		}
		if ((strcmp(line, "FACET") == 0) || (strcmp(line, "facet") == 0))
		{
			 new_polygon.m_normal[0] = x[0];
			 new_polygon.m_normal[1] = x[1];
			 new_polygon.m_normal[2] = x[2];
			 
		}

		 //  Go into loop, reading  facet normal and vertices
  //
//  while (fscanf(fp,"%*s %*s %f %f %f\n", x, x+1, x+2)!=EOF)
	  while (!done)
		{
		//if (ctr>=253840) {
		//    fprintf(stdout, "Reading record %d\n", ctr);
		//}
		//ctr += 7;
				
			fgets (line, READ_LINE_LEN, fp);

			//normal
	//		fscanf (fp, "%*s %f %f %f\n", new_polygon.m_normal,new_polygon.m_normal+1,new_polygon.m_normal+2);
		 
		 
			//point1
			fscanf (fp, "%*s %f %f %f\n", new_polygon.m_point1,new_polygon.m_point1+1,new_polygon.m_point1+2);
			//point2
			fscanf (fp, "%*s %f %f %f\n", new_polygon.m_point2,new_polygon.m_point2+1,new_polygon.m_point2+2);
			//point3
			fscanf (fp, "%*s %f %f %f\n", new_polygon.m_point3,new_polygon.m_point3+1,new_polygon.m_point3+2);
		 
		 
			m_polygonPts.push_back(new_polygon);

			fgets (line, READ_LINE_LEN, fp); // end loop
			fgets (line, READ_LINE_LEN, fp); // end facet

		  
	 
			done = (fscanf(fp,"%s", line)==EOF);
			if ((strcmp(line, "ENDSOLID") == 0) || (strcmp(line, "endsolid") == 0)) 
			  {
			  currentSolid++;
			  fgets(line, READ_LINE_LEN, fp);
			  done = feof(fp);
			  while ((strstr(line, "SOLID") == 0) && (strstr(line, "solid") == 0) && !done) 
				{
				fgets(line, READ_LINE_LEN, fp);
				done = feof(fp);
				}

			  done = (fscanf(fp,"%s", line)==EOF);
			  if ((strstr(line, "COLOR") == 0) || (strstr(line, "color") == 0))
				{
				  done = (fscanf(fp,"%f %f %f\n", x,x+1,x+2)==EOF);
				  done = (fscanf(fp,"%s", line)==EOF);
				}
			  }
				
			
	  

			if (!done) {
	 			done = (fscanf(fp,"%*s %f %f %f\n", x, x+1, x+2)==EOF);
				//for read normal
				//added by N.Furutsuki
				if ((strcmp(line, "FACET") == 0) || (strcmp(line, "facet") == 0))
				{
					 new_polygon.m_normal[0] = x[0];
					 new_polygon.m_normal[1] = x[1];
					 new_polygon.m_normal[2] = x[2];
					 
				}
			}
		}
	    fclose(fp);

	}catch(...){
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
inline void ComputeNormalDirection(float v1[3], float v2[3],float v3[3], float n[3])
{
  double ax, ay, az, bx, by, bz;

  // order is important!!! maintain consistency with triangle vertex order
  ax = v3[0] - v2[0]; ay = v3[1] - v2[1]; az = v3[2] - v2[2];
  bx = v1[0] - v2[0]; by = v1[1] - v2[1]; bz = v1[2] - v2[2];

  double length;
  double n_temp[3];
  n_temp[0] = (ay * bz - az * by);
  n_temp[1] = (az * bx - ax * bz);
  n_temp[2] = (ax * by - ay * bx);
  //
  if ( (length = sqrt((n_temp[0]*n_temp[0] + n_temp[1]*n_temp[1] + n_temp[2]*n_temp[2]))) != 0.0 )
   {
    n[0] = n_temp[0]/length;
    n[1] = n_temp[0]/length;
    n[2] = n_temp[0]/length;
  }else{
	n[0] = n_temp[0];
    n[1] = n_temp[0];
    n[2] = n_temp[0];
  }
}

static char header[]="Prexion generated STL File                                        ";

bool CStlReadWrite::writeStlText(const char *fileName)
{
	bool ret_b = true;

	FILE *fp ;
	if ((fp = fopen(fileName, "wt")) == NULL)
    {
     
    return false;
    }
	 
//
//  Write header
//
  
  if (fprintf (fp, "solid ascii\n") < 0)
    {
    fclose(fp);
    
    return false;
    }
  int polygon_size = m_polygonPts.size();
//
//  Write out triangle polygons.  In not a triangle polygon, only first 
//  three vertices are written.
//
  for(int i=0;i<polygon_size;i++ )
   {
    

    ComputeNormalDirection(m_polygonPts[i].m_point1, m_polygonPts[i].m_point2, m_polygonPts[i].m_point3, m_polygonPts[i].m_normal);

    if (fprintf (fp, " facet normal %.6g %.6g %.6g\n  outer loop\n",
                 m_polygonPts[i].m_normal[0], m_polygonPts[i].m_normal[1], m_polygonPts[i].m_normal[2]) < 0)
      {
      fclose(fp);
       
      return false;
      }

    if (fprintf (fp, "   vertex %.6g %.6g %.6g\n", m_polygonPts[i].m_point1[0], m_polygonPts[i].m_point1[1], m_polygonPts[i].m_point1[2]) < 0)
      {
      fclose(fp);
       
      return false;
      }
    if (fprintf (fp, "   vertex %.6g %.6g %.6g\n", m_polygonPts[i].m_point2[0], m_polygonPts[i].m_point2[1], m_polygonPts[i].m_point2[2]) < 0)
      {
      fclose(fp);
       
      return false;
      }
    if (fprintf (fp, "   vertex %.6g %.6g %.6g\n", m_polygonPts[i].m_point3[0], m_polygonPts[i].m_point3[1], m_polygonPts[i].m_point3[2]) < 0)
      {
      fclose(fp);
       
      return false;
      }

    if (fprintf (fp, "  endloop\n endfacet\n") < 0)
      {
      fclose(fp);
      
      return false;
      }
   }
  if (fprintf (fp, "endsolid\n") < 0)
  {
    ret_b = false;
  }
  fclose (fp);

  return ret_b;
}

bool CStlReadWrite::writeStlBinary(const char *fileName)
{
  FILE *fp;
 
  unsigned long ulint;
  unsigned short ibuff2=0;

  if ((fp = fopen(fileName, "wb")) == NULL)
    {
     
    return false;
    }
  
  //  Write header
  //
  
  if (fwrite (header, 1, 80, fp) < 80)
    {
    fclose(fp);
     
    return false;
    }

  ulint =  m_polygonPts.size();
 // vtkByteSwap::Swap4LE(&ulint);
  if (fwrite (&ulint, 1, 4, fp) < 4)
    {
    fclose(fp);
     
    return false;
    }

  //  Write out triangle polygons.  In not a triangle polygon, only first 
  //  three vertices are written.
  //
  for (int i=0;i<ulint;i++)
   {
	 ComputeNormalDirection(m_polygonPts[i].m_point1, m_polygonPts[i].m_point2, m_polygonPts[i].m_point3, m_polygonPts[i].m_normal);

    
    if (fwrite (m_polygonPts[i].m_normal, 4, 3, fp) < 3)
      {
      fclose(fp);
       
      return false;
      }

    
    if (fwrite (m_polygonPts[i].m_point1, 4, 3, fp) < 3)
      {
      fclose(fp);
      return false;
      }

    if (fwrite (m_polygonPts[i].m_point2, 4, 3, fp) < 3)
      {
      fclose(fp);
      return false;
      }

    if (fwrite (m_polygonPts[i].m_point3, 4, 3, fp) < 3)
      {
      fclose(fp);
      return false;
      }

    if (fwrite (&ibuff2, 2, 1, fp) < 1)
      {
		fclose(fp);
		return false;
       
      }
    }
  fclose (fp);

  return true;
}

inline void findRange(float &x_min,float &x_max,float &y_min,float &y_max,float &z_min,float &z_max,float data[3])
{
	//x
	if(data[0]<x_min) x_min = data[0];
	else
	if(data[0]>x_max) x_max = data[0];
	//y
	if(data[1]<y_min) y_min = data[1];
	else
	if(data[1]>y_max) y_max = data[1];
	//z
	if(data[2]<z_min) z_min = data[2];
	else
	if(data[2]>z_max) z_max = data[2];
}
void CStlReadWrite::profile(int &polygon_size,DataRange3D *polygonRange)
{
	float x_min = (float)0x7fffffff;
	float x_max = -x_min;
	float y_min = x_min;
	float y_max = -y_min;
	float z_min = x_min;
	float z_max = -z_min;

	polygon_size = m_polygonPts.size();
	for(int i=0;i<polygon_size;i++){
		 
		findRange(x_min,x_max,y_min,y_max,z_min,z_max,m_polygonPts[i].m_point1);
		findRange(x_min,x_max,y_min,y_max,z_min,z_max,m_polygonPts[i].m_point2);
		findRange(x_min,x_max,y_min,y_max,z_min,z_max,m_polygonPts[i].m_point3);
 
	}
	printf(" polygon size %d \n",polygon_size);
	printf(" x[%f,%f], y[%f,%f], z[%f,%f] \n", x_min,x_max,y_min,y_max,z_min,z_max);

	if(polygonRange){
		polygonRange->m_rangeX[0] = x_min;
		polygonRange->m_rangeX[1] = x_max;
		//
		polygonRange->m_rangeY[0] = y_min;
		polygonRange->m_rangeY[1] = y_max;
		//
		polygonRange->m_rangeZ[0] = z_min;
		polygonRange->m_rangeZ[1] = z_max;
	}
}

////////////////
//
//#1590 use PreXion format

class PXSTL_Binary_Header
{
#define PXSTL_Binary_CurrentVer (1)
#define PXSTL_MAGICID1 (0xf1f1f1f1)
#define PXSTL_MAGICID2 (0x1f1f1f1f)
#define PXSTL_HEADER_SIZE (1024*4)
public:
	unsigned int magicID1;
	unsigned int magicID2;
	unsigned int version;
	unsigned int headerSize;
	unsigned int polygonSize;
	unsigned int reserved[16-5];
};
bool CStlReadWrite::writeStlPXBinary(const char *fileName)
{
  FILE *fp;
 
  unsigned long ulint;
  unsigned short ibuff2=0;

  if ((fp = fopen(fileName, "wb")) == NULL)
    {
     
    return false;
    }
  
  //  Write header
  //
  PXSTL_Binary_Header PXStl_header;
  PXStl_header.magicID1 = PXSTL_MAGICID1 ;
  PXStl_header.magicID2 = PXSTL_MAGICID2 ;
  PXStl_header.version	= PXSTL_Binary_CurrentVer;
  PXStl_header.headerSize = PXSTL_HEADER_SIZE ;
  if(PXStl_header.headerSize < sizeof(PXSTL_Binary_Header)){
	  return false;
  };
  
  //
 
   ulint =  m_polygonPts.size();

   PXStl_header.polygonSize = ulint;
  if (fwrite (&PXStl_header, 1, sizeof(PXSTL_Binary_Header), fp) < sizeof(PXSTL_Binary_Header))
    {
    fclose(fp);
     
    return false;
    }
  if( fseek(fp,PXStl_header.headerSize ,SEEK_SET) != 0)
  {
	  fclose(fp);
     
    return false;
  }
 
 
	CStlPolygon px_data_stl;
  //  Write out triangle polygons.  In not a triangle polygon, only first 
  //  three vertices are written.
  //
  int write_nn = 0;
  for (int i=0;i<ulint;i++)
   {
 
	 ComputeNormalDirection(m_polygonPts[i].m_point1, m_polygonPts[i].m_point2, m_polygonPts[i].m_point3, m_polygonPts[i].m_normal);
     Indata2PxStlData(m_polygonPts[i],px_data_stl);

	 if ((write_nn =fwrite (&px_data_stl, sizeof(CStlPolygon), 1, fp)) < 1)
      {
      fclose(fp);
      return false;
      }

    
 
    }
  fclose (fp);

  return true;
}

//#1590 use PreXion format
bool CStlReadWrite::readStlPXBinary(const char *fileName)
{
	int i, numTris;
  unsigned long   ulint;
 
  m_polygonPts.clear();
  CStlPolygon new_polygon;
  CStlPolygon  in_data_polygon;

  bool ret_b = true;
	try{
		FILE *fp = fopen(fileName,"rb");
		if(fp == 0) return false;

		PXSTL_Binary_Header PXStl_header;
		memset(&PXStl_header,0,sizeof(PXSTL_Binary_Header));

		fread (&PXStl_header, 1, sizeof(PXSTL_Binary_Header), fp);
		if(	(PXStl_header.magicID1 != PXSTL_MAGICID1) || 
			(PXStl_header.magicID2 != PXSTL_MAGICID2) ){
				return false;
		}
		if(PXStl_header.polygonSize <1) return false;
  //
 
		ulint =  PXStl_header.polygonSize;

    
		if( fseek(fp,PXStl_header.headerSize ,SEEK_SET) != 0)
		{
			fclose(fp);
		 
			return false;
		}

		int read_nn = 0;
			 
		for(i =0 ;i<ulint;i++){
			
			if((read_nn=fread(&new_polygon,sizeof(CStlPolygon),1,fp))<1){
				ret_b = false;
				break;
			} 
			Indata2PxStlData(new_polygon,in_data_polygon);

			m_polygonPts.push_back(in_data_polygon);

			if ( (i % 5000) == 0 && i != 0 )
			 {
			  ;
			}
      
		}

	fclose(fp);
		 
	}catch(...){
		return false;
	}

	return ret_b;
}
////#1590 use PreXion format
bool CStlReadWrite::checkPxFormat(const char *fileName)
{
	try{
		FILE *fp = fopen(fileName,"rb");
		if(fp == 0) return false;

		PXSTL_Binary_Header PXStl_header;
		memset(&PXStl_header,0,sizeof(PXSTL_Binary_Header));
 
		fread (&PXStl_header, 1, sizeof(PXSTL_Binary_Header), fp);
		if(	(PXStl_header.magicID1 == PXSTL_MAGICID1) && 
			(PXStl_header.magicID2 == PXSTL_MAGICID2) ){
				return true;
		}
		fclose(fp);
	}catch(...){
		return false;
	}

	return false;
}