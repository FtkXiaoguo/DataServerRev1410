uniform sampler2D sceneTex; // 0
//uniform sampler1D lutTex; // 1 //changed LUT
uniform float filterCoef[64] ;
uniform int gw_X = 5;  
uniform int gw_Y = 5;
uniform int rt_w  ; // render target width
uniform int rt_h  ; // render target height
uniform float gain=1.0 ;    //changed LUT
uniform float offset=0.2 ;  //changed LUT

void main() 
{ 
	vec3 tc = vec3(1.0, 1.0, 1.0);

	vec2 uv = gl_TexCoord[0].xy;
	
	int gw_y_c = gw_Y/2 ;
	int gw_x_c = gw_X/2 ;
	
	float pix = 0.0;
	for(int j=0;j<gw_Y;j++){
	 float g_y = filterCoef[j];
	   for(int i=0;i<gw_X;i++){
	 		float g_x = filterCoef[i];
			vec2 pos = uv + vec2(float(i-gw_x_c)/rt_w,float(j-gw_y_c)/rt_h);
			float org_temp = texture2D(sceneTex, pos).x *g_y*g_x;
			pix +=  org_temp;
	   }
	}

//	vec4 lut= texture1D(lutTex,pix);	//changed LUT
//	vec3 disp_v =  tc*lut.w;			//changed LUT
	float pix_r = pix*gain + offset;	//changed LUT
	vec3 disp_v =  tc*pix_r;			//changed LUT
	
	gl_FragColor = vec4(disp_v,1.0);
 
}

