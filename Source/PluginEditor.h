#pragma once

// ---------------------------------------------------------------------------
// SuccoAudioProcessorEditor - main UI. Hosts:
//   - top tab bar (module navigation)
//   - central display (logo + preset name + meter)
//   - module-specific panel area (knobs swap with the tab)
//   - bottom row (Input / Punch / Mix / Output)
// ---------------------------------------------------------------------------

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "gui/SuccoLookAndFeel.h"
#include "gui/KnobComponent.h"
#include "gui/MeterComponent.h"
#include "gui/ModuleTabBar.h"
#include "gui/DisplayComponent.h"
#include <array>
#include <vector>

namespace succo
{

// A panel holds a set of knobs for one module tab.
// Optional: an "extra component" (meter / curve display) positioned at the top
// of the panel, taking a configurable height.
class ModulePanel : public juce::Component
{
public:
    ModulePanel (SuccoLookAndFeel* lf, const juce::String& title)
        : lookAndFeel (lf), panelTitle (title) {}

    void addKnob (std::unique_ptr<KnobComponent> knob)
    {
        addAndMakeVisible (*knob);
        knobs.push_back (std::move (knob));
    }

    // Set an extra component (meter, curve display, etc.) at the top of the panel
    void setExtraComponent (std::unique_ptr<juce::Component> comp, int height)
    {
        extraHeight = height;
        extraComponent = std::move (comp);
        if (extraComponent != nullptr)
            addAndMakeVisible (*extraComponent);
    }

    // Take ownership of any non-knob child (Label, TextButton, etc.)
    template <typename T>
    T* addOwned (std::unique_ptr<T> comp)
    {
        auto* raw = comp.get();
        addAndMakeVisible (*raw);
        ownedComponents.push_back (std::move (comp));
        return raw;
    }

    void paint (juce::Graphics& g) override
    {
        if (lookAndFeel != nullptr)
            lookAndFeel->drawModulePanel (g, getLocalBounds(), panelTitle, true);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (8);
        b.removeFromTop (22); // title bar

        // Reserve space for extra component on the right side (full height)
        // Used for vertical GR meters, etc.
        if (extraComponent != nullptr && extraHeight > 0)
        {
            auto extraArea = b.removeFromRight (extraHeight).reduced (2); // height == width here
            extraComponent->setBounds (extraArea);
            b.removeFromRight (4);
        }

        // Knob grid below
        const int cols = 4;
        const int rows = (int) std::ceil ((float) knobs.size() / (float) cols);
        const int itemW = b.getWidth() / cols;
        const int itemH = juce::jmin (b.getHeight() / std::max (1, rows), 100);

        for (size_t i = 0; i < knobs.size(); ++i)
        {
            const int r = (int) i / cols;
            const int c = (int) i % cols;
            const auto cell = juce::Rectangle<int> (b.getX() + c * itemW,
                                                     b.getY() + r * itemH,
                                                     itemW, itemH).reduced (4);
            knobs[i]->setBounds (cell);
        }

        // Owned non-knob components fill the same area
        if (! ownedComponents.empty() && extraComponent == nullptr)
            for (auto& c : ownedComponents)
                c->setBounds (b);
    }

    SuccoLookAndFeel* getLookAndFeel() const { return lookAndFeel; }
    const juce::String& getTitle() const { return panelTitle; }

    std::vector<std::unique_ptr<KnobComponent>> knobs;
    std::vector<std::unique_ptr<juce::Component>> ownedComponents;
    std::unique_ptr<juce::Component> extraComponent;
    int extraHeight = 0;
    SuccoLookAndFeel* lookAndFeel = nullptr;
    juce::String panelTitle;
};

class SuccoAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer,
                                  private ModuleTabBar::Listener
{
public:
    SuccoAudioProcessorEditor (SuccoAudioProcessor&);
    ~SuccoAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static const std::vector<juce::String>& getTabNames();

private:
    void timerCallback() override;
    void tabSelected (int moduleIndex) override;
    void buildModulePanels();
    void showPanelForCurrentModule();
    void refreshPresetLabel();
    void refreshPresetCombo();

    SuccoAudioProcessor& processor;
    SuccoLookAndFeel lookAndFeel;

    ModuleTabBar tabBar;
    DisplayComponent display;

    // Bottom-row macro knobs
    std::unique_ptr<KnobComponent> knobInput;
    std::unique_ptr<KnobComponent> knobPunch;
    std::unique_ptr<KnobComponent> knobMix;
    std::unique_ptr<KnobComponent> knobOutput;

    // 13 module panels (HOME, TUNE, GATE, EQ, COMP, DE-ESS, DRIVE, DOUBLE,
    // CHORUS, DELAY, VERB, LIMIT, PRESETS)
    std::array<std::unique_ptr<ModulePanel>, 13> modulePanels;

    // GR meters (live-updated from processor)
    MeterComponent* compGRMeter = nullptr;
    MeterComponent* limiterGRMeter = nullptr;

    juce::Label presetLabel;
    juce::TextButton prevPreset { "<" };
    juce::TextButton nextPreset { ">" };
    juce::ComboBox presetCombo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuccoAudioProcessorEditor)
};

} // namespace succo
