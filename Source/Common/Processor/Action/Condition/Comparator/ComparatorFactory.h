/*
  ==============================================================================

    ComparatorFactory.h
    Created: 2 Nov 2016 10:54:37pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "BaseComparator.h"
#include "comparators/BoolComparators.h"
#include "comparators/NumberComparators.h"
#include "comparators/StringComparators.h"
#include "comparators/EnumComparator.h"
#include "comparators/Point2DComparators.h"
#include "comparators/Point3DComparators.h"

class ComparatorFactory
{
public:
	static BaseComparator * createComparatorForControllable(Controllable * c)
	{
		switch (c->type)
		{
		case Controllable::TRIGGER:
			return new TriggerComparator(c);
			break;

		case Controllable::BOOL:
			return new BoolComparator(c);
			break;

		case Controllable::INT:
			return new IntComparator(c);
			break;

		case Controllable::FLOAT:
			return new FloatComparator(c);
			break;

		case Controllable::STRING:
			return new StringComparator(c);
			break;

		case Controllable::ENUM:
			return new EnumComparator(c);
			break;

		case Controllable::POINT2D:
			return new Point2DComparator(c);
			break;

		case Controllable::POINT3D:
			return new Point3DComparator(c);
			break;
			
		default:
		    //not handled now
		    break;

		}
		return nullptr;
	}

	
};