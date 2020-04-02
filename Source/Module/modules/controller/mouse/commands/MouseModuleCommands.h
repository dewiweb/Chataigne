/*
  ==============================================================================

    MouseCommands.h
    Created: 12 Mar 2020 3:17:55pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../MouseModule.h"

class MouseModuleCommands :
	public BaseCommand
{
public:
	MouseModuleCommands(MouseModule* m, CommandContext context, var params);
	~MouseModuleCommands();

	MouseModule* mouseModule;

	enum Type { SET_CURSOR_POSITION, BUTTON_DOWN, BUTTON_UP, BUTTON_CLICK };
	Type type;

	Point2DParameter* position;
	EnumParameter* buttonID;
	BoolParameter* isRelative;

	void triggerInternal() override;

	static BaseCommand* create(ControllableContainer* module, CommandContext context, var params) { return new MouseModuleCommands((MouseModule*)module, context, params); }

};