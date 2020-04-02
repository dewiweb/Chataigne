/*
  ==============================================================================

    MIDIManager.h
    Created: 20 Dec 2016 12:33:33pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "MIDIDevice.h"

class MIDIManager :
	public Timer
{
public:
	juce_DeclareSingleton(MIDIManager,true)
	MIDIManager();
	~MIDIManager();

	OwnedArray<MIDIInputDevice> inputs;
	OwnedArray<MIDIOutputDevice> outputs;

	void checkDevices();
	void addInputDeviceIfNotThere(const String &name);
	void addOutputDeviceIfNotThere(const String &name);
	void removeInputDevice(MIDIInputDevice * d);
	void removeOutputDevice(MIDIOutputDevice * d);

	MIDIInputDevice * getInputDeviceWithName(const String &name);
	MIDIOutputDevice * getOutputDeviceWithName(const String &name);
	
	//Gloabal Settings
	enum MIDIEventType { NOTE_ON, NOTE_OFF, FULL_NOTE, CONTROL_CHANGE, PITCH_WHEEL, SYSEX};
	EnumParameter * midiRouterDefaultType;

	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void midiDeviceInAdded(MIDIInputDevice* /*input*/) {}
		virtual void midiDeviceOutAdded(MIDIOutputDevice* /*output*/) {}

		virtual void midiDeviceInRemoved(MIDIInputDevice* /*input*/) {}
		virtual void midiDeviceOutRemoved(MIDIOutputDevice* /*output*/) {}
	};

	ListenerList<Listener> listeners;
	void addMIDIManagerListener(Listener* newListener) { listeners.add(newListener); }
	void removeMIDIManagerListener(Listener* listener) { listeners.remove(listener); }



	// Inherited via Timer
	virtual void timerCallback() override;
	
	static String getNoteName(const int &pitch, bool includeOctave = true)
	{
		return MidiMessage::getMidiNoteName(pitch, true, includeOctave, 5);
	}

	JUCE_DECLARE_NON_COPYABLE(MIDIManager)
};
