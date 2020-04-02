/*
  ==============================================================================

    DMXModule.h
    Created: 6 Apr 2017 10:22:10pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Module/Module.h"
#include "Common/DMX/device/DMXDevice.h"

class DMXModule :
	public Module,
	public DMXDevice::DMXDeviceListener
{
public:
	DMXModule();
	~DMXModule();

	enum DMXByteOrder { BIT8, MSB, LSB };

	EnumParameter * dmxType;
	std::unique_ptr<DMXDevice> dmxDevice;
	BoolParameter * dmxConnected;
	BoolParameter * autoAdd;

	HashMap<int, IntParameter *> channelMap;

	//Script
	const Identifier dmxEventId = "dmxEvent";
	const Identifier sendDMXId = "send";

	void setCurrentDMXDevice(DMXDevice * d);

	void sendDMXValue(int channel, int value);
	void sendDMXValues(int channel, Array<int> values);
	void send16BitDMXValue(int startChannel, int value, DMXByteOrder byteOrder);
	void send16BitDMXValues(int startChannel, Array<int> values, DMXByteOrder byteOrder);


	//Script
	static var sendDMXFromScript(const var::NativeFunctionArgs& args);

	virtual void clearItem() override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;
	
	void onContainerParameterChanged(Parameter* p) override;
	void controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;

	void dmxDeviceConnected() override;
	void dmxDeviceDisconnected() override;

	void dmxDataInChanged(int channel, int value) override;

	static void showMenuAndCreateValue(ControllableContainer * container);

	class DMXRouteParams :
		public RouteParams
	{
	public:
		DMXRouteParams(Module * sourceModule, Controllable * c);
		~DMXRouteParams() {}

		EnumParameter * mode16bit;
		BoolParameter * fullRange;
		IntParameter * channel;

	};

	virtual RouteParams * createRouteParamsForSourceValue(Module * sourceModule, Controllable * c, int /*index*/) override { return new DMXRouteParams(sourceModule, c); }
	virtual void handleRoutedModuleValue(Controllable * c, RouteParams * p) override;

	static DMXModule * create() { return new DMXModule(); }
	virtual String getDefaultTypeString() const override { return "DMX"; }


	class DMXModuleListener
	{
	public:
		virtual ~DMXModuleListener() {}

		virtual void dmxDeviceChanged() {}
	};

	ListenerList<DMXModuleListener> dmxModuleListeners;
	void addDMXModuleListener(DMXModuleListener* newListener) { dmxModuleListeners.add(newListener); }
	void removeDMXModuleListener(DMXModuleListener* listener) { dmxModuleListeners.remove(listener); }
};