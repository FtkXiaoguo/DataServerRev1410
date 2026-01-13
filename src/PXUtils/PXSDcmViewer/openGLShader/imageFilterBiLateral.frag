
uniform sampler2D sceneTex; // 0
//uniform sampler1D lutTex; // 1  //changed LUT
uniform float filterCoef[64] ;
uniform int gw_X = 5;  
uniform int gw_Y = 5;
uniform int rt_w  ; // render target width
uniform int rt_h  ; // render target height
uniform float sigma = 2.0;
uniform float gain=1.0 ;    //changed LUT
uniform float offset=0.2 ;  //changed LUT

float heuclideanLen(float pix  , float cc, float d)
{
    float mod = (pix - cc) ;
	mod = mod*mod;
	mod = -mod / (2.0 * d * d);
	return exp(mod);
}

void main() 
{ 
	vec3 tc = vec3(1.0, 1.0, 1.0);

	vec2 uv = gl_TexCoord[0].xy;
	
	float center_v = texture2D(sceneTex, uv).x ;
	
	int gw_y_c = gw_Y/2.0 ;
	int gw_x_c = gw_X/2.0 ;
	
	float pix = 0.0;
	for(int j=0;j<gw_Y;j++){
	 float g_y = filterCoef[j];
	   for(int i=0;i<gw_X;i++){
	 		float g_x = filterCoef[i];
			vec2 pos = uv + vec2(float(i-gw_x_c)/rt_w,float(j-gw_y_c)/rt_h);
			float org_temp = texture2D(sceneTex, pos).x ;
		 	float bi_v =  heuclideanLen(org_temp ,center_v,sigma);
			
			org_temp = org_temp*bi_v*g_x*g_y;
			pix +=  org_temp;
	   }
	}

//	vec4 lut= texture1D(lutTex,pix);
//	vec3 disp_v =  tc*lut.w;
	float pix_r = pix*gain + offset;	//changed LUT
	vec3 disp_v =  tc*pix_r;			//changed LUT
	
	gl_FragColor = vec4(disp_v,1.0);
 
}