/*
  ==============================================================================

	OSCModule.cpp
	Created: 29 Oct 2016 7:07:07pm
	Author:  bkupe

  ==============================================================================
*/

#include "OSCModule.h"
#include "Module/modules/common//ui/EnablingNetworkControllableContainerEditor.h"
#include "UI/ChataigneAssetManager.h"
#include "ui/OSCOutputEditor.h"
#include "custom/commands/CustomOSCCommand.h"
#include "Module/ModuleManager.h"

OSCModule::OSCModule(const String & name, int defaultLocalPort, int defaultRemotePort, bool canHaveInput, bool canHaveOutput) :
	Module(name),
	Thread("OSCZeroconf"),
	localPort(nullptr),
    servus("_osc._udp"),
	receiveCC(nullptr)
{
	
	setupIOConfiguration(canHaveInput, canHaveOutput);
	canHandleRouteValues = canHaveOutput;

	
	//Receive
	if (canHaveInput)
	{
		receiveCC.reset(new EnablingControllableContainer("OSC Input"));
		receiveCC->customGetEditorFunc = &EnablingNetworkControllableContainerEditor::create;
		moduleParams.addChildControllableContainer(receiveCC.get());

		localPort = receiveCC->addIntParameter("Local Port", "Local Port to bind to receive OSC Messages", defaultLocalPort, 1024, 65535);
		localPort->hideInOutliner = true;
		localPort->warningResolveInspectable = this;

		receiver.registerFormatErrorHandler(&OSCHelpers::logOSCFormatError);
		receiver.addListener(this);

		if(!Engine::mainEngine->isLoadingFile) setupReceiver();

		thruManager.reset(new ControllableContainer("Pass-through"));
		thruManager->userCanAddControllables = true;
		thruManager->customUserCreateControllableFunc = &OSCModule::createThruControllable;
	} else
	{
		if (receiveCC != nullptr) moduleParams.removeChildControllableContainer(receiveCC.get());
		receiveCC = nullptr;
	}

	//Send
	if (canHaveOutput)
	{
		outputManager.reset(new BaseManager<OSCOutput>("OSC Outputs"));
		outputManager->addBaseManagerListener(this);

		moduleParams.addChildControllableContainer(outputManager.get());

		outputManager->setCanBeDisabled(true);
		if (!Engine::mainEngine->isLoadingFile)
		{
			OSCOutput * o = outputManager->addItem(nullptr, var(), false);
			o->remotePort->setValue(defaultRemotePort);
			if (!Engine::mainEngine->isLoadingFile) setupSenders();
		}
	} else
	{
		if (outputManager != nullptr) removeChildControllableContainer(outputManager.get());
		outputManager = nullptr;
	}

	if (thruManager != nullptr)
	{
		moduleParams.addChildControllableContainer(thruManager.get());
	}

	//Script
	scriptObject.setMethod("send", OSCModule::sendOSCFromScript);
	scriptObject.setMethod("sendTo", OSCModule::sendOSCToFromScript);

	scriptManager->scriptTemplate += ChataigneAssetManager::getInstance()->getScriptTemplate("osc");

	defManager->add(CommandDefinition::createDef(this, "", "Custom Message", &CustomOSCCommand::create));

	genericSender.connect("0.0.0.0", 1);
}

OSCModule::~OSCModule()
{
	if (isThreadRunning())
	{
		signalThreadShouldExit();
		waitForThreadToExit(1000);
		stopThread(100);
	}
}

void OSCModule::setupReceiver()
{
	receiver.disconnect();
	if (receiveCC == nullptr) return;

	if (!receiveCC->enabled->boolValue())
	{
		localPort->clearWarning();
		return;
	}

	//DBG("Local port set to : " << localPort->intValue());
	bool result = receiver.connect(localPort->intValue());

	if (result)
	{
		NLOG(niceName, "Now receiving on port : " + localPort->stringValue());
		if(!isThreadRunning() && !Engine::mainEngine->isLoadingFile) startThread();

		Array<IPAddress> ad;

		IPAddress::findAllAddresses(ad);
		Array<String> ips;
		for (auto &a : ad) ips.add(a.toString());
		ips.sort();
		String s = "Local IPs:";
		for (auto &ip : ips) s += String("\n > ") + ip;

		NLOG(niceName, s);
		localPort->clearWarning();
	} else
	{
		NLOGERROR(niceName, "Error binding port " + localPort->stringValue());
		localPort->setWarningMessage("Error binding port " + localPort->stringValue());
	}
	
}

float OSCModule::getFloatArg(OSCArgument a)
{
	if (a.isFloat32()) return a.getFloat32();
	if (a.isInt32()) return (float)a.getInt32();
	if (a.isString()) return a.getString().getFloatValue();
	if (a.isColour()) return getIntArg(a);
	return 0;
}

int OSCModule::getIntArg(OSCArgument a)
{
	if (a.isInt32()) return a.getInt32();
	if (a.isFloat32()) return roundf(a.getFloat32());
	if (a.isString()) return a.getString().getIntValue();
	if (a.isColour()) return a.getColour().toInt32();
		//return c.red << 24 | c.green << 16 | c.blue << 8 | c.alpha;
	//}

	return 0;
}

String OSCModule::getStringArg(OSCArgument a)
{
	if (a.isString()) return a.getString();
	if (a.isInt32()) return String(a.getInt32());
	if (a.isFloat32()) return String(a.getFloat32());
	if (a.isColour()) return OSCHelpers::getColourFromOSC(a.getColour()).toString();
	return "";
}

Colour OSCModule::getColorArg(OSCArgument a)
{
	if (a.isColour()) return OSCHelpers::getColourFromOSC(a.getColour());
	if (a.isString()) return Colour::fromString(a.getString());
	if (a.isInt32()) return Colour(a.getInt32());
	if (a.isFloat32()) return Colour(a.getFloat32());
	return Colours::black;
}

void OSCModule::processMessage(const OSCMessage & msg)
{
	if (logIncomingData->boolValue())
	{
		String s = "";
		for (auto &a : msg) s += String(" ") + getStringArg(a);
		NLOG(niceName, msg.getAddressPattern().toString() << " :" << s);
	}

	inActivityTrigger->trigger();

	if (thruManager != nullptr)
	{
		for (auto& c : thruManager->controllables)
		{
			if (TargetParameter* mt = (TargetParameter*)c)
			{
				if(!mt->enabled) continue;
				if (OSCModule* m = (OSCModule*)(mt->targetContainer.get()))
				{
					m->sendOSC(msg);
				}
			}
		}
	}


	processMessageInternal(msg);

	if (scriptManager->items.size() > 0)
	{
		Array<var> params;
		params.add(msg.getAddressPattern().toString());
		var args = var(Array<var>()); //initialize force array
		for (auto &a : msg) args.append(OSCModule::argumentToVar(a));
		params.add(args);
		scriptManager->callFunctionOnAllItems(oscEventId, params);
	}

}

void OSCModule::setupModuleFromJSONData(var data)
{
	Module::setupModuleFromJSONData(data);

	if (receiveCC != nullptr)
	{
		receiveCC->enabled->setValue(hasInput);
		receiveCC->hideInEditor = !hasInput;
	}
	if (outputManager != nullptr)
	{
		outputManager->enabled->setValue(hasOutput);
		outputManager->hideInEditor = !hasOutput;
	}
}

void OSCModule::itemAdded(OSCOutput* output)
{
	output->warningResolveInspectable = this;
}

void OSCModule::setupSenders()
{
	for (auto &o : outputManager->items) o->setupSender();
}

void OSCModule::sendOSC(const OSCMessage & msg, String ip, int port)
{
	if (isClearing || outputManager == nullptr) return;
	if (!enabled->boolValue()) return;

	if (!outputManager->enabled->boolValue()) return;

	if (logOutgoingData->boolValue())
	{
		NLOG(niceName, "Send OSC : " << msg.getAddressPattern().toString());
		for (auto &a : msg)
		{
			LOG(getStringArg(a));
		}
	}

	outActivityTrigger->trigger();

	if (ip.isNotEmpty() && port > 0)
	{
		genericSender.sendToIPAddress(ip, port, msg);
	}
	else
	{
		for (auto& o : outputManager->items) o->sendOSC(msg);
	}
}

void OSCModule::setupZeroConf()
{
	if (Engine::mainEngine->isClearing || localPort == nullptr) return;

	String nameToAdvertise;
	int portToAdvertise = 0;
	while (nameToAdvertise != niceName || portToAdvertise != localPort->intValue())
	{
		nameToAdvertise = niceName;
		portToAdvertise = localPort->intValue();

		servus.withdraw();
		
		if (!hasInput) return;

		//DBG("ADVERTISE");
		servus.announce(portToAdvertise, ("Chataigne - " + nameToAdvertise).toStdString());
		
		if (nameToAdvertise != niceName || localPort->intValue() != portToAdvertise || !hasInput)
		{
			//DBG("Name or port changed during advertise, readvertising");
		}
	}
	
	NLOG(niceName,"Zeroconf service created : " << nameToAdvertise << ":" << portToAdvertise);
}

var OSCModule::sendOSCFromScript(const var::NativeFunctionArgs & a)
{
	OSCModule * m = getObjectFromJS<OSCModule>(a);
	if (!m->enabled->boolValue()) return var();

	if (!checkNumArgs(m->niceName, a, 1)) return var();

	try
	{
		OSCMessage msg(a.arguments[0].toString());

		for (int i = 1; i < a.numArguments; i++)
		{
			if (a.arguments[i].isArray())
			{
				Array<var> * arr = a.arguments[i].getArray();
				for (auto &aa : *arr) msg.addArgument(varToArgument(aa));
			}
			else
			{
				msg.addArgument(varToArgument(a.arguments[i]));
			}
		}

		m->sendOSC(msg);
	}
	catch (OSCFormatError &e)
	{
		NLOGERROR(m->niceName, "Error sending message : " << e.description);
	}
	

	return var();
}

var OSCModule::sendOSCToFromScript(const var::NativeFunctionArgs& a)
{
	OSCModule* m = getObjectFromJS<OSCModule>(a);
	if (!m->enabled->boolValue()) return var();
	if (!checkNumArgs(m->niceName, a, 3)) return var();

	try
	{
		OSCMessage msg(a.arguments[2].toString());

		for (int i = 3; i < a.numArguments; i++)
		{
			if (a.arguments[i].isArray())
			{
				Array<var>* arr = a.arguments[i].getArray();
				for (auto& aa : *arr) msg.addArgument(varToArgument(aa));
			}
			else
			{
				msg.addArgument(varToArgument(a.arguments[i]));
			}
		}

		m->sendOSC(msg, a.arguments[0], a.arguments[1]);
	}
	catch (OSCFormatError & e)
	{
		NLOGERROR(m->niceName, "Error sending message : " << e.description);
	}


	return var();
}

void OSCModule::createThruControllable(ControllableContainer* cc)
{
	TargetParameter* p = new TargetParameter("Output module", "Target module to send the raw data to","");
	p->targetType = TargetParameter::CONTAINER;
	p->customGetTargetContainerFunc = &ModuleManager::showAndGetModuleOfType<OSCModule>;
	p->isRemovableByUser = true;
	p->canBeDisabledByUser = true;
	p->saveValueOnly = false;
	cc->addParameter(p);
}


OSCArgument OSCModule::varToArgument(const var & v)
{
	if (v.isBool()) return OSCArgument(((bool)v) ? 1 : 0);
	else if (v.isInt()) return OSCArgument((int)v);
	else if (v.isInt64()) return OSCArgument((int)v);
	else if (v.isDouble()) return OSCArgument((float)v);
	else if (v.isString()) return OSCArgument(v.toString());
	jassert(false);
	return OSCArgument("error");
}

OSCArgument OSCModule::varToColorArgument(const var & v)
{
	if (v.isBool()) return OSCArgument(OSCHelpers::getOSCColour((bool)v ? Colours::white : Colours::black));
	else if (v.isInt() || v.isInt64() || v.isDouble())
	{
		int iv = (int)v;
		OSCColour c = OSCColour::fromInt32(iv);// >> 24 & 0xFF | iv >> 16 & 0xFF | iv >> 8 & 0xFF | iv & 0xFF);
		return OSCArgument(c);
	}else if (v.isString())
	{
		Colour c = Colour::fromString(v.toString());
		return OSCArgument(OSCHelpers::getOSCColour(c));
	}
	else if (v.isArray() && v.size() >= 3)
	{
		Colour c = Colour::fromFloatRGBA(v[0], v[1], v[2], v.size() >= 4 ? (float)v[3] : 1.0f);
		return OSCArgument(OSCHelpers::getOSCColour(c));
	}

	jassert(false);
	return OSCArgument("error");
}

var OSCModule::argumentToVar(const OSCArgument & a)
{
	if (a.isFloat32()) return var(a.getFloat32());
	else if (a.isInt32()) return var(a.getInt32());
	else if (a.isString()) return var(a.getString());
	else if (a.isColour())
	{
		OSCColour c = a.getColour();
		return var((int)c.toInt32());
	}
	
	return var("error");
}

var OSCModule::getJSONData()
{
	var data = Module::getJSONData();
	/*if (receiveCC != nullptr)
	{
		var inputData = receiveCC->getJSONData();
		if (!inputData.isVoid()) data.getDynamicObject()->setProperty("input", inputData);
	}

	if (outputManager != nullptr)
	{
		var outputsData = outputManager->getJSONData();
		if (!outputsData.isVoid()) data.getDynamicObject()->setProperty("outputs", outputsData);
	}

	if (thruManager != nullptr)
	{
		var thruData = thruManager->getJSONData();
		if (!thruData.isVoid()) data.getDynamicObject()->setProperty("thru", thruData);
	}
	*/
	return data;
}

void OSCModule::loadJSONDataInternal(var data)
{
	Module::loadJSONDataInternal(data);
	//if (receiveCC != nullptr) receiveCC->loadJSONData(data.getProperty("input", var()));
	if (outputManager != nullptr)
	{
		//outputManager->loadJSONData(data.getProperty("outputs", var()));
		setupSenders();
	}

	if (thruManager != nullptr)
	{
		//thruManager->loadJSONData(data.getProperty("thru", var()));
		for (auto& c : thruManager->controllables)
		{
			if (TargetParameter* mt = dynamic_cast<TargetParameter *>(c))
			{
				mt->targetType = TargetParameter::CONTAINER;
				mt->customGetTargetContainerFunc = &ModuleManager::showAndGetModuleOfType<OSCModule>;
				mt->isRemovableByUser = true;
				mt->canBeDisabledByUser = true;
			}
		}
	}

	if(!isThreadRunning()) startThread();

	setupReceiver();
}


void OSCModule::handleRoutedModuleValue(Controllable * c, RouteParams * p)
{
	if (OSCRouteParams* op = dynamic_cast<OSCRouteParams*>(p))
	{
		try
		{

			OSCMessage m(op->address->stringValue());

			if (c->type != Controllable::TRIGGER)
			{
				var v = dynamic_cast<Parameter*>(c)->getValue();

				if (c->type == Parameter::COLOR)
				{
					Colour col = ((ColorParameter*)p)->getColor();
					m.addColour(OSCHelpers::getOSCColour(col));
				}
				else
				{
					if (!v.isArray())  m.addArgument(varToArgument(v));
					else
					{
						for (int i = 0; i < v.size(); i++) m.addArgument(varToArgument(v[i]));
					}
				}

			}

			sendOSC(m);
		}
		catch (const OSCFormatError&)
		{
			NLOG(niceName, "Address is invalid : " << op->address->stringValue());
		}
	}

}

void OSCModule::onContainerParameterChangedInternal(Parameter * p)
{
	Module::onContainerParameterChangedInternal(p);	
}

void OSCModule::onContainerNiceNameChanged()
{
	Module::onContainerNiceNameChanged();
	if (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing) return;
	if(!isThreadRunning()) startThread();
}

void OSCModule::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	Module::onControllableFeedbackUpdateInternal(cc, c);

	if (outputManager != nullptr && c == outputManager->enabled)
	{
		bool rv = receiveCC != nullptr ? receiveCC->enabled->boolValue() : false;
		bool sv = outputManager->enabled->boolValue();
		for(auto &o : outputManager->items) o->setForceDisabled(!sv);
		setupIOConfiguration(rv, sv);

	}else if (receiveCC != nullptr && c == receiveCC->enabled)
	{
		bool rv = receiveCC->enabled->boolValue();
		bool sv = outputManager != nullptr ? outputManager->enabled->boolValue() : false;
		setupIOConfiguration(rv,sv);
		localPort->setEnabled(rv);
		if(!isCurrentlyLoadingData) setupReceiver();

	}
	else if (c == localPort)
	{
		if (!isCurrentlyLoadingData) setupReceiver();
	}
}

void OSCModule::oscMessageReceived(const OSCMessage & message)
{
	if (!enabled->boolValue()) return;
	processMessage(message);
}

void OSCModule::oscBundleReceived(const OSCBundle & bundle)
{
	if (!enabled->boolValue()) return;
	for (auto &m : bundle)
	{
		processMessage(m.getMessage());
	}
}

void OSCModule::run()
{
	setupZeroConf();
}

OSCModule::OSCRouteParams::OSCRouteParams(Module * sourceModule, Controllable * c) 
{
	bool sourceIsGenericOSC = sourceModule->getTypeString() == "OSC";

	String tAddress;

	if (!sourceIsGenericOSC)
	{
		tAddress = c->shortName;

		ControllableContainer * cc = c->parentContainer;
		while (cc != nullptr)
		{
			if (cc->shortName != "values")
			{
				tAddress = cc->shortName + "/" + tAddress;
			}
			Module * m = dynamic_cast<Module *>(cc);
			if (m != nullptr) break;

			cc = cc->parentContainer;
		}
	} else
	{
		tAddress = c->niceName; //on CustomOSCModule, niceName is the actual address
	}

	if (!tAddress.startsWithChar('/')) tAddress = "/" + tAddress;
	
	address = addStringParameter("Address", "Route Address", tAddress);
}



///// OSC OUTPUT

OSCOutput::OSCOutput() :
	 BaseItem("OSC Output"),
	forceDisabled(false),
	senderIsConnected(false)
{
	isSelectable = false;

	useLocal = addBoolParameter("Local", "Send to Local IP (127.0.0.1). Allow to quickly switch between local and remote IP.", true);
	remoteHost = addStringParameter("Remote Host", "Remote Host to send to.", "127.0.0.1");
	remoteHost->autoTrim = true;
	remoteHost->setEnabled(!useLocal->boolValue());
	remotePort = addIntParameter("Remote port", "Port on which the remote host is listening to", 9000, 1024, 65535);

	if (!Engine::mainEngine->isLoadingFile) setupSender();
}

OSCOutput::~OSCOutput()
{
}

void OSCOutput::setForceDisabled(bool value)
{
	if (forceDisabled == value) return;
	forceDisabled = value;
	setupSender();
}

void OSCOutput::onContainerParameterChangedInternal(Parameter * p)
{

	if (p == remoteHost || p == remotePort || p == useLocal)
	{
		if(!Engine::mainEngine->isLoadingFile) setupSender();
		if (p == useLocal) remoteHost->setEnabled(!useLocal->boolValue());
	}
}

InspectableEditor * OSCOutput::getEditor(bool isRoot)
{
	return new OSCOutputEditor(this, isRoot);
}

void OSCOutput::setupSender()
{
	sender.disconnect();
	if (!enabled->boolValue() || forceDisabled || Engine::mainEngine->isClearing) return;

	String targetHost = useLocal->boolValue() ? "127.0.0.1" : remoteHost->stringValue();
	senderIsConnected = sender.connect(targetHost, remotePort->intValue());
	if (senderIsConnected)
	{ 
		NLOG(niceName, "Now sending to " + remoteHost->stringValue() + ":" + remotePort->stringValue());
		clearWarning();
	} else
	{
		NLOGWARNING(niceName, "Could not connect to " << remoteHost->stringValue() << ":" + remotePort->stringValue());
		setWarningMessage("Could not connect to " + remoteHost->stringValue() + ":" + remotePort->stringValue());
	}
	
}

void OSCOutput::sendOSC(const OSCMessage & m)
{
	if (!enabled->boolValue() || forceDisabled || !senderIsConnected) return;
	sender.send(m);
}
