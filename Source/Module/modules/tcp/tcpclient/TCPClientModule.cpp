/*
  ==============================================================================

    TCPClientModule.cpp
    Created: 21 Oct 2017 5:04:54pm
    Author:  Ben

  ==============================================================================
*/

#include "TCPClientModule.h"

TCPClientModule::TCPClientModule(const String & name, int defaultRemotePort) :
	NetworkStreamingModule(name,false, true, 0, defaultRemotePort)
{
	connectionFeedbackRef = senderIsConnected;

	setupIOConfiguration(true, true);

	if (!Engine::mainEngine->isLoadingFile)
	{
		setupSender();
	}

	startTimerHz(1);
}

TCPClientModule::~TCPClientModule()
{
}

void TCPClientModule::setupSender()
{
	clearThread();
	clearInternal();
	
	if (!enabled->boolValue()) return;
	if (isCurrentlyLoadingData) return;
	if (sendCC == nullptr) return;
	if (!sendCC->enabled->boolValue())
	{
		sendCC->clearWarning();
		return;
	}

	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;

	startThread();
}

void TCPClientModule::initThread()
{
	//if (sender.isConnected()) sender.close();

	if (!enabled->boolValue()) return;

	String targetHost = useLocal->boolValue() ? "127.0.0.1" : remoteHost->stringValue();
	bool result = sender.connect(targetHost, remotePort->intValue(), 200);
	if(result) NLOG(niceName, "Sender bound to port " << sender.getBoundPort());
	senderIsConnected->setValue(result);

	if (result)
	{
		NLOG(niceName, "Client is connected to " << remoteHost->stringValue() << ":" << remotePort->intValue());
		sendCC->clearWarning();
		startThread();
	}
	else
	{
		String s = "Could not connect to " + remoteHost->stringValue() + ":" + remotePort->stringValue();
		if(getWarningMessage().isEmpty()) NLOGERROR(niceName, s);
		sendCC->setWarningMessage(s);
	}

}

void TCPClientModule::clearThread()
{
	NetworkStreamingModule::clearThread();
	if (sender.isConnected()) sender.close();
}

bool TCPClientModule::checkReceiverIsReady()
{
	if (!senderIsConnected->boolValue()) return false;
	int result = sender.waitUntilReady(true, 100);
	
	if(result == -1)
	{
		senderIsConnected->setValue(false);
		return false;
	}

	return result == 1;
}

bool TCPClientModule::isReadyToSend()
{
	return senderIsConnected->boolValue();
}

void TCPClientModule::sendMessageInternal(const String & message, var)
{
	int numBytes = sender.write(message.getCharPointer(), message.length());
	if (numBytes == -1)
	{
		NLOGERROR(niceName, "Error sending message");
		senderIsConnected->setValue(false);
	}
}

void TCPClientModule::sendBytesInternal(Array<uint8> data, var)
{
	int numBytes = sender.write(data.getRawDataPointer(), data.size());
	if (numBytes == -1)
	{
		NLOGERROR(niceName, "Error sending data");
		senderIsConnected->setValue(false);
	}
}

Array<uint8> TCPClientModule::readBytes()
{
	uint8 bytes[2048];
	int numRead = sender.read(bytes, 2048, false);

	if (numRead == 0)
	{
		NLOGWARNING(niceName, "Connection to TCP Server seems lost, disconnecting");
		senderIsConnected->setValue(false);
	}

	return Array<uint8>(bytes, numRead);
}

void TCPClientModule::clearInternal()
{
	if (sender.isConnected()) sender.close();
}


void TCPClientModule::timerCallback()
{
	if (!sender.isConnected()) senderIsConnected->setValue(false);
	if(!senderIsConnected->boolValue()) setupSender();
}
