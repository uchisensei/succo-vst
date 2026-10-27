#pragma once

// ---------------------------------------------------------------------------
// ParameterManager - builds and exposes the JUCE AudioProcessorValueTreeState
// (APVTS) for SUCCO. Each module contributes a set of atomic parameters.
// All parameter IDs are namespaced (e.g., "gate_thresh", "comp_ratio").
//
// UI labels are English; tooltips are Italian (bilingual per user request).
// ---------------------------------------------------------------------------

#include <juce_audio_processors/juce_audio_processors.h>
#include "../dsp/ChainProcessor.h"

namespace succo
{

namespace ParamIDs
{
    // Master
    constexpr const char* inputGain    = "master_in";
    constexpr const char* outputGain   = "master_out";
    constexpr const char* mix           = "master_mix";
    constexpr const char* punch        = "master_punch";
    constexpr const char* activeModule  = "ui_module";

    // Tune
    constexpr const char* tuneEnable     = "tune_enable";
    constexpr const char* tuneSemitones = "tune_semi";
    constexpr const char* tuneAmount    = "tune_amount";
    // Gate
    constexpr const char* gateEnable    = "gate_enable";
    constexpr const char* gateThresh    = "gate_thresh";
    constexpr const char* gateRange     = "gate_range";
    constexpr const char* gateAttack    = "gate_attack";
    constexpr const char* gateHold      = "gate_hold";
    constexpr const char* gateRelease   = "gate_release";
    // EQ
    constexpr const char* eqEnable       = "eq_enable";
    constexpr const char* eqHpfOn        = "eq_hpf_on";
    constexpr const char* eqHpfFreq      = "eq_hpf_freq";
    constexpr const char* eqLowShelfFreq = "eq_ls_freq";
    constexpr const char* eqLowShelfGain = "eq_ls_gain";
    constexpr const char* eqLowMidFreq   = "eq_lm_freq";
    constexpr const char* eqLowMidQ      = "eq_lm_q";
    constexpr const char* eqLowMidGain    = "eq_lm_gain";
    constexpr const char* eqHighMidFreq  = "eq_hm_freq";
    constexpr const char* eqHighMidQ      = "eq_hm_q";
    constexpr const char* eqHighMidGain   = "eq_hm_gain";
    constexpr const char* eqHighShelfFreq = "eq_hs_freq";
    constexpr const char* eqHighShelfGain = "eq_hs_gain";
    constexpr const char* eqLpfOn         = "eq_lpf_on";
    constexpr const char* eqLpfFreq       = "eq_lpf_freq";
    // Comp
    constexpr const char* compEnable    = "comp_enable";
    constexpr const char* compThresh    = "comp_thresh";
    constexpr const char* compRatio     = "comp_ratio";
    constexpr const char* compAttack    = "comp_attack";
    constexpr const char* compRelease   = "comp_release";
    constexpr const char* compKnee      = "comp_knee";
    constexpr const char* compMakeup    = "comp_makeup";
    // DeEss
    constexpr const char* deEssEnable   = "deess_enable";
    constexpr const char* deEssFreq     = "deess_freq";
    constexpr const char* deEssThresh   = "deess_thresh";
    constexpr const char* deEssAmount   = "deess_amount";
    // Drive
    constexpr const char* driveEnable   = "drive_enable";
    constexpr const char* driveDrive    = "drive_drive";
    constexpr const char* driveShape    = "drive_shape";
    constexpr const char* driveTone     = "drive_tone";
    constexpr const char* driveOutput   = "drive_output";
    // Doubler
    constexpr const char* doublerEnable = "doubler_enable";
    constexpr const char* doublerAmount = "doubler_amount";
    constexpr const char* doublerTime   = "doubler_time";
    constexpr const char* doublerSpread = "doubler_spread";
    // Chorus
    constexpr const char* chorusEnable  = "chorus_enable";
    constexpr const char* chorusRate    = "chorus_rate";
    constexpr const char* chorusDepth   = "chorus_depth";
    constexpr const char* chorusVoices  = "chorus_voices";
    constexpr const char* chorusMix      = "chorus_mix";
    // Delay
    constexpr const char* delayEnable   = "delay_enable";
    constexpr const char* delayTime     = "delay_time";
    constexpr const char* delayFeedback = "delay_fb";
    constexpr const char* delayPingPong = "delay_pp";
    constexpr const char* delayMix      = "delay_mix";
    // Reverb
    constexpr const char* reverbEnable   = "reverb_enable";
    constexpr const char* reverbSize     = "reverb_size";
    constexpr const char* reverbDecay    = "reverb_decay";
    constexpr const char* reverbDamp     = "reverb_damp";
    constexpr const char* reverbPredelay = "reverb_predelay";
    constexpr const char* reverbLowCut   = "reverb_lowcut";
    constexpr const char* reverbMix      = "reverb_mix";
    // Limiter
    constexpr const char* limitEnable    = "limit_enable";
    constexpr const char* limitCeiling  = "limit_ceiling";
    constexpr const char* limitRelease  = "limit_release";
    constexpr const char* limitTruePeak  = "limit_tp";
}

class ParameterManager
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout buildLayout()
    {
        using namespace juce;
        using ID = ParamIDs;
        AudioProcessorValueTreeState::ParameterLayout layout;

        // ---- Master ----
        layout.add (std::make_unique<AudioParameterFloat>(
            ParameterID { ID::inputGain },   "Input",    NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB").withStringFromValueFunction ([](float v, int){ return juce::String (v, 1) + " dB"; })));
        layout.add (std::make_unique<AudioParameterFloat>(
            ParameterID { ID::outputGain },  "Output",   NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB").withStringFromValueFunction ([](float v, int){ return juce::String (v, 1) + " dB"; })));
        layout.add (std::make_unique<AudioParameterFloat>(
            ParameterID { ID::mix },         "Mix",      NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v, int){ return juce::String (int (v * 100.0f)) + " %"; })));
        layout.add (std::make_unique<AudioParameterFloat>(
            ParameterID { ID::punch },       "Punch",    NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v, int){ return juce::String (int (v * 100.0f)) + " %"; })));
        layout.add (std::make_unique<AudioParameterChoice>(
            ParameterID { ID::activeModule }, "Active Module",
            StringArray { "HOME", "TUNE", "GATE", "EQ", "COMP", "DE-ESS", "DRIVE", "DOUBLE", "CHORUS", "DELAY", "VERB", "LIMIT", "PRESETS" },
            0));

        // ---- Tune ----
        addChoice (layout, ID::tuneEnable, "Tune Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::tuneSemitones, "Tune Semitones", -12.0f, 12.0f, 0.01f, 0.0f, [](float v){ return juce::String (v, 2) + " st"; });
        addFloat  (layout, ID::tuneAmount, "Tune Amount", 0.0f, 1.0f, 0.001f, 0.0f, [](float v){ return juce::String (int (v*100)) + " %"; });

        // ---- Gate ----
        addChoice (layout, ID::gateEnable, "Gate Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::gateThresh, "Gate Threshold", -80.0f, 0.0f, 0.1f, -60.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::gateRange, "Gate Range", 0.0f, 80.0f, 0.1f, 40.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::gateAttack, "Gate Attack", 0.1f, 100.0f, 0.1f, 1.0f, [](float v){ return juce::String (v, 2) + " ms"; });
        addFloat  (layout, ID::gateHold, "Gate Hold", 0.0f, 500.0f, 0.1f, 20.0f, [](float v){ return juce::String (v, 1) + " ms"; });
        addFloat  (layout, ID::gateRelease, "Gate Release", 1.0f, 1000.0f, 0.1f, 80.0f, [](float v){ return juce::String (v, 1) + " ms"; });

        // ---- EQ ----
        addChoice (layout, ID::eqEnable, "EQ Enable", { "Off", "On" }, 1);
        addChoice (layout, ID::eqHpfOn, "HPF On", { "Off", "On" }, 0);
        addFloat  (layout, ID::eqHpfFreq, "HPF Frequency", 20.0f, 1000.0f, 0.1f, 80.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::eqLowShelfFreq, "Low Shelf Freq", 30.0f, 1000.0f, 0.1f, 120.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::eqLowShelfGain, "Low Shelf Gain", -18.0f, 18.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::eqLowMidFreq, "Low-Mid Freq", 100.0f, 2000.0f, 0.1f, 350.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::eqLowMidQ, "Low-Mid Q", 0.1f, 6.0f, 0.01f, 0.9f, [](float v){ return juce::String (v, 2); });
        addFloat  (layout, ID::eqLowMidGain, "Low-Mid Gain", -18.0f, 18.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::eqHighMidFreq, "High-Mid Freq", 1000.0f, 12000.0f, 0.1f, 3000.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::eqHighMidQ, "High-Mid Q", 0.1f, 6.0f, 0.01f, 0.9f, [](float v){ return juce::String (v, 2); });
        addFloat  (layout, ID::eqHighMidGain, "High-Mid Gain", -18.0f, 18.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::eqHighShelfFreq, "High Shelf Freq", 2000.0f, 20000.0f, 0.1f, 8000.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::eqHighShelfGain, "High Shelf Gain", -18.0f, 18.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addChoice (layout, ID::eqLpfOn, "LPF On", { "Off", "On" }, 0);
        addFloat  (layout, ID::eqLpfFreq, "LPF Frequency", 1000.0f, 20000.0f, 0.1f, 16000.0f, [](float v){ return juce::String (int (v)) + " Hz"; });

        // ---- Comp ----
        addChoice (layout, ID::compEnable, "Comp Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::compThresh, "Comp Threshold", -60.0f, 0.0f, 0.1f, -20.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::compRatio, "Comp Ratio", 1.0f, 20.0f, 0.01f, 3.0f, [](float v){ return juce::String (v, 2) + " : 1"; });
        addFloat  (layout, ID::compAttack, "Comp Attack", 0.1f, 200.0f, 0.1f, 5.0f, [](float v){ return juce::String (v, 2) + " ms"; });
        addFloat  (layout, ID::compRelease, "Comp Release", 1.0f, 1000.0f, 0.1f, 100.0f, [](float v){ return juce::String (v, 1) + " ms"; });
        addFloat  (layout, ID::compKnee, "Comp Knee", 0.0f, 24.0f, 0.1f, 6.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::compMakeup, "Comp Makeup", -24.0f, 24.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });

        // ---- De-Ess ----
        addChoice (layout, ID::deEssEnable, "De-Ess Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::deEssFreq, "De-Ess Freq", 2000.0f, 12000.0f, 0.1f, 6500.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::deEssThresh, "De-Ess Threshold", -60.0f, 0.0f, 0.1f, -28.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::deEssAmount, "De-Ess Amount", 0.0f, 1.0f, 0.001f, 0.5f, [](float v){ return juce::String (int (v*100)) + " %"; });

        // ---- Drive ----
        addChoice (layout, ID::driveEnable, "Drive Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::driveDrive, "Drive Drive", 0.0f, 24.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });
        addFloat  (layout, ID::driveShape, "Drive Shape", 0.0f, 1.0f, 0.001f, 0.5f, [](float v){ return juce::String (int (v*100)) + " %"; });
        addFloat  (layout, ID::driveTone, "Drive Tone", -1.0f, 1.0f, 0.001f, 0.0f, [](float v){ return juce::String (v, 2); });
        addFloat  (layout, ID::driveOutput, "Drive Output", -24.0f, 12.0f, 0.1f, 0.0f, [](float v){ return juce::String (v, 1) + " dB"; });

        // ---- Doubler ----
        addChoice (layout, ID::doublerEnable, "Doubler Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::doublerAmount, "Doubler Amount", 0.0f, 1.0f, 0.001f, 0.4f, [](float v){ return juce::String (int (v*100)) + " %"; });
        addFloat  (layout, ID::doublerTime, "Doubler Time", 5.0f, 40.0f, 0.1f, 18.0f, [](float v){ return juce::String (v, 1) + " ms"; });
        addFloat  (layout, ID::doublerSpread, "Doubler Spread", 0.0f, 1.0f, 0.001f, 0.7f, [](float v){ return juce::String (int (v*100)) + " %"; });

        // ---- Chorus ----
        addChoice (layout, ID::chorusEnable, "Chorus Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::chorusRate, "Chorus Rate", 0.05f, 5.0f, 0.001f, 0.6f, [](float v){ return juce::String (v, 3) + " Hz"; });
        addFloat  (layout, ID::chorusDepth, "Chorus Depth", 0.0f, 10.0f, 0.01f, 2.5f, [](float v){ return juce::String (v, 2) + " ms"; });
        addChoice (layout, ID::chorusVoices, "Chorus Voices", { "1", "2", "3", "4" }, 1);
        addFloat  (layout, ID::chorusMix, "Chorus Mix", 0.0f, 1.0f, 0.001f, 0.3f, [](float v){ return juce::String (int (v*100)) + " %"; });

        // ---- Delay ----
        addChoice (layout, ID::delayEnable, "Delay Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::delayTime, "Delay Time", 1.0f, 2000.0f, 0.1f, 250.0f, [](float v){ return juce::String (int (v)) + " ms"; });
        addFloat  (layout, ID::delayFeedback, "Delay Feedback", 0.0f, 0.95f, 0.001f, 0.35f, [](float v){ return juce::String (int (v*100)) + " %"; });
        addChoice (layout, ID::delayPingPong, "Delay Ping-Pong", { "Off", "On" }, 0);
        addFloat  (layout, ID::delayMix, "Delay Mix", 0.0f, 1.0f, 0.001f, 0.25f, [](float v){ return juce::String (int (v*100)) + " %"; });

        // ---- Reverb ----
        addChoice (layout, ID::reverbEnable, "Reverb Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::reverbSize, "Reverb Size", 0.0f, 1.0f, 0.001f, 0.5f, [](float v){ return juce::String (int (v*100)) + " %"; });
        addFloat  (layout, ID::reverbDecay, "Reverb Decay", 0.1f, 5.0f, 0.01f, 1.2f, [](float v){ return juce::String (v, 2) + " s"; });
        addFloat  (layout, ID::reverbDamp, "Reverb Damp", 500.0f, 20000.0f, 0.1f, 4000.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::reverbPredelay, "Reverb Predelay", 0.0f, 250.0f, 0.1f, 0.0f, [](float v){ return juce::String (int (v)) + " ms"; });
        addFloat  (layout, ID::reverbLowCut, "Reverb Low Cut", 20.0f, 1000.0f, 0.1f, 100.0f, [](float v){ return juce::String (int (v)) + " Hz"; });
        addFloat  (layout, ID::reverbMix, "Reverb Mix", 0.0f, 1.0f, 0.001f, 0.25f, [](float v){ return juce::String (int (v*100)) + " %"; });

        // ---- Limiter ----
        addChoice (layout, ID::limitEnable, "Limiter Enable", { "Off", "On" }, 1);
        addFloat  (layout, ID::limitCeiling, "Limiter Ceiling", -12.0f, 0.0f, 0.01f, -0.3f, [](float v){ return juce::String (v, 2) + " dB"; });
        addFloat  (layout, ID::limitRelease, "Limiter Release", 1.0f, 500.0f, 0.1f, 50.0f, [](float v){ return juce::String (v, 1) + " ms"; });
        addChoice (layout, ID::limitTruePeak, "Limiter True Peak", { "Off", "On" }, 1);

        return layout;
    }

    // Pull the entire chain state from the APVTS into a ChainParams struct
    static ChainParams toChainParams (juce::AudioProcessorValueTreeState& apvts)
    {
        using namespace ParamIDs;
        ChainParams cp;

        cp.inputGainDb  = apvts.getRawParameterValue (inputGain)->load();
        cp.outputGainDb = apvts.getRawParameterValue (outputGain)->load();
        cp.mix          = apvts.getRawParameterValue (mix)->load();
        cp.punch        = apvts.getRawParameterValue (punch)->load();

        cp.tuneBypass   = apvts.getRawParameterValue (tuneEnable)->load() < 0.5f;
        cp.gateBypass   = apvts.getRawParameterValue (gateEnable)->load() < 0.5f;
        cp.eqBypass     = apvts.getRawParameterValue (eqEnable)->load() < 0.5f;
        cp.compBypass   = apvts.getRawParameterValue (compEnable)->load() < 0.5f;
        cp.deEssBypass  = apvts.getRawParameterValue (deEssEnable)->load() < 0.5f;
        cp.driveBypass  = apvts.getRawParameterValue (driveEnable)->load() < 0.5f;
        cp.doubleBypass = apvts.getRawParameterValue (doublerEnable)->load() < 0.5f;
        cp.chorusBypass = apvts.getRawParameterValue (chorusEnable)->load() < 0.5f;
        cp.delayBypass  = apvts.getRawParameterValue (delayEnable)->load() < 0.5f;
        cp.reverbBypass = apvts.getRawParameterValue (reverbEnable)->load() < 0.5f;
        cp.limitBypass  = apvts.getRawParameterValue (limitEnable)->load() < 0.5f;

        cp.tune.enabled       = !cp.tuneBypass;
        cp.tune.semitones     = apvts.getRawParameterValue (tuneSemitones)->load();
        cp.tune.amount        = apvts.getRawParameterValue (tuneAmount)->load();

        cp.gate.enabled       = !cp.gateBypass;
        cp.gate.thresholdDb   = apvts.getRawParameterValue (gateThresh)->load();
        cp.gate.rangeDb       = apvts.getRawParameterValue (gateRange)->load();
        cp.gate.attackMs      = apvts.getRawParameterValue (gateAttack)->load();
        cp.gate.holdMs        = apvts.getRawParameterValue (gateHold)->load();
        cp.gate.releaseMs     = apvts.getRawParameterValue (gateRelease)->load();

        cp.eq.enabled         = !cp.eqBypass;
        cp.eq.hpfOn           = apvts.getRawParameterValue (eqHpfOn)->load() > 0.5f;
        cp.eq.hpfFreq         = apvts.getRawParameterValue (eqHpfFreq)->load();
        cp.eq.lowShelfFreq    = apvts.getRawParameterValue (eqLowShelfFreq)->load();
        cp.eq.lowShelfGain    = apvts.getRawParameterValue (eqLowShelfGain)->load();
        cp.eq.lowMidFreq      = apvts.getRawParameterValue (eqLowMidFreq)->load();
        cp.eq.lowMidQ         = apvts.getRawParameterValue (eqLowMidQ)->load();
        cp.eq.lowMidGain      = apvts.getRawParameterValue (eqLowMidGain)->load();
        cp.eq.highMidFreq     = apvts.getRawParameterValue (eqHighMidFreq)->load();
        cp.eq.highMidQ        = apvts.getRawParameterValue (eqHighMidQ)->load();
        cp.eq.highMidGain     = apvts.getRawParameterValue (eqHighMidGain)->load();
        cp.eq.highShelfFreq   = apvts.getRawParameterValue (eqHighShelfFreq)->load();
        cp.eq.highShelfGain   = apvts.getRawParameterValue (eqHighShelfGain)->load();
        cp.eq.lpfOn           = apvts.getRawParameterValue (eqLpfOn)->load() > 0.5f;
        cp.eq.lpfFreq         = apvts.getRawParameterValue (eqLpfFreq)->load();

        cp.comp.enabled       = !cp.compBypass;
        cp.comp.thresholdDb   = apvts.getRawParameterValue (compThresh)->load();
        cp.comp.ratio         = apvts.getRawParameterValue (compRatio)->load();
        cp.comp.attackMs      = apvts.getRawParameterValue (compAttack)->load();
        cp.comp.releaseMs     = apvts.getRawParameterValue (compRelease)->load();
        cp.comp.kneeDb        = apvts.getRawParameterValue (compKnee)->load();
        cp.comp.makeupDb      = apvts.getRawParameterValue (compMakeup)->load();
        cp.comp.mix           = 1.0f;

        cp.deEss.enabled      = !cp.deEssBypass;
        cp.deEss.freq         = apvts.getRawParameterValue (deEssFreq)->load();
        cp.deEss.thresholdDb  = apvts.getRawParameterValue (deEssThresh)->load();
        cp.deEss.amount       = apvts.getRawParameterValue (deEssAmount)->load();

        cp.drive.enabled      = !cp.driveBypass;
        cp.drive.driveDb      = apvts.getRawParameterValue (driveDrive)->load();
        cp.drive.shape        = apvts.getRawParameterValue (driveShape)->load();
        cp.drive.tone         = apvts.getRawParameterValue (driveTone)->load();
        cp.drive.outputDb     = apvts.getRawParameterValue (driveOutput)->load();
        cp.drive.oversample   = 2;

        cp.doubler.enabled    = !cp.doubleBypass;
        cp.doubler.amount     = apvts.getRawParameterValue (doublerAmount)->load();
        cp.doubler.timeMs    = apvts.getRawParameterValue (doublerTime)->load();
        cp.doubler.spread    = apvts.getRawParameterValue (doublerSpread)->load();

        cp.chorus.enabled     = !cp.chorusBypass;
        cp.chorus.rate        = apvts.getRawParameterValue (chorusRate)->load();
        cp.chorus.depth       = apvts.getRawParameterValue (chorusDepth)->load();
        cp.chorus.voices      = (int) apvts.getRawParameterValue (chorusVoices)->load() + 1;
        cp.chorus.mix         = apvts.getRawParameterValue (chorusMix)->load();
        cp.chorus.feedback    = 0.0f;

        cp.delay.enabled      = !cp.delayBypass;
        cp.delay.timeMs      = apvts.getRawParameterValue (delayTime)->load();
        cp.delay.feedback     = apvts.getRawParameterValue (delayFeedback)->load();
        cp.delay.pingPong     = apvts.getRawParameterValue (delayPingPong)->load() > 0.5f;
        cp.delay.mix          = apvts.getRawParameterValue (delayMix)->load();
        cp.delay.hpFreq       = 200.0f;

        cp.reverb.enabled     = !cp.reverbBypass;
        cp.reverb.size        = apvts.getRawParameterValue (reverbSize)->load();
        cp.reverb.decay       = apvts.getRawParameterValue (reverbDecay)->load();
        cp.reverb.damp        = apvts.getRawParameterValue (reverbDamp)->load();
        cp.reverb.predelayMs  = apvts.getRawParameterValue (reverbPredelay)->load();
        cp.reverb.lowCut      = apvts.getRawParameterValue (reverbLowCut)->load();
        cp.reverb.mix         = apvts.getRawParameterValue (reverbMix)->load();

        cp.limiter.enabled    = !cp.limitBypass;
        cp.limiter.ceilingDb  = apvts.getRawParameterValue (limitCeiling)->load();
        cp.limiter.releaseMs  = apvts.getRawParameterValue (limitRelease)->load();
        cp.limiter.truePeak   = apvts.getRawParameterValue (limitTruePeak)->load() > 0.5f;

        return cp;
    }

private:
    using Layout = juce::AudioProcessorValueTreeState::ParameterLayout;

    static void addFloat (Layout& layout, const char* id, const juce::String& name,
                          float minv, float maxv, float step, float def,
                          std::function<juce::String (float)> valueToString)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id }, name,
            juce::NormalisableRange<float> (minv, maxv, step), def,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([valueToString](float v, int){ return valueToString (v); })
                .withLabel ("")));
    }

    static void addChoice (Layout& layout, const char* id, const juce::String& name,
                           std::initializer_list<const char*> options, int def)
    {
        juce::StringArray arr;
        for (auto o : options) arr.add (juce::String (o));
        layout.add (std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { id }, name, arr, def));
    }
};

} // namespace succo
