#pragma once

// ---------------------------------------------------------------------------
// ModulePresetManager - per-module preset save/load (true "suite" workflow).
//
// Each module (eq_, comp_, drive_, reverb_, etc.) has its own preset library
// stored in ~/Documents/SUCCO/ModulePresets/<module>/.
//
// To save: filter APVTS params by the module prefix (e.g. "eq_"), snapshot
// them into a ValueTree, serialize to XML on disk.
//
// To load: deserialize XML, apply params back to APVTS.
//
// A/B snapshots are in-memory only (no disk file).
// ---------------------------------------------------------------------------

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <memory>

namespace succo
{

class ModulePresetManager
{
public:
    ModulePresetManager (juce::AudioProcessorValueTreeState& apvtsRef)
        : apvts (apvtsRef)
    {
        rootFolder = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                        .getChildFile ("SUCCO").getChildFile ("ModulePresets");
        rootFolder.createDirectory();
    }

    // Save current state of a module's params to disk
    bool saveModulePreset (const juce::String& moduleName, const juce::String& presetName)
    {
        auto folder = rootFolder.getChildFile (moduleName);
        folder.createDirectory();

        juce::XmlElement xml ("SUCCO_MODULE_PRESET");
        xml.setAttribute ("module", moduleName);
        xml.setAttribute ("name", presetName);
        xml.setAttribute ("version", "2.0");

        // Snapshot matching params
        juce::ValueTree snapshot ("MODULE_PARAMS");
        auto prefix = moduleName.lowercase() + "_";
        auto state = apvts.copyState();
        for (auto& child : state)
        {
            const auto propName = child.getProperty (juce::Identifier ("id"), juce::String()).toString();
            // The id property holds the parameter ID
            const auto propID = child.getType().toString();
            if (propID.startsWith (prefix))
                snapshot.setProperty (juce::Identifier (propID), child [juce::Identifier ("value")], nullptr);
        }

        // Fallback approach: iterate APVTS parameters directly
        // (the above may not work depending on JUCE version's ValueTree layout)
        if (snapshot.getNumProperties() == 0)
        {
            const auto& params = apvts.getParameters();
            for (auto* param : params)
            {
                auto pid = param->getParameterID();
                if (pid.startsWith (prefix))
                {
                    snapshot.setProperty (juce::Identifier (pid),
                                          param->getValue(),  // normalized 0..1
                                          nullptr);
                }
            }
        }

        std::unique_ptr<juce::XmlElement> snapXml (snapshot.createXml());
        xml.addChildElement (snapXml.release());

        const auto file = folder.getChildFile (presetName + ".xml");
        const bool ok = xml.writeTo (file);
        return ok;
    }

    // Load a module preset from disk, apply params
    bool loadModulePreset (const juce::String& moduleName, const juce::String& presetName)
    {
        auto file = rootFolder.getChildFile (moduleName).getChildFile (presetName + ".xml");
        if (! file.existsAsFile())
            return false;

        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
        if (xml == nullptr || ! xml->hasTagName ("SUCCO_MODULE_PRESET"))
            return false;

        auto* snapXml = xml->getChildByName ("MODULE_PARAMS");
        if (snapXml == nullptr)
            return false;

        auto snapshot = juce::ValueTree::fromXml (*snapXml);
        auto prefix = moduleName.lowercase() + "_";
        const auto& params = apvts.getParameters();
        for (auto* param : params)
        {
            auto pid = param->getParameterID();
            if (pid.startsWith (prefix) && snapshot.hasProperty (juce::Identifier (pid)))
            {
                const float norm = (float) snapshot [juce::Identifier (pid)];
                param->setValueNotifyingHost (norm);
            }
        }
        return true;
    }

    // List available presets for a module
    juce::StringArray getModulePresets (const juce::String& moduleName) const
    {
        juce::StringArray list;
        auto folder = rootFolder.getChildFile (moduleName);
        if (! folder.exists())
            return list;

        auto files = folder.findChildFiles (juce::File::findFiles, false, "*.xml");
        for (auto& f : files)
            list.add (f.getFileNameWithoutExtension());
        return list;
    }

    // --- A/B snapshot system (in-memory only) ---
    void storeABState (const juce::String& moduleName, char slot)
    {
        auto prefix = moduleName.lowercase() + "_";
        juce::ValueTree snapshot ("AB_STATE");
        const auto& params = apvts.getParameters();
        for (auto* param : params)
        {
            auto pid = param->getParameterID();
            if (pid.startsWith (prefix))
                snapshot.setProperty (juce::Identifier (pid), param->getValue(), nullptr);
        }
        abStates[moduleName + "_" + juce::String::charToString (slot)] = snapshot;
    }

    void recallABState (const juce::String& moduleName, char slot)
    {
        auto key = moduleName + "_" + juce::String::charToString (slot);
        if (! abStates.count (key))
            return;
        auto snapshot = abStates.at (key);
        auto prefix = moduleName.lowercase() + "_";
        const auto& params = apvts.getParameters();
        for (auto* param : params)
        {
            auto pid = param->getParameterID();
            if (pid.startsWith (prefix) && snapshot.hasProperty (juce::Identifier (pid)))
            {
                const float norm = (float) snapshot [juce::Identifier (pid)];
                param->setValueNotifyingHost (norm);
            }
        }
    }

    void copyAToB (const juce::String& moduleName)
    {
        auto keyA = moduleName + "_A";
        auto keyB = moduleName + "_B";
        if (abStates.count (keyA))
            abStates[keyB] = abStates.at (keyA).createCopy();
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File rootFolder;
    std::map<juce::String, juce::ValueTree> abStates;
};

} // namespace succo
