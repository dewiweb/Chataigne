/*
  ==============================================================================

    StringComparators.cpp
    Created: 2 Nov 2016 8:58:35pm
    Author:  bkupe

  ==============================================================================
*/

#include "StringComparators.h"


StringComparator::StringComparator(Controllable * c) :
	ParameterComparator(c),
	stringParam((StringParameter *)c)
{
	stringRef = addStringParameter("Reference", "Comparison Reference to check against source value", stringParam->stringValue());
	stringRef->setValue(stringParam->stringValue(), false, true, true);
	reference = stringRef;

	addCompareOption("=", equalsId);
	addCompareOption("!=", differentId);
	addCompareOption("Contains", containsId);
	addCompareOption("Starts with", startsWith);
	addCompareOption("Ends with", endsWidth);
}

StringComparator::~StringComparator()
{
}

void StringComparator::compare()
{
	if (currentFunctionId == equalsId)				setValid(stringParam->stringValue() == stringRef->stringValue());
	else if (currentFunctionId == differentId)		setValid(stringParam->stringValue() != stringRef->stringValue());
	else if (currentFunctionId == containsId)		setValid(stringParam->stringValue().contains(stringRef->stringValue()));
	else if (currentFunctionId == startsWith)		setValid(stringParam->stringValue().startsWith(stringRef->stringValue()));
	else if (currentFunctionId == endsWidth)		setValid(stringParam->stringValue().endsWith(stringRef->stringValue()));
}