#pragma once

// ---------------------------------------------------------------------------
// ModulePresetBar - per-module preset UI bar placed at the top of each
// module panel. Contains:
//   - "PRESET:" label
//   - ComboBox dropdown listing saved presets for this module
//   - Save button (prompts for name)
//   - Delete button (removes selected preset)
//   - A button (recall A snapshot)
//   - B button (recall B snapshot)
//   - A→B button (copy current state to B)
//
// This is the FabFilter / plugin suite workflow: each module has its own
// preset library independent of the global chain preset.
// ---------------------------------------------------------------------------

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../presets/ModulePresetManager.h"
#include "SuccoLookAndFeel.h"

namespace succo
{

class ModulePresetBar : public juce::Component
{
public:
    ModulePresetBar (SuccoLookAndFeel* lf,
                     ModulePresetManager& mgr,
                     const juce::String& moduleName)
        : lookAndFeel (lf), presetManager (mgr), module (moduleName)
    {
        // Label
        presetLabel.setText ("PRESET:", juce::dontSendNotification);
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff9b85b8));
        presetLabel.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
        presetLabel.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (presetLabel);

        // Dropdown
        refreshDropdown();
        presetCombo.setTextWhenNothingSelected ("(no preset)");
        presetCombo.onChange = [this]
        {
            const int idx = presetCombo.getSelectedItemIndex();
            if (idx >= 0)
                presetManager.loadModulePreset (module, presetCombo.getItemText (idx));
        };
        addAndMakeVisible (presetCombo);

        // Save button
        saveBtn.setButtonText ("SAVE");
        saveBtn.setTooltip ("Salva il preset corrente di " + module);
        saveBtn.onClick = [this]
        {
            // Use AlertWindow to prompt for a name (JUCE 8 with modal loops enabled)
            juce::AlertWindow w ("Save preset for " + module,
                                  "Enter the preset name:", juce::AlertWindow::NoIcon, this);
            w.addTextEditor ("name", "My " + module + " Preset");
            w.addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
            w.addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
            if (w.runModalLoop() == 1)
            {
                const auto name = w.getTextEditorContents ("name");
                if (name.isNotEmpty())
                {
                    presetManager.saveModulePreset (module, name);
                    refreshDropdown();
                    presetCombo.setText (name, juce::dontSendNotification);
                }
            }
        };
        styleButton (saveBtn);
        addAndMakeVisible (saveBtn);

        // Delete button
        deleteBtn.setButtonText ("DEL");
        deleteBtn.setTooltip ("Elimina il preset selezionato");
        deleteBtn.onClick = [this]
        {
            const auto name = presetCombo.getText();
            if (name.isEmpty())
                return;
            auto file = presetManager.getModuleFolder (module).getChildFile (name + ".xml");
            if (file.deleteFile())
                refreshDropdown();
        };
        styleButton (deleteBtn);
        addAndMakeVisible (deleteBtn);

        // A/B buttons
        btnA.setButtonText ("A");
        btnA.setTooltip ("Recall snapshot A for " + module);
        btnA.setClickingTogglesState (false);
        btnA.onClick = [this] { presetManager.recallABState (module, 'A'); flashABButton ('A'); };
        styleButton (btnA);
        addAndMakeVisible (btnA);

        btnB.setButtonText ("B");
        btnB.setTooltip ("Recall snapshot B for " + module);
        btnB.setClickingTogglesState (false);
        btnB.onClick = [this] { presetManager.recallABState (module, 'B'); flashABButton ('B'); };
        styleButton (btnB);
        addAndMakeVisible (btnB);

        btnStoreA.setButtonText (">A");
        btnStoreA.setTooltip ("Store current state to A");
        btnStoreA.onClick = [this] { presetManager.storeABState (module, 'A'); flashABButton ('A'); };
        styleButton (btnStoreA);
        addAndMakeVisible (btnStoreA);

        btnStoreB.setButtonText (">B");
        btnStoreB.setTooltip ("Store current state to B");
        btnStoreB.onClick = [this] { presetManager.storeABState (module, 'B'); flashABButton ('B'); };
        styleButton (btnStoreB);
        addAndMakeVisible (btnStoreB);
    }

    void paint (juce::Graphics& g) override
    {
        // Subtle background bar with yellow accent on left
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff2a1438).withAlpha (0.5f));
        g.fillRoundedRectangle (b, 3.0f);
        g.setColour (lookAndFeel != nullptr ? lookAndFeel->getYellow() : juce::Colours::yellow);
        g.fillRoundedRectangle (juce::Rectangle<float> (b.getX(), b.getY(), 3.0f, b.getHeight()), 1.5f);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (4, 2);
        const int labelW = 50;
        const int comboW = 200;
        const int btnW = 38;
        const int gap = 4;

        presetLabel.setBounds (b.removeFromLeft (labelW));
        b.removeFromLeft (gap);
        presetCombo.setBounds (b.removeFromLeft (comboW));
        b.removeFromLeft (gap);
        saveBtn.setBounds (b.removeFromLeft (btnW));
        b.removeFromLeft (gap);
        deleteBtn.setBounds (b.removeFromLeft (btnW));
        b.removeFromLeft (gap * 4);
        btnA.setBounds (b.removeFromLeft (btnW));
        b.removeFromLeft (gap);
        btnB.setBounds (b.removeFromLeft (btnW));
        b.removeFromLeft (gap * 2);
        btnStoreA.setBounds (b.removeFromLeft (btnW));
        b.removeFromLeft (gap);
        btnStoreB.setBounds (b.removeFromLeft (btnW));
    }

    void refreshDropdown()
    {
        presetCombo.clear();
        const auto list = presetManager.getModulePresets (module);
        for (int i = 0; i < list.size(); ++i)
            presetCombo.addItem (list[i], i + 1);
    }

private:
    void styleButton (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff341a4a));
        btn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffd93d));
        btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff0e6ff));
        btn.setColour (juce::TextButton::textColourOnId, juce::Colour (0xff1a0d24));
        btn.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff6b3da3));
    }

    void flashABButton (char slot)
    {
        auto& btn = (slot == 'A') ? btnA : btnB;
        btn.setToggleState (true, juce::dontSendNotification);
        juce::Timer::callAfterDelay (200, [safeBtn = juce::Component::SafePointer<juce::TextButton> (&btn)]() {
            if (safeBtn != nullptr)
                safeBtn->setToggleState (false, juce::dontSendNotification);
        });
    }

    SuccoLookAndFeel* lookAndFeel;
    ModulePresetManager& presetManager;
    juce::String module;

    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::TextButton saveBtn, deleteBtn;
    juce::TextButton btnA, btnB, btnStoreA, btnStoreB;
};

} // namespace succo
