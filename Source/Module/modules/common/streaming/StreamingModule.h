/*
  ==============================================================================

    StreamingModule.h
    Created: 5 Jan 2018 10:39:38am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Module/Module.h"

class StreamingModule :
	public Module
{
public:
	StreamingModule(const String &name = "Streaming");
	virtual ~StreamingModule();

	enum StreamingType { LINES, DATA255, RAW, COBS };
	enum MessageStructure { LINES_SPACE, LINES_TAB, LINES_COMMA, LINES_COLON, LINES_SEMICOLON, LINES_EQUALS, NO_SEPARATION, RAW_1BYTE, RAW_FLOATS, RAW_COLORS};
	EnumParameter * streamingType;

	BoolParameter * autoAdd;
	EnumParameter * messageStructure;
	BoolParameter * firstValueIsTheName;

	std::unique_ptr<ControllableContainer> thruManager;

	const Identifier dataEventId = "dataReceived";
	const Identifier sendId = "send";
	const Identifier sendBytesId = "sendBytes";

	virtual void setAutoAddAvailable(bool value);

	virtual void buildMessageStructureOptions();

	virtual void processDataLine(const String &message);
	virtual void processDataLineInternal(const String &message) {}
	virtual void processDataBytes(Array<uint8> data);
	virtual void processDataBytesInternal(Array<uint8> data) {}

	virtual void sendMessage(const String &message, var params = var());
	virtual void sendMessageInternal(const String &message, var params) {}
	virtual void sendBytes(Array<uint8> bytes, var params = var());
	virtual void sendBytesInternal(Array<uint8> bytes, var params) {}

	static void showMenuAndCreateValue(ControllableContainer * container);
	static void createThruControllable(ControllableContainer* cc);

	virtual void onControllableFeedbackUpdateInternal(ControllableContainer *, Controllable * c) override;

	virtual bool isReadyToSend() { return false; }

	class StreamingRouteParams :
		public RouteParams
	{
	public:
		StreamingRouteParams(Module* sourceModule, Controllable* c);
		~StreamingRouteParams() {}
		
		StringParameter* prefix;

		BoolParameter* appendNL;
		BoolParameter* appendCR;
	};

	virtual RouteParams* createRouteParamsForSourceValue(Module* sourceModule, Controllable* c, int /*index*/) override { return new StreamingRouteParams(sourceModule, c); }
	virtual void handleRoutedModuleValue(Controllable* c, RouteParams* p) override;

	void loadJSONDataInternal(var data) override;

	static var sendStringFromScript(const var::NativeFunctionArgs &a);
	static var sendBytesFromScript(const var::NativeFunctionArgs &a);

	virtual String getDefaultTypeString() const override { return "Streaming"; }
};