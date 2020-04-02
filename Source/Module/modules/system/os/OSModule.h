/*
  ==============================================================================

    OSModule.h
    Created: 5 Jan 2018 3:41:34pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Module/Module.h"

class OSModule :
	public Module,
	public Timer
{
public:
	OSModule();
	~OSModule();
	
	enum OSType { OS_WIN, OS_MAC, OS_LINUX };

	EnumParameter * osType;
	StringParameter * osName;
	StringParameter* ips;

	//Script
	const Identifier launchAppId = "launchApp";

	void updateIps();

	bool launchFile(File f, String args = "");
	static var launchFileFromScript(const var::NativeFunctionArgs& args);

	void timerCallback() override;

	virtual String getDefaultTypeString() const override { return "OS"; }
	static OSModule * create() { return new OSModule(); }

};