#version 410

layout(vertices=4) out;

//in gl_in[];		//Struct containing gl_Position, gl_PointSize, and something else you'll probably never use
//out gl_out[];

void main()
{
	if(gl_InvocationID == 0)
	{
		float tessLevel = 10.0;
		gl_TessLevelOuter[0] = tessLevel;
		gl_TessLevelOuter[1] = tessLevel;		//Change this
		gl_TessLevelOuter[2] = tessLevel;
		gl_TessLevelOuter[3] = tessLevel;
		gl_TessLevelInner[0] = tessLevel;
		gl_TessLevelInner[1] = tessLevel;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
