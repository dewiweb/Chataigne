/*
  ==============================================================================

    GamepadModule.cpp
    Created: 26 Dec 2016 4:56:31pm
    Author:  Ben

  ==============================================================================
*/

#include "GamepadModule.h"


GamepadModule::GamepadModule(const String & name) :
	Module(name),
	calibCC("Calibration")
{
	setupIOConfiguration(true, false);
	gamepadParam = new GamepadParameter("Device", "The Gamepad to connect to");
	moduleParams.addParameter(gamepadParam);

	for (int i = 0; i < 4; i++)
	{
		axisOffset.add(calibCC.addFloatParameter("Axis " + String(i + 1) + " Offset", "Offset if axis is not centered", 0, -1, 1));
		axisDeadzone.add(calibCC.addFloatParameter("Axis " + String(i + 1) + " Dead zone", "Percentage of dead zone in the center to avoid noisy input", 0, 0, 1));
	}

	moduleParams.addChildControllableContainer(&calibCC);

	InputSystemManager::getInstance()->addInputManagerListener(this);
}

GamepadModule::~GamepadModule()
{
	if(InputSystemManager::getInstanceWithoutCreating() != nullptr) InputSystemManager::getInstance()->removeInputManagerListener(this);
}

void GamepadModule::rebuildValues()
{
	valuesCC.clear();
	if (gamepadParam->gamepad == nullptr) return;
	valuesCC.addChildControllableContainer(&gamepadParam->gamepad->axesCC);
	valuesCC.addChildControllableContainer(&gamepadParam->gamepad->buttonsCC);
}


void GamepadModule::gamepadAdded(Gamepad * g)
{
	String gName = String(SDL_GameControllerName(g->gamepad));
	if (gName == gamepadParam->ghostName)
	{
		gamepadParam->setGamepad(g);
		rebuildValues();
	}
}


void GamepadModule::gamepadRemoved(Gamepad * g)
{
	if (gamepadParam->gamepad == g)
	{
		valuesCC.removeChildControllableContainer(&gamepadParam->gamepad->axesCC);
		valuesCC.removeChildControllableContainer(&gamepadParam->gamepad->buttonsCC);
	}
}

void GamepadModule::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	Module::onControllableFeedbackUpdateInternal(cc, c);
	if (c == gamepadParam)
	{
		rebuildValues();
	}
	else if (cc == &gamepadParam->gamepad->axesCC || cc == &gamepadParam->gamepad->buttonsCC)
	{
		DBG("Game pad param changed");
	}

	if (c == gamepadParam || c->parentContainer == &calibCC)
	{
		if (gamepadParam->gamepad != nullptr)
		{
			for (int i = 0; i < 4; i++)
			{
				gamepadParam->gamepad->axisOffset[i] = axisOffset[i]->floatValue();
				gamepadParam->gamepad->axisDeadZone[i] = axisDeadzone[i]->floatValue();
			}
			gamepadParam->gamepad->update();

		}
	}
}
