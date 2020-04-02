/*
  ==============================================================================

    SequenceCommand.h
    Created: 20 Feb 2017 2:12:09pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Common/Command/BaseCommand.h"

class SequenceModule;

class SequenceCommand :
	public BaseCommand,
	public EngineListener
{
public:
	SequenceCommand(SequenceModule * _module, CommandContext context, var params);
	virtual ~SequenceCommand();

	enum ActionType { PLAY_SEQUENCE, PAUSE_SEQUENCE, STOP_SEQUENCE, STOP_ALL_SEQUENCES, TOGGLE_SEQUENCE, ENABLE_LAYER, DISABLE_LAYER, TOGGLE_LAYER, SET_TIME, MOVE_TIME, GOTO_CUE};

	ActionType actionType;
	SequenceModule * sequenceModule;

	BoolParameter * playFromStart;

	TargetParameter * target;
	FloatParameter * value;

	virtual void triggerInternal() override;

	var dataToLoad;

	virtual void loadJSONDataInternal(var data) override;
	virtual void endLoadFile() override;

	static BaseCommand * create(ControllableContainer * module, CommandContext context, var params);
};
