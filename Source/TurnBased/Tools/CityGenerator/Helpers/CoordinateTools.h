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
	static double CoordinateTools::degreeToRadian(double degree);
	static double CoordinateTools::radianToDegree(double radian);
	static double CoordinateTools::CoordinatesToAngle(double latitude1, double longitude1, double latitude2, double longitude2);
	static double CoordinateTools::CoordinatesToMeters(double latitude1, double longitude1, double latitude2, double longitude2);
};
