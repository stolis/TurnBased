// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include<iostream>
#include<iomanip>
#include<cmath>
#include "CoreMinimal.h"

/**
 * 
 */
class TURNBASED_API CoordinateTools
{
public:
	CoordinateTools();
	~CoordinateTools();
	static double degreeToRadian(const double degree);
	static double radianToDegree(const double radian);
	static double CoordinatesToAngle(double latitude1, const double longitude1, double latitude2, const double longitude2);
	static double CoordinatesToMeters(double latitude1, double longitude1, double latitude2, double longitude2);
	static void GeoDeticOffsetInv(double, double, double, double, double&, double&);
};
