/*
  ==============================================================================

    ConditionManagerEditor.h
    Created: 28 Oct 2016 8:07:44pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "ConditionEditor.h"
#include "../ConditionManager.h"

class ConditionManagerEditor :
	public GenericManagerEditor<Condition>
{
public:
	ConditionManagerEditor(ConditionManager *_manager, bool isRoot);
	~ConditionManagerEditor();

	ConditionManager * conditionManager;

	void itemAddedAsync(Condition *) override;
	void itemRemovedAsync(Condition *) override;

	void controllableFeedbackUpdate(Controllable * c) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConditionManagerEditor)
};