/*
  ==============================================================================

    InverseFilter.cpp
    Created: 4 Feb 2017 5:39:15pm
    Author:  Ben

  ==============================================================================
*/

#include "InverseFilter.h"


InverseFilter::InverseFilter(var params) :
	MappingFilter(getTypeString(), params)
{
	editorCanBeCollapsed = false;
	editorIsCollapsed = true;

	filterTypeFilters.add(Controllable::FLOAT, Controllable::INT);
}

InverseFilter::~InverseFilter()
{
}

void InverseFilter::processSingleParameterInternal(Parameter * source, Parameter * out)
{
	if (!source->hasRange())
	{
		out->setValue(source->getValue());
		return;
	}

	out->setValue(jmap<float>(source->getNormalizedValue(), source->maximumValue, source->minimumValue));
	
	/*if (sourceParam->isComplex())
	{
		var val;
		int numValToInverse = sourceParam->type != Controllable::COLOR ? sourceParam->value.size() : 3; //do not invert alpha by default (may improve to have an option)
		
		if (sourceParam->minimumValue.size() != sourceParam->value.size())
		{
			DBG("PROBLEM HERE !");
			return;
		}

		for (int i = 0; i < numValToInverse && i < sourceParam->value.size(); i++)
		{
			DBG("sizes " << sourceParam->value.size() << " / " << sourceParam->minimumValue.size() << " / " << sourceParam->maximumValue.size());

			float normVal = ((float)sourceParam->value[i] - (float)sourceParam->minimumValue[i]) / ((float)sourceParam->maximumValue[i] - (float)sourceParam->minimumValue[i]);
			val.append(jmap<float>(normVal, sourceParam->maximumValue[i], sourceParam->minimumValue[i]));
		}
		for (int i = numValToInverse; i < sourceParam->value.size(); i++)
		{
			val.append(sourceParam->value[i]);
		}

	
		filteredParameter->setValue(val);

		for (int i = 0; i < val.size(); i++)
		{
			DBG("Filtered Value " << i << " : " << (float)filteredParameter->value[i] << " / " << (float)val[i]);
		}

	}
	else
	{*/
	//}
}
