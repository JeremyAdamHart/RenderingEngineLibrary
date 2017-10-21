#version 330

// first output is mapped to the framebuffer's colour index by default
out vec4 PixelColour;

in vec2 FragmentTexCoord;

float distToLine(vec2 p, vec2 o, vec2 d){
	vec2 op = p - o;
	vec2 d_perp = normalize(vec2(-d.y, d.x));
	if(dot(op, d) > 0)
		return abs(dot(op, d_perp));
	else
		return length(op);
}

/*float distToLineSeg(vec2 p, vec2 a, vec2 b){
	vec2 op = p - o;
	vec2 d_perp = normalize(vec2(-d.y, d.x));
	if(dot(op, d) > 0)
		return abs(dot(op, d_perp));
	else
		return length(op);
}*/

float cubicBasis(float v){
	float u = clamp(v, 0, 1);
	return 2*u*u*u - 3*u*u + 1;
}

//Value, width, support
float modularBasis(float v, float w){
	mat3 system = mat3(vec3(1, 4, w*w*w*w),
						vec3(1, 3, w*w*w),
						vec3(1, 2, w*w));
	vec3 coeff = inverse(system)*vec3(-1, 0, -0.5);

	float u = clamp(v, 0, 1);

	return coeff.x*u*u*u*u + coeff.y*u*u*u + coeff.z*u*u + 1;
}

float fade(float v){
	float t = clamp(v, 0, 1);
	return 1 - (t*t*t*(t*(t*6 - 15) + 10));
}
//Distance and width
float f(float d, float w, float s){
//	return fade(d/s-w);
	return modularBasis(d/s, w/s);
}

//1 = a - ad^n
//a-1 = ad^n
//(a-1)/a = d^n
//sqrt^n((a-1)/a) = d

float trunkWidth(float bottomWidth, float topWidth, float y){
	float u = (y +1)/2;
	return (1-u)*bottomWidth + u*topWidth;
}

void main(void)
{
	vec2 o1 = vec2(0, 0);
	vec2 o2 = vec2(0, -1);
	vec2 o3 = vec2(0, 0);
	vec2 d1 = vec2(1, 0.1);
	vec2 d2 = vec2(0, 1);
	vec2 d3 = vec2(-1, 1);
	vec2 norm_coord = FragmentTexCoord*2.0 - vec2(1, 1);
/*	float intensity = max(max(f(distToLine(norm_coord, o1, d1), 0.125), 
						 f(distToLine(norm_coord, o2, d2), 0.25)),
						 f(distToLine(norm_coord, o3, d3), 0.125));*/
	float intensity = f(distToLine(norm_coord, o1, d1), 0.125, 0.25) + 
						f(distToLine(norm_coord, o2, d2), 0.25, 0.5) +//trunkWidth(0.35, 0.25, norm_coord.y)) +
						f(distToLine(norm_coord, o3, d3), 0.125, 0.25);

///*
	if(intensity < 0.5)
		intensity = 0;
	else
		intensity = 1;
//*/	
	PixelColour = vec4(intensity*vec3(1, 1, 1), 1);
}
