/*
  ==============================================================================

    CVGroup.cpp
    Created: 15 Feb 2018 3:49:35pm
    Author:  Ben

  ==============================================================================
*/

#include "CVGroup.h"

CVGroup::CVGroup(const String & name) :
	BaseItem(name),
	manager("Variables")
{

}

CVGroup::~CVGroup()
{
}
