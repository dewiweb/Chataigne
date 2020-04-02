/*
  ==============================================================================

    CommandTemplateManager.h
    Created: 31 May 2018 11:29:27am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "CommandTemplate.h"
#include "../CommandDefinitionManager.h"

class Module;

class CommandTemplateManager :
	public BaseManager<CommandTemplate>,
	public CommandTemplate::TemplateListener
{
public:
	CommandTemplateManager(Module * module);
	virtual ~CommandTemplateManager();

	Module * module;
	Factory<CommandTemplate> factory;

	const String menuName = "Templates";

	CommandDefinitionManager defManager;

	void addItemInternal(CommandTemplate * item, var data) override;
	void removeItemInternal(CommandTemplate * item) override;

	CommandTemplate * addItemFromData(var data, bool addToUndo = true) override;

	void setupDefinitionsFromModule();

	void setItemIndex(CommandTemplate* t, int newIndex) override;
	void reorderItems() override;
	void reorderDefinitions();

	void templateNameChanged(CommandTemplate * ct) override;
	void loadJSONDataManagerInternal(var data) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommandTemplateManager)
};