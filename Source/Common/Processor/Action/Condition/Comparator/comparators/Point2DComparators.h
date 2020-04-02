/*
  ==============================================================================

    Point2DComparators.h
    Created: 2 Nov 2016 8:58:14pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../BaseComparator.h"

class Point2DComparator :
	public ParameterComparator
{
public:
	Point2DComparator(Controllable * c);
	virtual ~Point2DComparator();

	const Identifier equalsId = "=";
	const Identifier distGreaterId = "dist>";
	const Identifier distLessId = "dist<";
	const Identifier magnGreaterId = "magn>";
	const Identifier magnLessId = "magn>";


	Point2DParameter * p2dParam;
	Point2DParameter * p2dRef;
	FloatParameter * valParam;

	virtual void compare() override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Point2DComparator)
};