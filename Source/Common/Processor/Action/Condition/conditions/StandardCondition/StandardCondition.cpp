/*
  ==============================================================================

	StandardCondition.cpp
	Created: 21 Feb 2017 11:37:26am
	Author:  Ben

  ==============================================================================
*/

#include "StandardCondition.h"
#include "../../Comparator/ComparatorFactory.h"
#include "ui/StandardConditionEditor.h"
#include "Module/ModuleManager.h"

StandardCondition::StandardCondition(var params) :
	Condition(StandardCondition::getTypeStringStatic(),params)
{
	sourceTarget = addTargetParameter("Input Value", "Element that will be the source to check if condition is active or not"); 
	sourceTarget->customGetTargetFunc = &ModuleManager::showAllValuesAndGetControllable;
	sourceTarget->customGetControllableLabelFunc = &Module::getTargetLabelForValueControllable;
	sourceTarget->customCheckAssignOnNextChangeFunc = &ModuleManager::checkControllableIsAValue;
	sourceTarget->hideInEditor = true;
}

StandardCondition::~StandardCondition()
{
	setSourceControllable(nullptr);
}


var StandardCondition::getJSONData()
{
	var data = Condition::getJSONData();
	if (comparator != nullptr) data.getDynamicObject()->setProperty("comparator", comparator->getJSONData());
	return data;
}

void StandardCondition::loadJSONDataInternal(var data)
{
	Condition::loadJSONDataInternal(data);
	if (comparator != nullptr)
	{
		comparator->loadJSONData(data.getProperty("comparator", var()));
		isValid->alwaysNotify = comparator->alwaysTrigger->boolValue();
	}
	else if (Engine::mainEngine->isLoadingFile)
	{
		loadingComparatorData = data.getProperty("comparator", var());
	}
}


void StandardCondition::setSourceControllable(WeakReference<Controllable> c)
{
	if (!sourceControllable.wasObjectDeleted() && sourceControllable != nullptr)
	{
		if (sourceControllable->type == Controllable::TRIGGER) ((Trigger *)sourceControllable.get())->removeTriggerListener(this);
		else ((Parameter *)sourceControllable.get())->removeParameterListener(this);

		Module * m = ControllableUtil::findParentAs<Module>(sourceControllable);
		if(m != nullptr) unregisterLinkedInspectable(m);
	}

	sourceControllable = c;


	if (sourceControllable != nullptr)
	{
		var oldData = var();
		if (comparator != nullptr) oldData = comparator->getJSONData();
		
		if (comparator != nullptr)
		{
			removeChildControllableContainer(comparator.get());
		}

		comparator.reset(ComparatorFactory::createComparatorForControllable(sourceControllable));


		addChildControllableContainer(comparator.get());

		Module * m = ControllableUtil::findParentAs<Module>(sourceControllable);
		if (m != nullptr) registerLinkedInspectable(m);

		if (comparator != nullptr)
		{
			if (!isCurrentlyLoadingData)
			{
				if (!loadingComparatorData.isVoid())
				{
					comparator->loadJSONData(loadingComparatorData);
					//loadingComparatorData = var();
				}
				else if (!oldData.isVoid()) comparator->loadJSONData(oldData);
			}
			
			comparator->hideInEditor = true;
			comparator->addComparatorListener(this);
			comparator->compare();
			
			isValid->alwaysNotify = comparator->alwaysTrigger->boolValue();
			isValid->setValue(comparator->isValid);

		}
	} else
	{
		if (comparator != nullptr)
		{
			loadingComparatorData = comparator->getJSONData();
			comparator->removeComparatorListener(this);
			removeChildControllableContainer(comparator.get());
			comparator = nullptr;
			
		}
	}

	conditionListeners.call(&ConditionListener::conditionSourceChanged, this);
	conditionAsyncNotifier.addMessage(new ConditionEvent(ConditionEvent::SOURCE_CHANGED, this));

	
}

void StandardCondition::onContainerParameterChangedInternal(Parameter * p)
{
	Condition::onContainerParameterChangedInternal(p);
	if (p == sourceTarget && sourceTarget != nullptr)
	{
		setSourceControllable(sourceTarget->target);
	}
}

void StandardCondition::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	Condition::onControllableFeedbackUpdateInternal(cc, c);

	if (comparator != nullptr && c == comparator->alwaysTrigger)
	{
		isValid->alwaysNotify = comparator->alwaysTrigger->boolValue();
	}
}

InspectableEditor * StandardCondition::getEditor(bool isRoot)
{
	return new StandardConditionEditor(this, isRoot);
}


void StandardCondition::comparatorValidationChanged(BaseComparator *)
{
	isValid->setValue(comparator->isValid);
}

