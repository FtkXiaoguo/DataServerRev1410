// tstReadSTL.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"


void testRead();
int _tmain(int argc, _TCHAR* argv[])
{
	
	testRead();
	return 0;
}


#include "StlReadWrite.h"

void resizePolygon(const std::vector<CStlPolygon> &input,  std::vector<CStlPolygon> &output );
void testRead()
{
	int polysize;
	CStlReadWrite::DataRange3D polygonRange;
	

	CStlReadWrite reader;

#if 0
	reader.readStl("BL_DM33_NC_08.stl");
	reader.profile();
	reader.writeStlBinary("test_binary.stl");
	reader.writeStlText("test_text.stl");
	{
		CStlReadWrite reader1;
		reader1.readStl("test_binary.stl");
		reader1.profile();
		//
		reader1.readStl("test_text.stl");
		reader1.profile();
	}
	return;
#endif
//	reader.readStl("C:\\AQNetImplant\\imp4\\data.stl");
//	reader.profile();
//	reader.readStl("cube-ascii.stl");
//	reader.readStl("cube-binary.stl");
	
	reader.readStl("C:\\ext_data\\temp\\BL_DM48_RC_14.stl");
//	reader.profile();
	reader.profile( polysize,&polygonRange);

	reader.writeStlPXBinary("dbg_data.pxstl");
	
	{
		CStlReadWrite reader_px;
		reader_px.readStl("dbg_data.pxstl");
		reader_px.profile( polysize,&polygonRange);
		reader.writeStlBinary("dbg_data.stl");
	}
	{
		CStlReadWrite reader_resize;
	 
		resizePolygon(reader.getPolygon(),  reader_resize.getPolygon() );
//		reader_resize.profile();

		CStlReadWrite reader_resize1;
	 
		resizePolygon(reader_resize.getPolygon(),  reader_resize1.getPolygon() );
//		reader_resize1.profile();

		reader.writeStlBinary("ttt_b_o.stl");
	//	reader_resize1.writeStlText("ttt.stl");
		reader_resize.writeStlBinary("ttt_b_r.stl");
		reader_resize1.writeStlBinary("ttt_b_r1.stl");

	}
	//reader.writeStlText("ttt.stl");


}

//3 times point
inline void assign_point(float dest[3],const float src[3]  )
{
	dest[0]	= src[0]  ;
	dest[1]	= src[1]  ;
	dest[2]	= src[2]  ;
}
void resizePolygon(const std::vector<CStlPolygon> &input,  std::vector<CStlPolygon> &output )
{
	int polygon_size = input.size();
	output.resize(polygon_size*3);
	float new_point[3];
	for(int i=0;i<polygon_size;i++){
		new_point[0] = (input[i].m_point1[0] + input[i].m_point2[0] +input[i].m_point3[0])/3.0f ;
		new_point[1] = (input[i].m_point1[1] + input[i].m_point2[1] +input[i].m_point3[1])/3.0f ;
		new_point[2] = (input[i].m_point1[2] + input[i].m_point2[2] +input[i].m_point3[2])/3.0f ;

		//new polygon -1 
		assign_point(output[3*i + 0].m_point1 ,	input[i].m_point1);
		assign_point(output[3*i + 0].m_point2 ,	input[i].m_point2);
		assign_point(output[3*i + 0].m_point3 ,	new_point );

		//new polygon -2 
		assign_point(output[3*i + 1].m_point1 ,	input[i].m_point2);
		assign_point(output[3*i + 1].m_point2 ,	input[i].m_point3);
		assign_point(output[3*i + 1].m_point3 ,	new_point );

		//new polygon -3 
		assign_point(output[3*i + 2].m_point1 ,	input[i].m_point3);
		assign_point(output[3*i + 2].m_point2 ,	input[i].m_point1);
		assign_point(output[3*i + 2].m_point3 ,	new_point );
		
	}

}