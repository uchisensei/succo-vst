#pragma once

// ---------------------------------------------------------------------------
// SuccoLookAndFeel - dark purple theme for SUCCO.
// Custom knob rendering, typography, and component background colours.
// ---------------------------------------------------------------------------

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

namespace succo
{

class SuccoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SuccoLookAndFeel()
    {
        colBackground     = juce::Colour (0xff1a0d24);   // very dark purple
        colBackgroundAlt  = juce::Colour (0xff2a1438);
        colPanel          = juce::Colour (0xff341a4a);
        colPanelAlt       = juce::Colour (0xff42225e);
        colAccent         = juce::Colour (0xffb975ff);   // bright purple accent
        colAccentBright   = juce::Colour (0xffe0a3ff);
        colAccentDim      = juce::Colour (0xff6b3da3);
        colText           = juce::Colour (0xfff0e6ff);
        colTextDim        = juce::Colour (0xff9b85b8);
        colKnob           = juce::Colour (0xff1f1030);
        colKnobFill       = juce::Colour (0xffb975ff);
        colDisplay        = juce::Colour (0xff2a0d3a);
        colDisplayBorder  = juce::Colour (0xff6b3da3);
        colGood           = juce::Colour (0xff4dd599);
        colWarn           = juce::Colour (0xffffcc44);
        colDanger         = juce::Colour (0xffff5577);

        setColour (juce::ResizableWindow::backgroundColourId, colBackground);
        setColour (juce::DocumentWindow::backgroundColourId, colBackground);
        setColour (juce::ComboBox::backgroundColourId, colPanel);
        setColour (juce::ComboBox::textColourId, colText);
        setColour (juce::ComboBox::arrowColourId, colAccent);
        setColour (juce::PopupMenu::backgroundColourId, colPanel);
        setColour (juce::PopupMenu::textColourId, colText);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, colAccentDim);
        setColour (juce::PopupMenu::highlightedTextColourId, colText);
        setColour (juce::TextButton::buttonColourId, colPanel);
        setColour (juce::TextButton::buttonOnColourId, colAccent);
        setColour (juce::TextButton::textColourOffId, colText);
        setColour (juce::TextButton::textColourOnId, colBackground);
        setColour (juce::Label::textColourId, colText);
        setColour (juce::Label::textWhenEditingColourId, colAccentBright);
        setColour (juce::Slider::textBoxTextColourId, colText);
        setColour (juce::Slider::textBoxBackgroundColourId, colPanel);
        setColour (juce::Slider::textBoxOutlineColourId, colAccentDim);
        setColour (juce::Slider::thumbColourId, colAccent);
        setColour (juce::Slider::rotarySliderFillColourId, colAccent);
        setColour (juce::Slider::rotarySliderOutlineColourId, colKnob);

        // JUCE 8: this is a member of LookAndFeel, not Font. 'this' is a LookAndFeel subclass.
        setDefaultSansSerifTypefaceName ("Verdana");
    }

    // -------- Custom knob drawing --------
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float pos, const float startAngle, const float endAngle,
                            juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<float> ((float) x, (float) y,
                                                     (float) width, (float) height).reduced (4.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = startAngle + pos * (endAngle - startAngle);

        const float trackWidth = radius * 0.22f;
        const float trackRadius = radius * 0.85f;

        // Background ring (track)
        juce::Path bgRing;
        bgRing.addCentredArc (centre.x, centre.y, trackRadius, trackRadius,
            0.0f, startAngle, endAngle, true);
        g.setColour (colKnob);
        g.strokePath (bgRing, juce::PathStrokeType (trackWidth));

        // Filled arc up to current pos
        juce::Path fillRing;
        fillRing.addCentredArc (centre.x, centre.y, trackRadius, trackRadius,
            0.0f, startAngle, angle, true);
        g.setColour (colKnobFill);
        g.strokePath (fillRing, juce::PathStrokeType (trackWidth));

        // Inner knob disc with radial gradient
        const float knobRadius = radius * 0.55f;
        juce::ColourGradient grad (
            colAccentBright.withAlpha (0.4f),
            centre.x - knobRadius * 0.3f, centre.y - knobRadius * 0.3f,
            colKnob,
            centre.x + knobRadius * 0.3f, centre.y + knobRadius * 0.3f,
            true);
        g.setGradientFill (grad);
        g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius,
                       knobRadius * 2.0f, knobRadius * 2.0f);

        // Outline
        g.setColour (colAccent.withAlpha (0.8f));
        g.drawEllipse (centre.x - knobRadius, centre.y - knobRadius,
                       knobRadius * 2.0f, knobRadius * 2.0f, 1.5f);

        // Pointer line
        const float pointerLen = knobRadius * 0.85f;
        const float pointerX = centre.x + std::sin (angle) * pointerLen;
        const float pointerY = centre.y - std::cos (angle) * pointerLen;
        juce::Path pointer;
        pointer.startNewSubPath (centre.x, centre.y);
        pointer.lineTo (pointerX, pointerY);
        g.setColour (colAccentBright);
        g.strokePath (pointer, juce::PathStrokeType (2.5f));
    }

    // -------- Display panel background --------
    void drawDisplayBackground (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        const auto f = bounds.toFloat();
        juce::ColourGradient grad (colPanel, f.getCentreX(), f.getCentreY(),
            colDisplay, f.getX(), f.getBottom(), true);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (f, 8.0f);

        g.setColour (colDisplayBorder);
        g.drawRoundedRectangle (f, 8.0f, 1.5f);
    }

    // -------- Getters --------
    juce::Colour getBackgroundColour() const { return colBackground; }
    juce::Colour getPanelColour() const     { return colPanel; }
    juce::Colour getAccentColour() const    { return colAccent; }
    juce::Colour getAccentBright() const   { return colAccentBright; }
    juce::Colour getAccentDim() const      { return colAccentDim; }
    juce::Colour getTextColour() const     { return colText; }
    juce::Colour getTextDim() const         { return colTextDim; }
    juce::Colour getDanger() const          { return colDanger; }
    juce::Colour getGood() const            { return colGood; }

private:
    juce::Colour colBackground, colBackgroundAlt, colPanel, colPanelAlt;
    juce::Colour colAccent, colAccentBright, colAccentDim;
    juce::Colour colText, colTextDim;
    juce::Colour colKnob, colKnobFill;
    juce::Colour colDisplay, colDisplayBorder;
    juce::Colour colGood, colWarn, colDanger;
};

} // namespace succo
