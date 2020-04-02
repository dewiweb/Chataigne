/*
  ==============================================================================

    Point3DComparators.h
    Created: 2 Nov 2016 8:58:22pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../BaseComparator.h"

class Point3DComparator :
	public ParameterComparator
{
public:
	Point3DComparator(Controllable * c);
	virtual ~Point3DComparator();

	const Identifier equalsId = "=";
	const Identifier distGreaterId = "dist>";
	const Identifier distLessId = "dist<";
	const Identifier magnGreaterId = "magn>";
	const Identifier magnLessId = "magn>";


	Point3DParameter * p3dParam;
	Point3DParameter * p3dRef;
	FloatParameter * valParam;

	virtual void compare() override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Point3DComparator)
};