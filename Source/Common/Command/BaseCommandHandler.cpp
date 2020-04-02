/*
  ==============================================================================

    BaseCommandHandler.cpp
    Created: 19 Jan 2017 6:42:31pm
    Author:  Ben

  ==============================================================================
*/

#include "BaseCommandHandler.h"
#include "CommandFactory.h"
#include "ui/BaseCommandHandlerEditor.h"

BaseCommandHandler::BaseCommandHandler(const String & name, CommandContext _context, Module * _lockedModule) :
	BaseItem(name),
	context(_context),
	lockedModule(_lockedModule),
	handlerNotifier(5)
{
	trigger = addTrigger("Trigger", "Trigger this consequence");
	trigger->hideInEditor = true;

}

BaseCommandHandler::~BaseCommandHandler()
{
	clearItem();
}

void BaseCommandHandler::clearItem()
{
	BaseItem::clearItem();
    if (ModuleManager::getInstanceWithoutCreating() != nullptr) ModuleManager::getInstance()->removeBaseManagerListener(this);
    if(command != nullptr && command->module != nullptr && command->module->templateManager != nullptr && !command->module->isClearing) command->module->templateManager->removeBaseManagerListener(this);
    setCommand(nullptr);
}


void BaseCommandHandler::triggerCommand()
{
	if (command != nullptr) command->trigger();
}

void BaseCommandHandler::setCommand(CommandDefinition * commandDef)
{
	if (!commandDefinition.wasObjectDeleted() && commandDefinition == commandDef) return;

	var prevCommandData;
	if (command != nullptr)
	{
		removeChildControllableContainer(command.get());
		prevCommandData = command->getJSONData();

		command->removeCommandListener(this);
		command->module->removeInspectableListener(this);

		unregisterLinkedInspectable(command->module);
	}


	commandDefinition = commandDef;
	if (commandDef != nullptr) command.reset(commandDef->create(context));
	else command.reset();

	if (command != nullptr)
	{
		addChildControllableContainer(command.get());

		
		/*if(!prevCommandData.isVoid()) command->loadPreviousCommandData(prevCommandData); //keep as much as similar parameter possible
		else */ 
		if (!ghostCommandData.isVoid()) command->loadJSONData(ghostCommandData);
		//else if (!isCurrentlyLoadingData) setNiceName(commandDef->commandType);

		ghostModuleName = command->module->shortName;
		ghostCommandMenuPath = commandDef->menuPath;
		ghostCommandName = commandDef->commandType;
		ghostCommandData = var();

		command->addCommandListener(this);
		command->module->addInspectableListener(this);
		command->module->templateManager->removeBaseManagerListener(this);

		if (!Engine::mainEngine->isClearing) clearWarning();

		registerLinkedInspectable(command->module);


		if (ModuleManager::getInstanceWithoutCreating() != nullptr) ModuleManager::getInstance()->removeBaseManagerListener(this);
	}
	else
	{
		if (!Engine::mainEngine->isClearing)
		{
			setWarningMessage("Command not found : " + ghostModuleName + ":" + ghostCommandName);
		}
	}


	commandHandlerListeners.call(&CommandHandlerListener::commandChanged, this);
	handlerNotifier.addMessage(new CommandHandlerEvent(CommandHandlerEvent::COMMAND_CHANGED, this));
}


var BaseCommandHandler::getJSONData()
{
	var data = BaseItem::getJSONData();
	if (command != nullptr && !commandDefinition.wasObjectDeleted())
	{
		if(command->module != nullptr) data.getDynamicObject()->setProperty("commandModule", command->module->shortName);
		data.getDynamicObject()->setProperty("commandPath", commandDefinition->menuPath);
		data.getDynamicObject()->setProperty("commandType", commandDefinition->commandType);
		data.getDynamicObject()->setProperty("command", command->getJSONData());
	}
	else if (!ghostCommandData.isVoid())
	{
		data.getDynamicObject()->setProperty("ghostCommandData", ghostCommandData);
		data.getDynamicObject()->setProperty("ghostModuleName", ghostModuleName);
		data.getDynamicObject()->setProperty("ghostCommandMenuPath", ghostCommandMenuPath);
		data.getDynamicObject()->setProperty("ghostCommandName", ghostCommandName);
	}

	return data;
}

void BaseCommandHandler::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	
	ghostCommandData = data.getProperty("ghostCommandData", var());
	ghostModuleName = data.getProperty("ghostModuleName", "");
	ghostCommandMenuPath = data.getProperty("ghostCommandMenuPath", "");
	ghostCommandName = data.getProperty("ghostCommandName", "");
	
	if (data.getDynamicObject()->hasProperty("commandModule"))
	{
		Module * m = ModuleManager::getInstance()->getModuleWithName(data.getProperty("commandModule",""));
		if (m != nullptr)
		{
			String menuPath = data.getProperty("commandPath", "");
			String commandType = data.getProperty("commandType", "");
			setCommand(m->getCommandDefinitionFor(menuPath, commandType));
			if (command != nullptr)
			{
				command->loadJSONData(data.getProperty("command", var()));
			}
		} else
		{
			DBG("Output not found : " << data.getProperty("commandModule", "").toString());
		}
	}

	if (command == nullptr && ghostModuleName.isNotEmpty() && ModuleManager::getInstanceWithoutCreating() != nullptr)
	{
		ModuleManager::getInstance()->addBaseManagerListener(this);
	}

	if (command == nullptr && ghostCommandName.isNotEmpty())
	{
		setWarningMessage("Command not found : " + ghostModuleName + ":" + ghostCommandName);
	}
	else
	{
		clearWarning();
	}
}

void BaseCommandHandler::onContainerTriggerTriggered(Trigger * t)
{
	if (t == trigger)
	{
		triggerCommand();
	}
}

void BaseCommandHandler::commandContentChanged()
{
	commandHandlerListeners.call(&CommandHandlerListener::commandUpdated, this);
	handlerNotifier.addMessage(new CommandHandlerEvent(CommandHandlerEvent::COMMAND_UPDATED, this));
}

void BaseCommandHandler::commandTemplateDestroyed()
{
	if (command != nullptr)
	{
		ghostCommandData = command->getJSONData();
		//DBG("Template destroyed, command data = "+JSON::toString(ghostCommandData));
		if(command->module != nullptr && command->module->templateManager != nullptr) command->module->templateManager->addBaseManagerListener(this);
		if (!Engine::mainEngine->isClearing && ModuleManager::getInstanceWithoutCreating() != nullptr) ModuleManager::getInstance()->addBaseManagerListener(this);
	}
	setCommand(nullptr);
}

void BaseCommandHandler::inspectableDestroyed(Inspectable *)
{
    if(isClearing) return;
	if (command != nullptr) ghostCommandData = command->getJSONData();
	setCommand(nullptr);
	if(!Engine::mainEngine->isClearing && ModuleManager::getInstanceWithoutCreating() != nullptr) ModuleManager::getInstance()->addBaseManagerListener(this);
}

void BaseCommandHandler::itemAdded(Module * m)
{
	if (command == nullptr && m->shortName == ghostModuleName)
	{
		setCommand(m->getCommandDefinitionFor(ghostCommandMenuPath, ghostCommandName));
	}
}

void BaseCommandHandler::itemAdded(CommandTemplate * t)
{
	if (command == nullptr && ghostCommandMenuPath == "Templates" && t->niceName == ghostCommandName)
	{
		Module * m = ModuleManager::getInstance()->getItemWithName(ghostModuleName);
		if(m != nullptr) setCommand(m->getCommandDefinitionFor(ghostCommandMenuPath, ghostCommandName));
	}
}

InspectableEditor * BaseCommandHandler::getEditor(bool isRoot)
{
	return new BaseCommandHandlerEditor(this,isRoot);
}
