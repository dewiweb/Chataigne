/*
  ==============================================================================

    ChataigneAudioLayerPanel.cpp
    Created: 20 Nov 2016 3:08:49pm
    Author:  Ben Kuper

  ==============================================================================
*/

#include "ChataigneAudioLayerPanel.h"

ChataigneAudioLayerPanel::ChataigneAudioLayerPanel(ChataigneAudioLayer * layer) :
	AudioLayerPanel(layer),
	chataigneAudioLayer(layer)
{
	
	moduleChooser.setTextWhenNoChoicesAvailable("No audio module");
	moduleChooser.setTextWhenNothingSelected("Choose an audio module");
	
	moduleChooser.filterModuleFunc = &ChataigneAudioLayerPanel::isAudioModule;
	moduleChooser.buildModuleBox();

	moduleChooser.addChooserListener(this);
	moduleChooser.setModuleSelected(chataigneAudioLayer->audioModule,true);

	addAndMakeVisible(&moduleChooser);

	chataigneAudioLayer->addAudioLayerListener(this);
}

ChataigneAudioLayerPanel::~ChataigneAudioLayerPanel()
{
	if (!inspectable.wasObjectDeleted()) chataigneAudioLayer->removeAudioLayerListener(this);
}


void ChataigneAudioLayerPanel::resizedInternalContent(Rectangle<int>& r)
{
	SequenceLayerPanel::resizedInternalContent(r);  
	Rectangle<int> gr = r.reduced(2).removeFromTop(16);

	moduleChooser.setBounds(gr.removeFromLeft(80));
	enveloppeUI->setBounds(gr);
}

void ChataigneAudioLayerPanel::targetAudioModuleChanged(ChataigneAudioLayer *)
{
	moduleChooser.setModuleSelected(chataigneAudioLayer->audioModule,true);
}

void ChataigneAudioLayerPanel::selectedModuleChanged(ModuleChooserUI *, Module * m)
{
	chataigneAudioLayer->setAudioModule(dynamic_cast<AudioModule *>(m));
}

void ChataigneAudioLayerPanel::moduleListChanged(ModuleChooserUI *)
{
	moduleChooser.setModuleSelected(chataigneAudioLayer->audioModule, true);

}

void ChataigneAudioLayerPanel::mouseDown(const MouseEvent& e)
{
	AudioLayerPanel::mouseDown(e);

	if (e.mods.isRightButtonDown())
	{
		if (e.eventComponent == this)
		{
			PopupMenu p;
			p.addItem(1, "Export Enveloppe to new mapping layer");
			p.addItem(2, "Export Enveloppe to clipboard");
			p.addItem(3, "Export Enveloppe to clipboard (data only)");

			int result = p.show();
			switch (result)
			{
			case 1:
				chataigneAudioLayer->exportRMS(true, false, false);
				break;

			case 2:
				chataigneAudioLayer->exportRMS(false, true, false);
				break;

			case 3:
				chataigneAudioLayer->exportRMS(false, true, true);
				break;
			}
		}
	}
	
}
