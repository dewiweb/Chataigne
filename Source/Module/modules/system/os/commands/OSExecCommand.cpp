/*
  ==============================================================================

    OSFileCommand.cpp
    Created: 5 Jan 2018 4:05:49pm
    Author:  Ben

  ==============================================================================
*/

#include "OSExecCommand.h"

#if JUCE_WINDOWS
#include <windows.h>
#include <Tlhelp32.h>
#endif

OSExecCommand::OSExecCommand(OSModule* _module, CommandContext context, var params) :
	BaseCommand(_module, context, params),
	launchOptions(nullptr),
	silentMode(nullptr)
{
	actionType = (ActionType)(int)params.getProperty("type", LAUNCH_APP);

	switch (actionType)
	{
	case OPEN_FILE:
	case LAUNCH_APP:
	case LAUNCH_COMMAND_FILE:
		target = new FileParameter("Target", "The target file of app for this command", "");
		addParameter(target);
		if (actionType != LAUNCH_COMMAND_FILE)
		{
			launchOptions = addStringParameter("Launch Options", "Additional options when launching the app", "");
			launchOptions->multiline = true;
		}
		break;

	case KILL_APP:
		target = addStringParameter("Target", "The process name to kill", "");
		killMode = addBoolParameter("Hard kill", "If enabled, will kill like a boss, not very gently", false);
		break;

	case LAUNCH_COMMAND:
		target = addStringParameter("Command", "The command to launch.Every line is a new command", "");
		target->multiline = true;
		break;

	}

	if (actionType == LAUNCH_COMMAND || actionType == LAUNCH_COMMAND_FILE)
	{
		silentMode = addBoolParameter("Silent mode", "If checked, will not open a new terminal window, but if the process never stops, there is no way to shut it down without closing Chataigne !", false);
	}
}

OSExecCommand::~OSExecCommand()
{
}

void OSExecCommand::triggerInternal()
{
	BaseCommand::triggerInternal();

	if (!module->enabled->boolValue()) return;

	switch (actionType)
	{
	case LAUNCH_APP:
	case OPEN_FILE:
	{
		File f = ((FileParameter*)target)->getFile();
		File wDir = File::getCurrentWorkingDirectory();

		f.getParentDirectory().setAsCurrentWorkingDirectory();
		bool result = f.startAsProcess(launchOptions->stringValue());
		if (!result) NLOGERROR(module->niceName, "Error trying to launch process : " << f.getFullPathName());
		wDir.setAsCurrentWorkingDirectory();
		module->outActivityTrigger->trigger();
	}
	break;

	case KILL_APP:
	{
		killProcess(target->stringValue());
		module->outActivityTrigger->trigger();
	}
	break;

	case LAUNCH_COMMAND:
	{
		String command = target->stringValue().replace("\n", " && ");
		int result = 0;
#if JUCE_WINDOWS
        if(silentMode->boolValue()) WinExec(command.getCharPointer(), SW_HIDE);
		else result = system(command.getCharPointer());
#else
        result = system(command.getCharPointer());
#endif
		if (module->logOutgoingData->boolValue()) NLOG(module->niceName, "Launching : " + command);
		if (result != 0) NLOGERROR(module->niceName, "Error trying to launch command : " << target->stringValue());
		module->outActivityTrigger->trigger();
	}
	break;

	case LAUNCH_COMMAND_FILE:
	{
		File f = ((FileParameter*)target)->getFile();
		String dir = f.getParentDirectory().getFullPathName();
		
#if JUCE_WINDOWS
		String driveLetter = dir.substring(0, 2);
		String command = driveLetter + " && cd \"" + dir + "\" && \"" + f.getFileName()+"\"";
		if (module->logOutgoingData->boolValue()) NLOG(module->niceName, "Launching : " + command);
		int result = 0;
		if (silentMode->boolValue()) WinExec(command.getCharPointer(), SW_HIDE);
		else system(command.getCharPointer());
#else
        String launchPrefix = f.getFileName().endsWith("sh")?"sh ":"./";
        
    #if JUCE_MAC
        
        String command = "osascript -e 'tell application \"Terminal\""
        +String("\nactivate")
        +"\ndo script \"cd "+ dir +" && "+launchPrefix + "\\\""+f.getFileName()+"\\\"\""
        +"\nend tell'";
        
            //"osascript -e 'tell application \"Terminal\" to do script \"cd "+ dir +" && "+launchPrefix + f.getFileName()+"\"'";
    #else //linux
        String command = "cd \"" + dir + "\" && gnome-terminal -- bash -c '" + launchPrefix + "\""+ f.getFileName()+"\"'";
    #endif
        
        if (module->logOutgoingData->boolValue()) NLOG(module->niceName, "Launching : " + command);
        int result = system(command.getCharPointer());
#endif
        
		if (result != 0) NLOGERROR(module->niceName, "Error trying to launch command : " << f.getFullPathName());
		module->outActivityTrigger->trigger();
	}
	break;


	}
}

void OSExecCommand::killProcess(const String & name)
{
#if JUCE_WINDOWS
	int result = system(String("taskkill " + String(killMode->boolValue() ? "/f " : "") + "/im \"" + target->stringValue() + "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + target->stringValue());
#else
	int result = system(String("killall "+ String(killMode->boolValue()?"-9":"-2") +" \""+target->stringValue()+"\"").getCharPointer());
	if(result != 0) LOGWARNING("Problem killing app " + target->stringValue());
#endif
}

