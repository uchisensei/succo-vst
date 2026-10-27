#include "PluginEditor.h"
#include "params/ParameterManager.h"

namespace succo
{

// Static table of tab names
const std::vector<juce::String>& SuccoAudioProcessorEditor::getTabNames()
{
    static const std::vector<juce::String> names = {
        "HOME", "TUNE", "GATE", "EQ", "COMP", "DE-ESS",
        "DRIVE", "DOUBLE", "CHORUS", "DELAY", "VERB", "LIMIT", "PRESETS"
    };
    return names;
}

SuccoAudioProcessorEditor::SuccoAudioProcessorEditor (SuccoAudioProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processor (p),
      lookAndFeel(),
      tabBar (p.getAPVTS(), &lookAndFeel),
      display (&lookAndFeel, [this]() { return processor.getOutputLevelLin(); })
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (tabBar);
    tabBar.setListener (this);

    addAndMakeVisible (display);

    // ---- Bottom-row macro knobs ----
    using namespace ParamIDs;
    knobInput  = std::make_unique<KnobComponent> (processor.getAPVTS(), inputGain,  "INPUT",  "Guadagno in ingresso (dB)");
    knobPunch  = std::make_unique<KnobComponent> (processor.getAPVTS(), punch,      "PUNCH",  "Mix di compressione parallela per dare impatto");
    knobMix    = std::make_unique<KnobComponent> (processor.getAPVTS(), mix,        "MIX",    "Bilanciamento segnale secco/processato");
    knobOutput = std::make_unique<KnobComponent> (processor.getAPVTS(), outputGain, "OUTPUT", "Guadagno in uscita (dB)");

    for (auto* k : { knobInput.get(), knobPunch.get(), knobMix.get(), knobOutput.get() })
    {
        k->setLookAndFeelToUse (&lookAndFeel);
        addAndMakeVisible (*k);
    }

    // ---- Preset bar ----
    presetLabel.setColour (juce::Label::textColourId, lookAndFeel.getTextColour());
    presetLabel.setJustificationType (juce::Justification::centredLeft);
    presetLabel.setFont (juce::Font (13.0f));
    addAndMakeVisible (presetLabel);

    addAndMakeVisible (prevPreset);
    prevPreset.onClick = [this]
    {
        const int n = processor.getPresetManager().getNumPresets();
        const int i = processor.getPresetManager().getCurrentPresetIndex();
        const int next = (i - 1 + n) % n;
        processor.getPresetManager().loadPreset (next);
        refreshPresetLabel();
    };

    addAndMakeVisible (nextPreset);
    nextPreset.onClick = [this]
    {
        const int n = processor.getPresetManager().getNumPresets();
        const int i = processor.getPresetManager().getCurrentPresetIndex();
        const int next = (i + 1) % n;
        processor.getPresetManager().loadPreset (next);
        refreshPresetLabel();
    };

    presetCombo.setTextWhenNothingSelected ("Select preset...");
    presetCombo.onChange = [this]
    {
        const int idx = presetCombo.getSelectedItemIndex();
        if (idx >= 0)
        {
            processor.getPresetManager().loadPreset (idx);
            refreshPresetLabel();
        }
    };
    addAndMakeVisible (presetCombo);
    refreshPresetCombo();
    refreshPresetLabel();

    buildModulePanels();
    showPanelForCurrentModule();

    // Window size
    setSize (1100, 720);
    setResizable (true, true);
    setResizeLimits (900, 600, 1600, 1000);

    startTimerHz (30);
}

SuccoAudioProcessorEditor::~SuccoAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SuccoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.getBackgroundColour());

    // Title bar
    auto top = getLocalBounds().removeFromTop (40);
    g.setColour (lookAndFeel.getPanelColour());
    g.fillRect (top);

    g.setColour (lookAndFeel.getTextColour());
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawText ("SUCCO", top.reduced (12, 4), juce::Justification::centredLeft, true);

    g.setColour (lookAndFeel.getTextDim());
    g.setFont (juce::Font (11.0f));
    g.drawText ("Vocal Chain  |  v1.0",
                top.reduced (12, 4), juce::Justification::centredRight, true);
}

void SuccoAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();
    b.removeFromTop (40);  // title bar painted over

    // Top tab bar
    tabBar.setBounds (b.removeFromTop (28).reduced (2, 2));

    // Preset bar
    auto presetArea = b.removeFromTop (28).reduced (8, 2);
    prevPreset.setBounds (presetArea.removeFromLeft (24));
    nextPreset.setBounds (presetArea.removeFromLeft (24));
    presetCombo.setBounds (presetArea.removeFromLeft (200));
    presetLabel.setBounds (presetArea);

    // Main content area: display on left, panel on right
    auto content = b.removeFromTop (440);
    auto displayArea = content.removeFromLeft (520).reduced (8);
    display.setBounds (displayArea);

    auto rightArea = content.reduced (8);
    for (auto& panel : modulePanels)
        if (panel != nullptr)
            panel->setBounds (rightArea);

    // Bottom macro row
    auto bottom = b.removeFromTop (110).reduced (8);
    const int knobCount = 4;
    const int kw = bottom.getWidth() / knobCount;
    knobInput->setBounds  (bottom.removeFromLeft (kw).reduced (4));
    knobPunch->setBounds  (bottom.removeFromLeft (kw).reduced (4));
    knobMix->setBounds    (bottom.removeFromLeft (kw).reduced (4));
    knobOutput->setBounds (bottom.removeFromLeft (kw).reduced (4));
}

void SuccoAudioProcessorEditor::timerCallback()
{
    refreshPresetLabel();
}

void SuccoAudioProcessorEditor::tabSelected (int moduleIndex)
{
    showPanelForCurrentModule();
    const auto& names = getTabNames();
    if (moduleIndex >= 0 && moduleIndex < (int) names.size())
        display.setModuleName (names[(size_t) moduleIndex]);
}

void SuccoAudioProcessorEditor::showPanelForCurrentModule()
{
    // ui_module is AudioParameterChoice: normalized 0..1, must convert to integer
    const float norm = processor.getAPVTS().getRawParameterValue ("ui_module")->load();
    const int idx = juce::roundToInt (norm * 12.0f);  // 13 options -> 0..12
    for (size_t i = 0; i < modulePanels.size(); ++i)
        if (modulePanels[i] != nullptr)
            modulePanels[i]->setVisible ((int) i == idx);
    repaint();
}

void SuccoAudioProcessorEditor::refreshPresetLabel()
{
    const int i = processor.getPresetManager().getCurrentPresetIndex();
    const juce::String name = processor.getPresetManager().getPresetName (i);
    presetLabel.setText ("PRESET: " + name, juce::dontSendNotification);
    display.setPresetName (name);
    const auto& names = getTabNames();
    const float norm = processor.getAPVTS().getRawParameterValue ("ui_module")->load();
    const int modIdx = juce::roundToInt (norm * 12.0f);
    if (modIdx >= 0 && modIdx < (int) names.size())
        display.setModuleName (names[(size_t) modIdx]);
}

void SuccoAudioProcessorEditor::refreshPresetCombo()
{
    presetCombo.clear();
    const int n = processor.getPresetManager().getNumPresets();
    for (int i = 0; i < n; ++i)
        presetCombo.addItem (processor.getPresetManager().getPresetName (i), i + 1);
    presetCombo.setSelectedItemIndex (processor.getPresetManager().getCurrentPresetIndex(),
                                       juce::dontSendNotification);
}

// ---- Build per-module knob panels ----
void SuccoAudioProcessorEditor::buildModulePanels()
{
    using namespace ParamIDs;
    auto& apvts = processor.getAPVTS();

    auto newPanel = [this](int idx) -> ModulePanel*
    {
        auto p = std::make_unique<ModulePanel>();
        auto* raw = p.get();
        modulePanels[(size_t) idx] = std::move (p);
        addAndMakeVisible (*raw);
        raw->setVisible (false);
        return raw;
    };

    auto addKnobTo = [this, &apvts](ModulePanel* panel, const char* pid,
                                    const char* label, const char* tipIT)
    {
        auto k = std::make_unique<KnobComponent> (apvts, pid, label, tipIT);
        k->setLookAndFeelToUse (&lookAndFeel);
        panel->addKnob (std::move (k));
    };

    // TUNE (1)
    {
        auto* p = newPanel (1);
        addKnobTo (p, tuneEnable,     "ENABLE",    "Attiva il modulo Tune (pitch shift)");
        addKnobTo (p, tuneSemitones, "SEMITONES", "Shift in semitoni (-12..+12)");
        addKnobTo (p, tuneAmount,    "AMOUNT",    "Quantita applicata (0..100%)");
    }

    // GATE (2)
    {
        auto* p = newPanel (2);
        addKnobTo (p, gateEnable, "ENABLE",  "Attiva il gate");
        addKnobTo (p, gateThresh, "THRESH",  "Soglia (dB sotto la quale il gate si chiude)");
        addKnobTo (p, gateRange,  "RANGE",   "Attenuazione a gate chiuso (dB)");
        addKnobTo (p, gateAttack, "ATTACK",  "Tempo di attacco (ms)");
        addKnobTo (p, gateHold,    "HOLD",    "Tempo di hold (ms)");
        addKnobTo (p, gateRelease,"RELEASE", "Tempo di rilascio (ms)");
    }

    // EQ (3)
    {
        auto* p = newPanel (3);
        addKnobTo (p, eqEnable,        "ENABLE",   "Attiva l'equalizzatore");
        addKnobTo (p, eqHpfOn,         "HPF ON",   "Abilita il filtro passaalto");
        addKnobTo (p, eqHpfFreq,       "HPF",      "Frequenza del passaalto (Hz)");
        addKnobTo (p, eqLowShelfFreq,  "LS FREQ",  "Frequenza low shelf (Hz)");
        addKnobTo (p, eqLowShelfGain,  "LS GAIN",  "Guadagno low shelf (dB)");
        addKnobTo (p, eqLowMidFreq,    "LM FREQ",  "Frequenza banda low-mid (Hz)");
        addKnobTo (p, eqLowMidQ,       "LM Q",     "Fattore Q low-mid");
        addKnobTo (p, eqLowMidGain,    "LM GAIN",  "Guadagno low-mid (dB)");
        addKnobTo (p, eqHighMidFreq,   "HM FREQ",  "Frequenza banda high-mid (Hz)");
        addKnobTo (p, eqHighMidQ,      "HM Q",     "Fattore Q high-mid");
        addKnobTo (p, eqHighMidGain,   "HM GAIN",  "Guadagno high-mid (dB)");
        addKnobTo (p, eqHighShelfFreq, "HS FREQ", "Frequenza high shelf (Hz)");
        addKnobTo (p, eqHighShelfGain, "HS GAIN", "Guadagno high shelf (dB)");
        addKnobTo (p, eqLpfOn,         "LPF ON",   "Abilita il filtro passabasso");
        addKnobTo (p, eqLpfFreq,       "LPF",      "Frequenza del passabasso (Hz)");
    }

    // COMP (4)
    {
        auto* p = newPanel (4);
        addKnobTo (p, compEnable,  "ENABLE",  "Attiva il compressore");
        addKnobTo (p, compThresh,  "THRESH",  "Soglia (dB)");
        addKnobTo (p, compRatio,   "RATIO",   "Rapporto (n:1)");
        addKnobTo (p, compAttack,  "ATTACK",  "Tempo di attacco (ms)");
        addKnobTo (p, compRelease, "RELEASE", "Tempo di rilascio (ms)");
        addKnobTo (p, compKnee,    "KNEE",    "Ammorbidimento del ginocchio (dB)");
        addKnobTo (p, compMakeup,  "MAKEUP",  "Guadagno di compensazione (dB)");
    }

    // DE-ESS (5)
    {
        auto* p = newPanel (5);
        addKnobTo (p, deEssEnable, "ENABLE",  "Attiva il de-esser");
        addKnobTo (p, deEssFreq,   "FREQ",    "Frequenza di crossover (Hz)");
        addKnobTo (p, deEssThresh, "THRESH",  "Soglia (dB)");
        addKnobTo (p, deEssAmount, "AMOUNT",  "Quantita di riduzione (0..100%)");
    }

    // DRIVE (6)
    {
        auto* p = newPanel (6);
        addKnobTo (p, driveEnable, "ENABLE",  "Attiva il drive (saturation)");
        addKnobTo (p, driveDrive,  "DRIVE",   "Quantita di drive (dB)");
        addKnobTo (p, driveShape,  "SHAPE",   "Forma del clip (0=tanh soft, 1=hard)");
        addKnobTo (p, driveTone,   "TONE",    "Tono post-tilt (-1=basso, +1=alto)");
        addKnobTo (p, driveOutput, "OUTPUT",  "Guadagno in uscita (dB)");
    }

    // DOUBLE (7)
    {
        auto* p = newPanel (7);
        addKnobTo (p, doublerEnable, "ENABLE",  "Attiva il doubler (raddoppio vocale)");
        addKnobTo (p, doublerAmount, "AMOUNT",  "Livello del raddoppio (0..100%)");
        addKnobTo (p, doublerTime,   "TIME",    "Tempo di ritardo (ms)");
        addKnobTo (p, doublerSpread, "SPREAD",  "Larghezza stereo (0..100%)");
    }

    // CHORUS (8)
    {
        auto* p = newPanel (8);
        addKnobTo (p, chorusEnable, "ENABLE",  "Attiva il chorus");
        addKnobTo (p, chorusRate,   "RATE",    "Velocita del LFO (Hz)");
        addKnobTo (p, chorusDepth,  "DEPTH",   "Profondita di modulazione (ms)");
        addKnobTo (p, chorusVoices, "VOICES",  "Numero di voci (1..4)");
        addKnobTo (p, chorusMix,   "MIX",     "Mix dry/wet");
    }

    // DELAY (9)
    {
        auto* p = newPanel (9);
        addKnobTo (p, delayEnable,   "ENABLE",    "Attiva il delay");
        addKnobTo (p, delayTime,     "TIME",      "Tempo di ritardo (ms)");
        addKnobTo (p, delayFeedback, "FEEDBACK",  "Quantita di feedback (0..95%)");
        addKnobTo (p, delayPingPong, "PING-PONG", "Modalita ping-pong");
        addKnobTo (p, delayMix,      "MIX",       "Mix dry/wet");
    }

    // VERB (10)
    {
        auto* p = newPanel (10);
        addKnobTo (p, reverbEnable,   "ENABLE",   "Attiva la reverb");
        addKnobTo (p, reverbSize,     "SIZE",     "Dimensione della stanza (0..100%)");
        addKnobTo (p, reverbDecay,    "DECAY",    "Tempo di decadimento (s)");
        addKnobTo (p, reverbDamp,     "DAMP",     "Smorzamento (Hz)");
        addKnobTo (p, reverbPredelay, "PREDELAY", "Ritardo iniziale (ms)");
        addKnobTo (p, reverbLowCut,   "LOW CUT",  "Taglio basso (Hz)");
        addKnobTo (p, reverbMix,      "MIX",      "Mix dry/wet");
    }

    // LIMIT (11)
    {
        auto* p = newPanel (11);
        addKnobTo (p, limitEnable,  "ENABLE",    "Attiva il limitatore");
        addKnobTo (p, limitCeiling, "CEILING",   "Soglia massima di uscita (dB)");
        addKnobTo (p, limitRelease, "RELEASE",   "Tempo di rilascio (ms)");
        addKnobTo (p, limitTruePeak,"TRUE PEAK", "Controllo inter-sample peak (4x OS)");
    }

    // HOME (0) - intro text
    {
        auto* p = newPanel (0);
        auto hint = std::make_unique<juce::Label>("hint",
            "SUCCO Vocal Chain\n\n"
            "Seleziona un modulo dalla barra in alto per iniziare.\n"
            "I preset sono selezionabili dal menu a tendina in basso.\n"
            "Salva i tuoi preset in ~/Documents/SUCCO/Presets");
        hint->setColour (juce::Label::textColourId, lookAndFeel.getTextColour());
        hint->setJustificationType (juce::Justification::centred);
        hint->setFont (juce::Font (14.0f));
        p->addAndMakeVisible (std::move (hint));  // transfers ownership
    }

    // PRESETS (12)
    {
        auto* p = newPanel (12);
        auto saveBtn = std::make_unique<juce::TextButton>("Save Preset");
        saveBtn->setColour (juce::TextButton::buttonColourId, lookAndFeel.getAccentColour());
        saveBtn->setColour (juce::TextButton::textColourOnId, lookAndFeel.getBackgroundColour());
        saveBtn->setColour (juce::TextButton::textColourOffId, lookAndFeel.getTextColour());
        saveBtn->onClick = [this]()
        {
            juce::AlertWindow w ("Salva preset", "Inserisci il nome del preset:", juce::AlertWindow::NoIcon);
            w.addTextEditor ("name", "My Preset");
            w.addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
            w.addButton ("Annulla", 0, juce::KeyPress (juce::KeyPress::escapeKey));
            if (w.runModalLoop() == 1)
            {
                auto name = w.getTextEditorContents ("name");
                if (name.isNotEmpty())
                {
                    processor.getPresetManager().saveUserPreset (name);
                    refreshPresetCombo();
                    refreshPresetLabel();
                }
            }
        };
        p->addAndMakeVisible (std::move (saveBtn));  // transfers ownership
    }
}

} // namespace succo
