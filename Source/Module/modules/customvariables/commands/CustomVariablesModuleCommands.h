/*
  ==============================================================================

    CustomVariablesModuleCommands.h
    Created: 23 Feb 2018 12:25:24am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Common/Command/BaseCommand.h"

class CustomVariablesModule;
class CVGroupManager;

class CVCommand :
	public BaseCommand
{
public:
	CVCommand(CustomVariablesModule * _module, CommandContext context, var params);
	virtual ~CVCommand();

	CVGroupManager * manager;

	enum Type { SET_VALUE, SET_PRESET, GO_TO_PRESET, KILL_GO_TO_PRESET, LERP_PRESETS, SET_PRESET_WEIGHT, SET_2DTARGET, LOAD_PRESET, SAVE_PRESET };
	Type type;

	TargetParameter * target;
	TargetParameter * targetPreset;
	TargetParameter * targetPreset2;
	FileParameter* presetFile;

	//interpolation
	FloatParameter* time;
	Automation* automation;
	
	enum Operator { EQUAL, INVERSE, ADD, SUBTRACT, MULTIPLY, DIVIDE };
	EnumParameter * valueOperator; 
	Parameter * value;
	var ghostValueData;

	void updateOperatorOptions();

	void onContainerParameterChanged(Parameter * p) override;
	void triggerInternal() override;

	static BaseCommand * create(ControllableContainer * module, CommandContext context, var params);
};
