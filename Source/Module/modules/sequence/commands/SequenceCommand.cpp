/*
  ==============================================================================

	SequenceCommand.cpp
	Created: 20 Feb 2017 2:12:09pm
	Author:  Ben

  ==============================================================================
*/

#include "../SequenceModule.h"
#include "SequenceCommand.h"
#include "TimeMachine/ChataigneSequenceManager.h"

SequenceCommand::SequenceCommand(SequenceModule* _module, CommandContext context, var params) :
	BaseCommand(_module, context, params),
	sequenceModule(_module),
	target(nullptr)
{
	actionType = (ActionType)(int)params.getProperty("type", PLAY_SEQUENCE);

	if (actionType != STOP_ALL_SEQUENCES)
	{
		target = addTargetParameter("Target", "Target for the command");
		target->targetType = TargetParameter::CONTAINER;
	}

	switch (actionType)
	{
	case PLAY_SEQUENCE:
	case TOGGLE_SEQUENCE:
		playFromStart = addBoolParameter("Play From Start", "If enabled, when the command is triggered, will position the time at 0 before playing", false);
	case STOP_SEQUENCE:
	case PAUSE_SEQUENCE:
		target->showParentNameInEditor = false;
		target->customGetTargetContainerFunc = &ChataigneSequenceManager::showMenuAndGetSequenceStatic;
		break;


	case DISABLE_LAYER:
	case ENABLE_LAYER:
	case TOGGLE_LAYER:
		target->customGetTargetContainerFunc = &ChataigneSequenceManager::showmMenuAndGetLayerStatic;
		break;

	case SET_TIME:
	case MOVE_TIME:
		target->customGetTargetContainerFunc = &ChataigneSequenceManager::showMenuAndGetSequenceStatic;

		value = addFloatParameter("Time", "Target time to set", 0, actionType == MOVE_TIME ? -3600 : 0, 3600);
		value->defaultUI = FloatParameter::TIME;
		addTargetMappingParameterAt(value, 0);

		playFromStart = addBoolParameter("Play", "If enabled, will force playing the sequence after setting the time", false);
		break;

	case GOTO_CUE:
		target->customGetTargetContainerFunc = &ChataigneSequenceManager::showMenuAndGetCueStatic;
		playFromStart = addBoolParameter("Play", "If enabled, will force playing the sequence after setting the time to the cue", false);
		break;

	default:
		break;
	}
}

SequenceCommand::~SequenceCommand()
{
}

void SequenceCommand::triggerInternal()
{
	BaseCommand::triggerInternal();

	if (actionType != STOP_ALL_SEQUENCES)
	{
		if (target->targetContainer == nullptr) return;
		if (target->targetContainer.wasObjectDeleted()) return;
	}
	
	switch (actionType)
	{
	case PLAY_SEQUENCE:
		if (playFromStart->boolValue()) ((Sequence*)target->targetContainer.get())->stopTrigger->trigger();
		((Sequence*)target->targetContainer.get())->playTrigger->trigger();
		break;

	case PAUSE_SEQUENCE:
		((Sequence*)target->targetContainer.get())->pauseTrigger->trigger();
		break;

	case STOP_SEQUENCE:
		((Sequence*)target->targetContainer.get())->stopTrigger->trigger();
		break;

	case STOP_ALL_SEQUENCES:
		ChataigneSequenceManager::getInstance()->stopAllTrigger->trigger();
		break;

	case TOGGLE_SEQUENCE:
		if (playFromStart->boolValue() && !((Sequence*)target->targetContainer.get())->isPlaying->boolValue()) ((Sequence*)target->targetContainer.get())->setCurrentTime(0);
		((Sequence*)target->targetContainer.get())->togglePlayTrigger->trigger();
		break;

	case ENABLE_LAYER:
		((SequenceLayer*)target->targetContainer.get())->enabled->setValue(true);
		break;

	case DISABLE_LAYER:
		((SequenceLayer*)target->targetContainer.get())->enabled->setValue(false);
		break;

	case TOGGLE_LAYER:
		((SequenceLayer*)target->targetContainer.get())->enabled->setValue(!((SequenceLayer*)target->targetContainer.get())->enabled->boolValue());
		break;

	case SET_TIME:
	case MOVE_TIME:
	{
		Sequence* s = (Sequence*)target->targetContainer.get();
		float t = actionType == SET_TIME ? 0 : s->currentTime->floatValue();
		s->setCurrentTime(t + value->floatValue(), true, true);
		if (playFromStart->boolValue()) ((Sequence*)target->targetContainer.get())->playTrigger->trigger();
	}
	break;

	case GOTO_CUE:
		TimeCue* cue = dynamic_cast<TimeCue*>(target->targetContainer.get());
		if (cue != nullptr)
		{
			Sequence* s = cue->getSequence();
			s->setCurrentTime(cue->time->floatValue(), true, true);
			if (playFromStart->boolValue()) s->playTrigger->trigger();
		}
		break;
	}
}

void SequenceCommand::loadJSONDataInternal(var data)
{
	if (Engine::mainEngine->isLoadingFile)
	{
		Engine::mainEngine->addEngineListener(this);
		dataToLoad = data;
	}
	else BaseCommand::loadJSONDataInternal(data);
}

void SequenceCommand::endLoadFile()
{
	//reset data we want to reload
	if(target != nullptr) target->setValue("", true);

	loadJSONData(dataToLoad);
	dataToLoad = var();

	Engine::mainEngine->removeEngineListener(this);

}


BaseCommand* SequenceCommand::create(ControllableContainer* module, CommandContext context, var params) {
	return new SequenceCommand((SequenceModule*)module, context, params);
}

/*
InspectableEditor * SequenceCommand::getEditor(bool isRoot)
{
	return new SequenceCommandEditor(this, isRoot);
}
*/
