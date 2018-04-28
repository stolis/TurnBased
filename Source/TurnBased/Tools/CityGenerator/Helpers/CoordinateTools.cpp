// Fill out your copyright notice in the Description page of Project Settings.

#include "CoordinateTools.h"

static const double earthDiameterMeters = 6371.0 * 2 * 1000;
CoordinateTools::CoordinateTools()
{
}

CoordinateTools::~CoordinateTools()
{
}

double CoordinateTools::degreeToRadian(const double degree) 
{
	return (degree * PI / 180); 
};
double CoordinateTools::radianToDegree(const double radian) 
{ 
	return (radian * 180 / PI); 
};


double CoordinateTools::CoordinatesToAngle(double latitude1,
	const double longitude1,
	double latitude2,
	const double longitude2)
{
	const auto longitudeDifference = degreeToRadian(longitude2 - longitude1);
	latitude1 = degreeToRadian(latitude1);
	latitude2 = degreeToRadian(latitude2);

	using namespace std;
	const auto x = (cos(latitude1) * sin(latitude2)) -
		(sin(latitude1) * cos(latitude2) * cos(longitudeDifference));
	const auto y = sin(longitudeDifference) * cos(latitude2);

	const auto degree = radianToDegree(atan2(y, x));
	return (degree >= 0) ? degree : (degree + 360);
}

double CoordinateTools::CoordinatesToMeters(double latitude1,
	double longitude1,
	double latitude2,
	double longitude2)
{
	latitude1 = degreeToRadian(latitude1);
	longitude1 = degreeToRadian(longitude1);
	latitude2 = degreeToRadian(latitude2);
	longitude2 = degreeToRadian(longitude2);

	using namespace std;
	auto x = sin((latitude2 - latitude1) / 2), y = sin((longitude2 - longitude1) / 2);
#if 1
	return earthDiameterMeters * asin(sqrt((x * x) + (cos(latitude1) * cos(latitude2) * y * y)));
#else
	auto value = (x * x) + (cos(latitude1) * cos(latitude2) * y * y);
	return earthDiameterMeters * atan2(sqrt(value), sqrt(1 - value));
#endif
}

void CoordinateTools::GeoDeticOffsetInv(double refLat, double refLon,
	double lat, double lon,
	double& xOffset, double& yOffset)
{
	double a = 6378137.000000; //GD_semiMajorAxis;
	double b = 6356752.314245; //GD_TranMercB;
	double f = 0.003352810664; // GD_geocentF;

	double L = lon - refLon;
	double U1 = atan((1 - f) * tan(refLat));
	double U2 = atan((1 - f) * tan(lat));
	double sinU1 = sin(U1);
	double cosU1 = cos(U1);
	double sinU2 = sin(U2);
	double cosU2 = cos(U2);

	double lambda = L;
	double lambdaP;
	double sinSigma;
	double sigma;
	double cosSigma;
	double cosSqAlpha;
	double cos2SigmaM;
	double sinLambda;
	double cosLambda;
	double sinAlpha;
	int iterLimit = 100;
	do {
		sinLambda = sin(lambda);
		cosLambda = cos(lambda);
		sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) +
			(cosU1*sinU2 - sinU1 * cosU2*cosLambda) *
			(cosU1*sinU2 - sinU1 * cosU2*cosLambda));
		if (sinSigma == 0)
		{
			xOffset = 0.0;
			yOffset = 0.0;
			return;  // co-incident points
		}
		cosSigma = sinU1 * sinU2 + cosU1 * cosU2*cosLambda;
		sigma = atan2(sinSigma, cosSigma);
		sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
		cosSqAlpha = 1 - sinAlpha * sinAlpha;
		cos2SigmaM = cosSigma - 2 * sinU1*sinU2 / cosSqAlpha;
		if (cos2SigmaM != cos2SigmaM) //isNaN
		{
			cos2SigmaM = 0;  // equatorial line: cosSqAlpha=0 (§6)
		}
		double C = f / 16 * cosSqAlpha*(4 + f * (4 - 3 * cosSqAlpha));
		lambdaP = lambda;
		lambda = L + (1 - C) * f * sinAlpha *
			(sigma + C * sinSigma*(cos2SigmaM + C * cosSigma*(-1 + 2 * cos2SigmaM*cos2SigmaM)));
	} while (fabs(lambda - lambdaP) > 1e-12 && --iterLimit>0);

	if (iterLimit == 0)
	{
		xOffset = 0.0;
		yOffset = 0.0;
		return;  // formula failed to converge
	}

	double uSq = cosSqAlpha * (a*a - b * b) / (b*b);
	double A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
	double B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));
	double deltaSigma = B * sinSigma*(cos2SigmaM + B / 4 * (cosSigma*(-1 + 2 * cos2SigmaM*cos2SigmaM) -
		B / 6 * cos2SigmaM*(-3 + 4 * sinSigma*sinSigma)*(-3 + 4 * cos2SigmaM*cos2SigmaM)));
	double s = b * A*(sigma - deltaSigma);

	double bearing = atan2(cosU2*sinLambda, cosU1*sinU2 - sinU1 * cosU2*cosLambda);
	xOffset = sin(bearing)*s;
	yOffset = cos(bearing)*s;
}
