#pragma once

// ---------------------------------------------------------------------------
// DisplayComponent - the central purple screen. Shows:
//   - the plugin logo "SUCCO"
//   - the currently-selected module name
//   - a simple input/output level meter (peak trace)
//   - the preset name (or chain hint when no preset is loaded)
// ---------------------------------------------------------------------------

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <atomic>
#include "SuccoLookAndFeel.h"

namespace succo
{

class DisplayComponent : public juce::Component,
                          private juce::Timer
{
public:
    DisplayComponent (SuccoLookAndFeel* lf, std::function<float()> getLevelFn)
        : lookAndFeel (lf), getLevel (std::move (getLevelFn))
    {
        startTimerHz (60);
    }

    void setPresetName (const juce::String& name) { presetName = name; repaint(); }
    void setModuleName (const juce::String& name) { moduleName = name; repaint(); }

    void paint (juce::Graphics& g) override
    {
        if (lookAndFeel == nullptr) return;
        const auto b = getLocalBounds();
        lookAndFeel->drawDisplayBackground (g, b);

        // Title bar
        auto titleArea = b.removeFromTop (22);
        g.setColour (lookAndFeel->getTextColour());
        g.setFont (juce::Font (15.0f, juce::Font::bold));
        g.drawText ("NOW POURING: " + presetName, titleArea.reduced (8, 2),
                    juce::Justification::centredLeft, true);

        g.setColour (lookAndFeel->getAccentColour());
        g.setFont (juce::Font (11.0f));
        g.drawText (moduleName, titleArea.reduced (8, 2),
                    juce::Justification::centredRight, true);

        // Waveform / level meter
        auto meterArea = b.removeFromTop (b.getHeight() / 2).reduced (8, 4);
        drawMeter (g, meterArea);

        // Logo area
        drawLogo (g, b);
    }

    void timerCallback() override
    {
        const float level = getLevel ? getLevel() : 0.0f;
        smoothedLevel = juce::jmax (level, smoothedLevel * 0.92f);
        repaint();
    }

private:
    SuccoLookAndFeel* lookAndFeel;
    std::function<float()> getLevel;
    juce::String presetName { "<INIT>" };
    juce::String moduleName { "HOME" };
    float smoothedLevel = 0.0f;

    void drawMeter (juce::Graphics& g, juce::Rectangle<int> area)
    {
        g.setColour (lookAndFeel->getBackgroundColour().withAlpha (0.4f));
        g.fillRoundedRectangle (area.toFloat(), 4.0f);

        const auto h = (float) area.getHeight();
        const auto w = (float) area.getWidth();
        const auto cx = area.getCentreX();
        const auto cy = area.getCentreY();
        const float len = smoothedLevel * w * 0.45f;

        // Center line
        g.setColour (lookAndFeel->getTextDim().withAlpha (0.3f));
        g.drawVerticalLine (cx, area.getY() + 4.0f, area.getBottom() - 4.0f);

        if (len > 0.5f)
        {
            // Stereo L/R bars from center
            juce::Colour barColor = lookAndFeel->getAccentBright();
            if (smoothedLevel > 0.85f) barColor = lookAndFeel->getDanger();
            else if (smoothedLevel > 0.7f) barColor = juce::Colours::orange;

            g.setColour (barColor);
            g.fillRoundedRectangle (juce::Rectangle<float> (cx - len, cy - 4.0f, len * 2.0f, 8.0f), 2.0f);
        }

        // Grid lines
        g.setColour (lookAndFeel->getTextDim().withAlpha (0.15f));
        for (int i = 1; i < 8; ++i)
        {
            const float x = area.getX() + (float) i * w / 8.0f;
            g.drawVerticalLine ((int) x, area.getY() + 4.0f, area.getBottom() - 4.0f);
        }
    }

    void drawLogo (juce::Graphics& g, juce::Rectangle<int> area)
    {
        const auto bounds = area.toFloat().reduced (4.0f);

        // Glow background
        juce::ColourGradient glow (lookAndFeel->getAccentColour().withAlpha (0.15f),
                                    bounds.getCentreX(), bounds.getCentreY(),
            juce::Colours::transparentBlack,
            bounds.getCentreX(), bounds.getBottom(), true);
        g.setGradientFill (glow);
        g.fillRect (bounds);

        // 3D-ish "SUCCO" text with layered shadow
        const juce::String logoText = "SUCCO";
        const juce::Font logoFont (36.0f, juce::Font::bold);

        g.setColour (lookAndFeel->getAccentColour().withAlpha (0.4f));
        g.setFont (logoFont);
        g.drawText (logoText, bounds.expanded (0, 2).translated (2.0f, 2.0f),
                    juce::Justification::centred, true);

        g.setColour (lookAndFeel->getAccentBright());
        g.drawText (logoText, bounds, juce::Justification::centred, true);

        // Subtitle
        g.setColour (lookAndFeel->getTextDim());
        g.setFont (juce::Font (10.0f));
        g.drawText ("VOCAL CHAIN  v1.0", bounds.translated (0, 18),
                    juce::Justification::centred, true);
    }
};

} // namespace succo
