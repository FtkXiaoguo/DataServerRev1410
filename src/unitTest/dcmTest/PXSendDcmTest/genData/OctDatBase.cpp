
#include "StdAfx.h"
#pragma warning (disable: 4244)
#include <math.h>
#include <stdlib.h>

#include "OctDatBase.h"

#include "cubedata.h"
 
#define SMALL_GRID

#include "MultiObjSlice.hpp"

COctDatBase::COctDatBase(void)
{
	m_dataBuff = new TrCtCbctCubeData ;
	m_offset = 0;
}

COctDatBase::~COctDatBase(void)
{
	delete m_dataBuff;
}
void * COctDatBase::getDataPtr(int frame) const
{
	return m_dataBuff->getSliceZ(frame);
}
template<class dataT> void genTestData(int patID,TrCtCbctCubeData *dataBuff,dataT val1,dataT val2,dataT val3)
{
	int sizeX,sizeY, sizeZ;
	dataBuff->getDim(sizeX,sizeY,sizeZ);
	int pos1_1 = sizeZ*2/8;
	int pos1_2 = sizeZ*3/8;
	int pos2_1 = sizeZ*6/8;
	int pos2_2 = sizeZ*7/8;

	int x_pos1_1 = sizeX*2/8;
	int x_pos1_2 = sizeX*3/8;

	int centerX = sizeX/2;
	int centerY = sizeY/2;
	int centerZ = sizeZ/2;
	int centerZ1 = centerZ-centerZ/3;
	int centerZ2 = centerZ+centerZ/3;
	float rad = (float)(sqrt((double)(sizeX*sizeX + sizeY*sizeY + sizeZ*sizeZ)));
	float dd_level = centerZ/4*0.9f ;
	float dd_level2 = dd_level*dd_level;
//
	int delta_dd = sizeX/4 ;

	int grid_dd = val1;
#ifdef SMALL_GRID
	if( (sizeX<33) || (sizeY<33) ||(sizeZ<33) ){
	 grid_dd = 2;
	}
#endif
	float grid_val = val3;
	for(int z_i=0;z_i<sizeZ;z_i++){
		dataT *slice_ptr = (dataT *)dataBuff->getSliceZ(z_i);
		for(int y_i=0;y_i<sizeY;y_i++){
			for(int x_i=0;x_i<sizeX;x_i++){
				   
					switch(patID){
					default:
					case 0: // 2 plane
						{
							float rnd_f1 = 0.0f;
							int rand_num = rand();
							if(rand_num%4==0){
								rnd_f1 = rand_num/(float)RAND_MAX;
							}
							float rnd_f2 = rand_num/(float)RAND_MAX;

						   if((x_i>x_pos1_1) && (x_i<x_pos1_2)){
							   slice_ptr[y_i*sizeX +x_i] = val3*(0.1+rnd_f2);
						   }else{
							   if((z_i>pos1_1) && (z_i<pos1_2)){
									slice_ptr[y_i*sizeX +x_i] = val2;
							   }else
								if((z_i>pos2_1) && (z_i<pos2_2)){
								   slice_ptr[y_i*sizeX +x_i] = val3;
								}else{
									slice_ptr[y_i*sizeX +x_i] = val1 + val3*rnd_f1;
								}
						   }
						}
						break;
					case 1: //surface
						{
						   float setVal = 0.5*val2*rand()/float( RAND_MAX );

							float center = (x_i-centerX)*(x_i-centerX) + (y_i-centerY)*(y_i-centerY) ;
							int z_pos1 = (int)(sizeZ*0.1 + 0.005*sizeZ*rand()/float( RAND_MAX ) + center*0.05);
							float diff1 = z_i-z_pos1;
							diff1 *=diff1;

							int z_pos2 = (int)(sizeZ*0.4 + 0.005*sizeZ*rand()/float( RAND_MAX ) + center*0.016);
							float diff2 = z_i-z_pos2;
							diff2 *= diff2;

							if(diff1<4.18){
								setVal += val3;
							}else if(diff2<4.18) {
								setVal += val3;
							}else{
								setVal =  setVal;//val_max*z_i/sizeZ*0.1;
							}

							slice_ptr[y_i*sizeX +x_i] = setVal;
						}
						break;
					case 2: //gray grad
						{
						   float setVal = z_i*(val3-val1)/(sizeZ-1.0);

							slice_ptr[y_i*sizeX +x_i] = setVal;
						}
						break;
					
					case 3: //ball plane
						{
						   float setVal = 0.002*val2*rand()/float( RAND_MAX );

							float c_d1	=	(x_i-centerX)*(x_i-centerX) + 
											(y_i-centerY)*(y_i-centerY) +
											(z_i-centerZ1)*(z_i-centerZ1) ;
							float c_d2	=	(x_i-centerX)*(x_i-centerX) + 
											(y_i-centerY)*(y_i-centerY) +
											(z_i-centerZ2)*(z_i-centerZ2) ;
						//	center = sqrt(center);

							float d_rand =  dd_level2*(1.0+0.00001*rand()/float( RAND_MAX )) ;
							float dz_rand = dd_level2*(0.1+0.00001*rand()/float( RAND_MAX )) ;
							 
							float diff_z = (z_i-centerZ)*(z_i-centerZ);
							if(	(c_d1<d_rand) || 
								(c_d2<d_rand) ||
								(diff_z<dz_rand)){
								setVal += val3;
							}else{
								setVal =  setVal;//val_max*z_i/sizeZ*0.1;
							}

							slice_ptr[y_i*sizeX +x_i] = setVal;
						}
						break;
						case 4: //grid
						{
						  if(	((x_i%grid_dd) == 0) &&
								((y_i%grid_dd) == 0) &&
								((z_i%grid_dd) == 0) 
								){
							  slice_ptr[y_i*sizeX +x_i] = grid_val;
						  }else{
							  slice_ptr[y_i*sizeX + x_i] = (dataT)0.1;
						  }
						   
						}
						break;
							
						case 5: //diagonal
						{
						  if(	(x_i == y_i) &&
								(y_i == z_i) 
								){
							  slice_ptr[y_i*sizeX +x_i] = grid_val;
						  }else{
							  slice_ptr[y_i*sizeX + x_i] = (dataT)0.1;
						  }
						   
						}
						break;
						case 6: //block
						{
						  if(	(x_i <(centerX+delta_dd) )   && (x_i >(centerX-delta_dd) ) 
								){
							  slice_ptr[y_i*sizeX +x_i] = val2; //high level
						  }else{
							  slice_ptr[y_i*sizeX +x_i] = val1;
						  }
						   
						}
						break;
						case 10: //black all zero
						{
						   
							 slice_ptr[y_i*sizeX +x_i] = val1; //high level
						   
						   
						}
						break;
					}
				}
				 
			}
		}
		 
}
template<class dataT> void SmoothTestData( TrCtCbctCubeData *dataBuff ,dataT dumy_data)
{
	int sizeX,sizeY, sizeZ;
	dataBuff->getDim(sizeX,sizeY,sizeZ);

	float sum_temp;
	float dd0  = 0.6f;
	float dd1  = 0.2f;
	float dd01 = dd0*dd1;
	float dd11 = dd1*dd1;
	float dd111 = dd1*dd1*dd1;
	float dd011 = dd0*dd1*dd1;

	float dd_sum =
		dd111 +	dd011 +	dd111	+
		dd011 +	dd01  +	dd011	+
 		dd111 +	dd011 +	dd111	+
		dd011 +	dd01  +	dd011	+
		dd01  +	dd0   +	dd01 	+
 		dd011 +	dd01  +	dd011	+
		dd111 +	dd011 +	dd111	+
		dd011 +	dd01  +	dd011	+
 		dd111 +	dd011 +	dd111	 
		;


	dd0 /= dd_sum;
	dd1 /= dd_sum;
	dd01 /= dd_sum;
	dd11 /= dd_sum;
	dd111 /= dd_sum;
	dd011 /= dd_sum;

	for(int z_i=0;z_i<sizeZ;z_i++){
		dataT *slice_ptr_nn = (dataT *)dataBuff->getRingSliceZ(z_i-1);
		dataT *slice_ptr_cc = (dataT *)dataBuff->getRingSliceZ(z_i);
		dataT *slice_ptr_pp = (dataT *)dataBuff->getRingSliceZ(z_i+1);
		for(int y_i=0;y_i<sizeY;y_i++){
			int y_nn = (y_i-1+sizeY)%sizeY;
			int y_pp = (y_i+1+sizeY)%sizeY;
			for(int x_i=0;x_i<sizeX;x_i++){
				int x_nn = (x_i-1+sizeX)%sizeX;
				int x_pp = (x_i+1+sizeX)%sizeX;

				sum_temp =	
							// z nn
							dd111*slice_ptr_nn[y_nn*sizeX + x_nn] +	dd011*slice_ptr_nn[y_nn*sizeX + x_i] +	dd111*slice_ptr_nn[y_nn*sizeX + x_pp]	+
							dd011*slice_ptr_nn[ y_i*sizeX + x_nn] +	dd01 *slice_ptr_nn[ y_i*sizeX + x_i] +	dd011*slice_ptr_nn[ y_i*sizeX + x_pp]	+
					 		dd111*slice_ptr_nn[y_pp*sizeX + x_nn] +	dd011*slice_ptr_nn[y_pp*sizeX + x_i] +	dd111*slice_ptr_nn[y_pp*sizeX + x_pp]	+
							// z cc
							dd011*slice_ptr_cc[y_nn*sizeX + x_nn] +	dd01 *slice_ptr_cc[y_nn*sizeX + x_i] +	dd011*slice_ptr_cc[y_nn*sizeX + x_pp]	+
							dd01 *slice_ptr_cc[ y_i*sizeX + x_nn] +	dd0  *slice_ptr_cc[ y_i*sizeX + x_i] +	dd01 *slice_ptr_cc[ y_i*sizeX + x_pp]	+
					 		dd011*slice_ptr_cc[y_pp*sizeX + x_nn] +	dd01 *slice_ptr_cc[y_pp*sizeX + x_i] +	dd011*slice_ptr_cc[y_pp*sizeX + x_pp]	+
							// z pp
							dd111*slice_ptr_pp[y_nn*sizeX + x_nn] +	dd011*slice_ptr_pp[y_nn*sizeX + x_i] +	dd111*slice_ptr_pp[y_nn*sizeX + x_pp]	+
							dd011*slice_ptr_pp[ y_i*sizeX + x_nn] +	dd01 *slice_ptr_pp[ y_i*sizeX + x_i] +	dd011*slice_ptr_pp[ y_i*sizeX + x_pp]	+
					 		dd111*slice_ptr_pp[y_pp*sizeX + x_nn] +	dd011*slice_ptr_pp[y_pp*sizeX + x_i] +	dd111*slice_ptr_pp[y_pp*sizeX + x_pp]	 
							;

				slice_ptr_cc[y_i*sizeX + x_i]	= (dataT)sum_temp;
			}
		}
	}

}
void COctDatBase::genData(DATA_TYPE dataType,int patID,float ref_val1,float ref_val2)
{
	m_dataBuff->setDataType( dataType);
	m_dataBuff->setDim(m_sizeX,m_sizeY,m_sizeZ);
	m_dataBuff->allocDataBuffer(true);

	//
	switch( m_dataBuff->getDataType()){
		case DataType_INT:
			genTestData(patID,	m_dataBuff,	(int)10,(int)100,(int)1000);

			SmoothTestData(m_dataBuff	, (  int)0);
			break;
		case DataType_UINT:	
			genTestData(patID,	m_dataBuff,	(unsigned int)10,(unsigned int)100,(unsigned int)1000);

			SmoothTestData(m_dataBuff	, ( unsigned int)0);
			break;							
		case DataType_SHORT:
			if(patID == 6){
				genTestData(6,	m_dataBuff,	( short)(ref_val1+0.5),( short)(ref_val2+0.5),( short)(ref_val2+0.5));
			}else{
				if (patID == 12){
					genMultiObjSlice(m_dataBuff, (short)(ref_val1 + 0.5), (short)(ref_val2 + 0.5) );
				}else
				if(patID == 10){
					genTestData(patID,	m_dataBuff,	( short)ref_val1,( short)0,( short)0);
				}else{
					genTestData(patID,	m_dataBuff,	( short)10,( short)100,( short)1000);
					SmoothTestData(m_dataBuff	, ( short)0);
				}
			}
			break;	
		case DataType_USHORT:
			if(patID == 6){
				genTestData(6,	m_dataBuff,	( short)0,( short)(ref_val2+0.5),( short)(ref_val2+0.5));
			}else{
				if(patID == 10){
					genTestData(patID,	m_dataBuff,	(unsigned short)ref_val1,(unsigned short)0,( unsigned short)0);
				}else{
					genTestData(patID,	m_dataBuff,	(unsigned short)10,(unsigned short)100,(unsigned short)1000);
				}
			}
#ifndef SMALL_GRID
			SmoothTestData(m_dataBuff	, (unsigned short)0);
			SmoothTestData(m_dataBuff	, (unsigned short)0);
			SmoothTestData(m_dataBuff	, (unsigned short)0);
#endif
			break;															
		case DataType_FLOAT:					
			 
			genTestData(patID,	m_dataBuff,	(float)10,(float)100,(float)1000);

			SmoothTestData(m_dataBuff	, ( float)0);
			break;								
		case DataType_DOUBLE:					
			 
			genTestData(patID,	m_dataBuff,	(double)10,(double)100,(double)1000);

			SmoothTestData(m_dataBuff	, ( double)0);
			break;	
		case DataType_CHAR:					
			 
			genTestData(patID,	m_dataBuff,	(unsigned char)5,(unsigned char)10,(unsigned char)100);

			SmoothTestData(m_dataBuff	, ( unsigned char)0);
			break;
		default:
			break;
	}
	 
}

 
