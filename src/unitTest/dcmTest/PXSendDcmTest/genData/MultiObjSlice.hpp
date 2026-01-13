
template<class dataT> void genMultiObjSlice(TrCtCbctCubeData *dataBuff, dataT val1, dataT val2 )
{
	int sizeX,sizeY, sizeZ;
	dataBuff->getDim(sizeX,sizeY,sizeZ);
 
	int x_pos1_1 = sizeX*2/8;
	int x_pos1_2 = sizeX*5/8;
	int y_pos1_1 = sizeY * 1 / 8;
	int y_pos1_2 = sizeY * 7 / 8;

	int seg_yy_num = 30;
	float dx = 1500.0f/(x_pos1_2 - x_pos1_1);
	float dy = 2500 / seg_yy_num;// (val2 - val1) / seg_yy_num;
	for(int z_i=0;z_i<sizeZ;z_i++){
		dataT *slice_ptr = (dataT *)dataBuff->getSliceZ(z_i);
		for(int y_i=0;y_i<sizeY;y_i++){
			bool b_seg_yy_on = false;
			int seg_yy = y_i - y_pos1_1;
			int seg_yy_i = seg_yy % seg_yy_num;
			int seg_yy_no = (float)seg_yy / (y_pos1_2 - y_pos1_1)* seg_yy_num;
			if ((y_i >= y_pos1_1) && (y_i<y_pos1_2)){
				b_seg_yy_on = true;
			}
			for(int x_i=0;x_i<sizeX;x_i++){
				dataT set_val = val1;
				if (b_seg_yy_on){
					int xx = x_i - x_pos1_1;
					if (xx > 0){
						if (seg_yy >= xx){
							set_val = val2;// (dataT)(val2 + xx*dx);
						}
					}
					if ((seg_yy_i > 5) && (seg_yy_i < 15) &&
						(xx > 10) && (xx < 25)){
						set_val = (dataT)(val1 + seg_yy_no*dy); //val1;
					}
				}
				slice_ptr[y_i*sizeX + x_i] = set_val;	 
			}
				 
		}
	}
		 
}
 