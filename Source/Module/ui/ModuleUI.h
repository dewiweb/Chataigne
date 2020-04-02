/*
  ==============================================================================

    ModuleUI.h
    Created: 8 Dec 2016 2:36:51pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../Module.h"

class ModuleUI :
	public BaseItemUI<Module>,
	public Module::ModuleListener
{
public:
	ModuleUI(Module * module);
	virtual ~ModuleUI();

	virtual void resizedInternalHeader(Rectangle<int> &r) override;
	virtual void resizedInternalHeaderModule(Rectangle<int>& r) {}

	std::unique_ptr<TriggerImageUI> inActivityUI;
	std::unique_ptr<TriggerImageUI> outActivityUI;
	ImageComponent iconUI;

	std::unique_ptr<BoolImageToggleUI> connectionFeedbackUI;

	void moduleIOConfigurationChanged() override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleUI)
};