uniform sampler2D sceneTex; // 0
//uniform sampler1D lutTex; // 1
uniform int rt_w  ; // render target width
uniform int rt_h  ; // render target height
uniform float gain=1.0 ;    //changed LUT
uniform float offset=0.2 ;  //changed LUT
uniform int	isRGBA = 0; 
void main() 
{ 
	vec3 tc = vec3(1.0, 1.0, 1.0);

	vec2 uv = gl_TexCoord[0].xy;

	if(isRGBA == 1){
	 	vec3 pix  = texture2D(sceneTex, uv).xyz;
	  	vec3 pix_r = pix*gain + offset;
		vec3 disp_v =  tc*pix_r;
		gl_FragColor = vec4(disp_v,1.0);
	}else{
		float pix = texture2D(sceneTex, uv).x ;

	//	vec4 lut= texture1D(lutTex,pix);//changed LUT
	//	vec3 disp_v =  tc*lut.w;        //changed LUT
		float pix_r = pix*gain + offset;
		vec3 disp_v =  tc*pix_r;
		
		gl_FragColor = vec4(disp_v,1.0);
	}
 
}

