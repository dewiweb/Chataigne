/*
  ==============================================================================

    SendStreamValuesCommand.h
    Created: 27 Jan 2018 12:04:01pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

#include "StreamingCommand.h"
#include "../../commands/customvalues/CustomValuesCommandArgumentManager.h"

class SendStreamValuesCommand :
	public StreamingCommand
{
public:
	SendStreamValuesCommand(StreamingModule * output, CommandContext context, var params);
	~SendStreamValuesCommand();

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	void triggerInternal() override;

	static SendStreamValuesCommand * create(ControllableContainer * module, CommandContext context, var params) { return new SendStreamValuesCommand((StreamingModule *)module, context, params); }

};
