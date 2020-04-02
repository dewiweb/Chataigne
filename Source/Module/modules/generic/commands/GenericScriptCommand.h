/*
  ==============================================================================

	GenericScriptCommand.h
	Created: 6 Apr 2019 1:01:44pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../ChataigneGenericModule.h"
#include "Common/Command/BaseCommand.h"

class GenericScriptCommand :
	public BaseCommand
{
public:
	GenericScriptCommand(ChataigneGenericModule * _module, CommandContext context, var params);
	~GenericScriptCommand();

	static String commandScriptTemplate;
	Script script;

	const Identifier setValueId = "setValue";
	const Identifier triggerId = "trigger";

	void setValueInternal(var value) override;
	void triggerInternal() override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	static BaseCommand * create(ControllableContainer * module, CommandContext context, var params);
};