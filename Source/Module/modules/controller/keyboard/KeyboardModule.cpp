/*
  ==============================================================================

    KeyboardModule.cpp
    Created: 15 Mar 2018 9:36:44am
    Author:  Ben

  ==============================================================================
*/

#include "KeyboardModule.h"
#include "commands/KeyboardModuleCommands.h"


#if JUCE_WINDOWS
#include <windows.h>
#define KEY_CTRL VK_CONTROL
#define KEY_ALT VK_MENU
#define KEY_SHIFT VK_SHIFT
#elif JUCE_MAC
#include "KeyboardMacFunctions.h"
#define KEY_CTRL 55
#define KEY_ALT 58
#define KEY_SHIFT 57
#else
#define KEY_CTRL 0
#define KEY_ALT 0
#define KEY_SHIFT 0
#endif

KeyboardModule::KeyboardModule() :
	Module(getDefaultTypeString()),
	window(nullptr)
{
	setupIOConfiguration(true, true);

	window = TopLevelWindow::getActiveTopLevelWindow();
	if(window != nullptr) window->addKeyListener(this);
	
	lastKey = valuesCC.addStringParameter("Last Key", "Last Key pressed", "");
	ctrl = valuesCC.addBoolParameter("Ctrl", "Is Control down ?", false);
	shift = valuesCC.addBoolParameter("Shift", "Is shift down ?", false);
	command = valuesCC.addBoolParameter("Command", "Is command down ?", false);
	alt = valuesCC.addBoolParameter("Alt", "Is alt down ?", false);

	defManager->add(CommandDefinition::createDef(this, "", "Key Down", &KeyboardModuleCommands::create, CommandContext::ACTION)->addParam("type", KeyboardModuleCommands::KEY_DOWN));
	defManager->add(CommandDefinition::createDef(this, "", "Key Up", &KeyboardModuleCommands::create, CommandContext::ACTION)->addParam("type", KeyboardModuleCommands::KEY_UP));
	defManager->add(CommandDefinition::createDef(this, "", "Key hit", &KeyboardModuleCommands::create, CommandContext::ACTION)->addParam("type", KeyboardModuleCommands::KEY_HIT));

}

KeyboardModule::~KeyboardModule()
{
	if(TopLevelWindow::getActiveTopLevelWindow() == window) window->removeKeyListener(this);
}

void KeyboardModule::sendKeyDown(int keyID)
{
	if (!enabled->boolValue()) return;
	outActivityTrigger->trigger();

	MessageManagerLock mmLock;

#if JUCE_WINDOWS
	// Set up a generic keyboard event.
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	ip.ki.wVk = keyID; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));
#elif JUCE_MAC
   // keyboardmac::sendMacKeyEvent(keyID, true);
#endif
}

void KeyboardModule::sendKeyUp(int keyID)
{
	if (!enabled->boolValue()) return;
	outActivityTrigger->trigger();

	MessageManagerLock mmLock;

#if JUCE_WINDOWS
	// Set up a generic keyboard event.
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;

	ip.ki.wVk = keyID; // virtual-key code for the "a" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
    
#elif JUCE_MAC
   // keyboardmac::sendMacKeyEvent(keyID, false);
#endif

}

void KeyboardModule::sendKeyHit(int keyID, bool ctrlPressed, bool altPressed, bool shiftPressed)
{
	if (ctrlPressed) sendKeyDown(KEY_CTRL);
	if (altPressed) sendKeyDown(KEY_ALT);
	if (shiftPressed) sendKeyDown(KEY_SHIFT);

	sendKeyDown(keyID);
	sendKeyUp(keyID);
    
	if (ctrlPressed) sendKeyUp(KEY_CTRL);
	if (altPressed) sendKeyUp(KEY_ALT);
	if (shiftPressed) sendKeyUp(KEY_SHIFT);
}

bool KeyboardModule::keyPressed(const KeyPress & key, juce::Component * originatingComponent)
{
	char k = (char)key.getKeyCode();
	String ks = String::fromUTF8(&k, 1);
	lastKey->setValue(ks.toLowerCase());

	shift->setValue(key.getModifiers().isShiftDown());
	ctrl->setValue(key.getModifiers().isCtrlDown());
	command->setValue(key.getModifiers().isCommandDown());
	alt->setValue(key.getModifiers().isAltDown());

	return false;
}

bool KeyboardModule::keyStateChanged(bool isKeyDown, juce::Component * originatingComponent)
{
	if (!isKeyDown)
	{
		lastKey->setValue("");
		ctrl->setValue(false);
		shift->setValue(false);
		command->setValue(false);
		alt->setValue(false);
	}
	return false;
}
