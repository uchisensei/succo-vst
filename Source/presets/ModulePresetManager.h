#pragma once

// ---------------------------------------------------------------------------
// ModulePresetManager - per-module preset save/load (true "suite" workflow).
//
// Each module (eq_, comp_, drive_, reverb_, etc.) has its own preset library
// stored in ~/Documents/SUCCO/ModulePresets/<module>/.
//
// To save: walk the APVTS state ValueTree, filter params by the module prefix,
// snapshot them into a ValueTree, serialize to XML on disk.
// To load: deserialize XML, apply params back to APVTS via getParameter().
// A/B snapshots are in-memory only (no disk file).
// ---------------------------------------------------------------------------

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <memory>
#include <map>

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

        // Snapshot matching params from APVTS state ValueTree
        const juce::String prefix = moduleName.toLowerCase() + "_";
        juce::ValueTree snapshot ("MODULE_PARAMS");

        auto state = apvts.copyState();
        for (const auto& child : state)
        {
            const juce::String paramID = child.getType().toString();
            if (paramID.startsWith (prefix))
            {
                const float value = (float) child.getProperty (juce::Identifier ("value"), 0.0f);
                snapshot.setProperty (juce::Identifier (paramID), value, nullptr);
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
        const juce::String prefix = moduleName.toLowerCase() + "_";

        // Iterate the snapshot and apply matching params
        for (const auto& child : snapshot)
        {
            const juce::String paramID = child.getType().toString();
            if (paramID.startsWith (prefix))
            {
                const float value = (float) child.getProperty (juce::Identifier ("value"), 0.0f);
                if (auto* param = apvts.getParameter (paramID))
                    param->setValueNotifyingHost (value);
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
        const juce::String prefix = moduleName.toLowerCase() + "_";
        juce::ValueTree snapshot ("AB_STATE");

        auto state = apvts.copyState();
        for (const auto& child : state)
        {
            const juce::String paramID = child.getType().toString();
            if (paramID.startsWith (prefix))
            {
                const float value = (float) child.getProperty (juce::Identifier ("value"), 0.0f);
                snapshot.setProperty (juce::Identifier (paramID), value, nullptr);
            }
        }
        abStates[moduleName + "_" + juce::String::charToString (slot)] = snapshot;
    }

    void recallABState (const juce::String& moduleName, char slot)
    {
        const juce::String key = moduleName + "_" + juce::String::charToString (slot);
        if (! abStates.count (key))
            return;
        auto snapshot = abStates.at (key).createCopy();
        const juce::String prefix = moduleName.toLowerCase() + "_";

        for (const auto& child : snapshot)
        {
            const juce::String paramID = child.getType().toString();
            if (paramID.startsWith (prefix))
            {
                const float value = (float) child.getProperty (juce::Identifier ("value"), 0.0f);
                if (auto* param = apvts.getParameter (paramID))
                    param->setValueNotifyingHost (value);
            }
        }
    }

    void copyAToB (const juce::String& moduleName)
    {
        const juce::String keyA = moduleName + "_A";
        const juce::String keyB = moduleName + "_B";
        if (abStates.count (keyA))
            abStates[keyB] = abStates.at (keyA).createCopy();
    }

    // Get root folder (for the delete button in UI)
    juce::File getModuleFolder (const juce::String& moduleName) const
    {
        return rootFolder.getChildFile (moduleName);
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File rootFolder;
    std::map<juce::String, juce::ValueTree> abStates;
};

} // namespace succo
