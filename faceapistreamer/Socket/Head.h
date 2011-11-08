#pragma once

class Head
{


public:
	Head(void);
	double x,y,z;
	double rx,ry,rz;
	double confidence; // by Murivan on 07/11/2011 to export also confidence value
	~Head(void);
};
