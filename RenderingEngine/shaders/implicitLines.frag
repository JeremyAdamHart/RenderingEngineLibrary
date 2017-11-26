#version 330

// first output is mapped to the framebuffer's colour index by default
out vec4 PixelColour;

in vec2 FragmentTexCoord;

float distToLine(vec2 p, vec2 a, vec2 b){
	vec2 d = b - a;
	vec2 d_perp = normalize(vec2(-d.y, d.x));
	if(dot(p-a, d) > 0 && dot(p-b, -d) > 0)
		return abs(dot(p-a, d_perp));
	else if(dot(p-b, -d) < 0)
		return length(p-b);
	else
		return length(p-a);
}

vec2 closestPoint(vec2 p, vec2 a, vec2 b){
	vec2 d = b - a;
	vec2 d_perp = normalize(vec2(-d.y, d.x));
	vec2 c = dot(normalize(d), p-a)*normalize(d)+a;
	if(dot(p-a, d) > 0 && dot(p-b, -d) > 0)
		return c;
	else if(dot(p-b, -d) < 0)
		return b;
	else
		return a;
}
//v = distance, w = width of trunk, s = support
float basis(float v, float w, float s){
	float u = max(v-w, 0);
	return clamp(1 - sqrt(u/s), 0, 1);
} 

//v = distance, w = width of trunk, s = support
float basis2(float v, float w, float s){
	float u = max((v-w)/s, 0);
	return clamp(pow(pow(1-u, 2), 3), 0, 1);
}

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
 
const float E = 0.0001;
const vec2 dxE = vec2(E, 0);
const vec2 dyE = vec2(0, E);

float calculateIntensity(vec2 norm_coord){
	vec2 o1 = vec2(0, -0.25);
	vec2 o2 = vec2(0, -1);
	vec2 o3 = vec2(0, 0.25);
	vec2 d1 = vec2(1, 1)*0.5;
	vec2 d2 = vec2(0, 1.0);
	vec2 d3 = vec2(-1, 1);

	vec2 a = closestPoint(norm_coord, o1, d1);
	vec2 b = closestPoint(norm_coord, o2, d2);
	vec2 c = closestPoint(norm_coord, o3, d3);

	float dab = distToLine(norm_coord, a, b);
	float dbc = distToLine(norm_coord, b, c);
	float dca = distToLine(norm_coord, c, a);

	//return basis(dab, 0.08, 0.08);

	return basis(distToLine(norm_coord, o1, d1), 0.08, 0.08) + 
						basis(distToLine(norm_coord, o3, d3), 0.0625, 0.0625) +
						basis(distToLine(norm_coord, o2, d2), 0.15, 0.08);
}

vec2 distGradient(vec2 coord, vec2 o, vec2 d, float width){
	vec2 xE = vec2(0.001, 0);
	vec2 yE = vec2(0, 0.001);

	float dx = (distToLine(coord+xE, o, d) - width)
			- (distToLine(coord-xE, o, d) - width);
	float dy = (distToLine(coord+yE, o, d) - width)
			- (distToLine(coord-yE, o, d) - width);
	return vec2(dx, dy);
}

float superEllipticalBlend(vec2 norm_coord){
	vec2 o1 = vec2(0, -0.25);
	vec2 o2 = vec2(0, -1);
	vec2 o3 = vec2(1, 1)*0.5;	//vec2(0, 0.25);
	vec2 d1 = vec2(1, 1)*0.5;
	vec2 d2 = vec2(0, 1.0);
	vec2 d3 = vec2(0.75, -0.25);

	const int NUM = 3;

	float p[NUM];
	p[0] = distToLine(norm_coord, o1, d1) - 0.08; 
	p[1] = distToLine(norm_coord, o3, d3) - 0.07;
	p[2] = distToLine(norm_coord, o2, d2) - 0.15;

	vec2 g[NUM];
	g[0] = normalize(distGradient(norm_coord, o1, d1, 0.08));
	g[1] = normalize(distGradient(norm_coord, o3, d3, 0.07 ));
	g[2] = normalize(distGradient(norm_coord, o2, d2, 0.15));

	float r[NUM] = float[](0.2, 0.1, 0.4);
	float f[NUM];
	f[0] = pow(max(1-p[0]/r[0], 0), 3);
	f[1] = pow(max(1-p[1]/r[1], 0), 3);
	f[2] = pow(max(1-p[2]/r[2], 0), 3);

	float b01 = 1-f[0]/(f[0] + f[1]);
	float b12 = 1-f[1]/(f[1] + f[2]);
	float b20 = 1-f[2]/(f[2] + f[0]);

	float R[NUM];
	R[0] = (1-b12)*dot(g[0], g[1]) + (1-b12)*dot(g[0], g[2]);
	R[1] = b20*dot(g[0], g[1]) + (1-b20)*dot(g[1], g[2]);
	R[2] = b01*dot(g[2], g[1]) + (1-b01)*dot(g[0], g[2]);


	float sum = 1.0;

	for(int i=0; i<3; i++){
		sum -= pow(max(1 - p[i]/r[i], 0), 3);
	}

/*	for(int i=0; i<3; i++){
		float weight = r[i]*clamp(1-R[i], 0, 0.99);
		sum -= pow(max(1 - p[i]/weight, 0), 3);
	}

	sum = 1.0 - pow(max(1 - p[2]/(r[2]*(1-clamp(dot(g[0], g[2]), 0, 0.99))), 0), 3)
			  - pow(max(1 - p[0]/(r[0]*(1-clamp(dot(g[0], g[2]), 0, 0.99))), 0), 3); 
*/
	return sum;
}

void main(void)
{
	vec2 norm_coord = FragmentTexCoord*2.0 - vec2(1, 1);

	float intensity = calculateIntensity(norm_coord);
	intensity = -1*superEllipticalBlend(norm_coord);
	/*float dx = calculateIntensity(norm_coord+vec2(E, 0)) - 
				calculateIntensity(norm_coord-vec2(E, 0));
	float dy = calculateIntensity(norm_coord+vec2(0, E)) - 
				calculateIntensity(norm_coord-vec2(0, E));

	vec2 gradient;
	if(abs(dx + dy) < 0.000001)
		gradient = vec2(0, 0);
	else
		gradient = 0.5*normalize(vec2(dx, dy)) + vec2(0.5, 0.5);
	*/
	vec3 color = vec3(1, 1, 1);

	if(intensity < 1){		//Or 0.5
		color = vec3(0.2, 1.0, 0.7);
	//	intensity = 0;
	}
	else
		intensity = intensity;

	if (intensity> 0){
		color = vec3(1, 1, 1);
		intensity = 1;	
	}
//	intensity *= 0.5;
//	PixelColour = vec4(intensity*color, 1);
	PixelColour = vec4(color*intensity, 1);
}

/*
void main(void)
{
	vec2 o1 = vec2(0, 0);
	vec2 o2 = vec2(0, -1);
	vec2 o3 = vec2(0, 0);
	vec2 d1 = vec2(1, 1);
	vec2 d2 = vec2(0, 1.0);
	vec2 d3 = vec2(-1, 1);
	vec2 norm_coord = FragmentTexCoord*2.0 - vec2(1, 1);

	float intensity = f(distToLine(norm_coord, o1, d1), 0.125, 0.25) + 
						f(distToLine(norm_coord, o3, d3), 0.125, 0.25) +
						f(distToLine(norm_coord, o2, d2), 0.25, 0.5);	//trunkWidth(0.35, 0.25, norm_coord.y)) +

vec3 color = vec3(1, 1, 1);
	if(intensity < 0.5){
		color = vec3(0.2, 1.0, 0.7);
	//	intensity = 0;
	}
	else
		intensity = 1;
	
	PixelColour = vec4(intensity*color, 1);
}
*/