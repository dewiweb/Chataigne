/*
  ==============================================================================

    MIDIDeviceParameter.h
    Created: 20 Dec 2016 3:05:54pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "MIDIManager.h"

class MIDIDeviceParameterUI;

class MIDIDeviceParameter :
	public Parameter,
	public MIDIManager::Listener
{
public:
	MIDIDeviceParameter(const String &name);
	~MIDIDeviceParameter();

	MIDIInputDevice * inputDevice;
	MIDIOutputDevice * outputDevice;
	
	String ghostDeviceIn;
	String ghostDeviceOut;

	void setInputDevice(MIDIInputDevice * i);
	void setOutputDevice(MIDIOutputDevice * o);

	// Inherited via Listener
	virtual void midiDeviceInAdded(MIDIInputDevice *) override;
	virtual void midiDeviceOutAdded(MIDIOutputDevice *) override;
	virtual void midiDeviceInRemoved(MIDIInputDevice *) override;
	virtual void midiDeviceOutRemoved(MIDIOutputDevice *) override;

	MIDIDeviceParameterUI * createMIDIParameterUI();
	ControllableUI * createDefaultUI(Controllable * targetControllable) override;

	void loadJSONDataInternal(var data) override;

};