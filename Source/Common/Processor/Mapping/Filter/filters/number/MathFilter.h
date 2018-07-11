/*
  ==============================================================================

    MathFilter.h
    Created: 4 Jul 2018 2:15:50pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../../MappingFilter.h"

class MathFilter :
	public MappingFilter
{
public:
	MathFilter(var params);
	~MathFilter();

	enum Operation { OFFSET, MULTIPLY, DIVIDE, MODULO, FLOOR, CEIL, ROUND };

	EnumParameter * operation;
	Parameter * operationValue;

	Parameter * setupParameterInternal(Parameter * sourceParam) override;
	void processInternal() override;

	void updateFilteredParamRange(Parameter * p = nullptr);
	void filterParamChanged(Parameter * p) override;

	float getProcessedValue(float val, int index = -1);

	bool filteredParamShouldHaveRange();

	static MathFilter * create(var params) { return new MathFilter(params); }
	virtual String getTypeString() const override { return "Math"; }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MathFilter)
};