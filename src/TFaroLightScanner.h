#pragma once

#include <ofxSoylent.h>
#include <SoyMath.h>
#include <sstream>

class TPoint
{
public:
	vec3f	mPosition;
	vec3f	mRgb;
};

class TPointCloud
{
public:
	Array<TPoint>	mPoints;
};

class TFileFaroLightScanner
{
public:
	static bool		Parse(TPointCloud& PointCloud,const ArrayBridge<char>& Binary,std::stringstream& Error);
};

