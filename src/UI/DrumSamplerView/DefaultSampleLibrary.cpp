// DefaultSampleLibrary.cpp
#include <juce_core/juce_core.h>
#include "../../DrumSamplerEngine/DefaultSampleLibrary.h"
#include "BinaryData.h"

using namespace juce;

namespace DefaultSampleLibrary
{
    File installRoot()
    {
        auto root = File::getSpecialLocation(File::userApplicationDataDirectory)
                        .getChildFile("GrooveKit").getChildFile("Samples");
        root.createDirectory();
        DBG("[DefaultSamples] installRoot = " << root.getFullPathName()
            << "  exists=" << (int) root.isDirectory());
        return root;
    }

    static String baseNameFrom (String s)
    {
        s = s.replaceCharacter('\\', '/');
        if (const int idx = s.lastIndexOfChar ('/'); idx >= 0)
            return s.substring ((size_t) idx + 1);
        return s;
    }

    static String categorizeRelative (const String& originalPathOrName)
    {
        const auto lower = originalPathOrName.toLowerCase();

        if (const int idx = lower.lastIndexOf ("/samples/"); idx >= 0)
        {
            auto rel = originalPathOrName.substring ((size_t) idx + String("/samples/").length());
            return rel.replaceCharacter ('\\', '/');  // e.g. "Kicks/foo.wav"
        }

        const auto name = baseNameFrom (originalPathOrName);

        if (lower.contains ("kick")  || lower.contains ("_bd") || lower.contains (" bd")
            || lower.contains ("bd"))                     return "Kicks/"   + name;
        if (lower.contains ("snare") || lower.startsWithIgnoreCase ("sd ")
            || lower.contains ("_sd"))                   return "Snares/"  + name;
        if (lower.contains ("hihat") || lower.contains ("hi-hat")
            || lower.contains (" hh"))                   return "HiHats/"  + name;
        if (lower.contains ("tom"))                      return "Toms/"    + name;

        return "UserImports/" + name; // catch-all
    }

    static File ensureSample (const String& relPath, const void* data, int dataSize)
    {
        auto dst = installRoot().getChildFile (relPath);
        dst.getParentDirectory().createDirectory();

        if (! dst.existsAsFile() || dst.getSize() != (int64) dataSize)
        {
            const bool ok = dst.replaceWithData (data, (size_t) dataSize);
            if (! ok)
                DBG ("[DefaultSamples] ERROR writing: " << dst.getFullPathName());
        }
        return dst;
    }

    static void migrateFlatFiles()
    {
        auto root = installRoot();
        Array<File> flat;
        root.findChildFiles (flat, File::findFiles, false, "*.wav;*.WAV"); // non-recursive
        int moved = 0;

        for (auto& f : flat)
        {
            const auto rel  = categorizeRelative (f.getFileName());
            auto dest = root.getChildFile (rel);
            dest.getParentDirectory().createDirectory();

            if (dest.getFullPathName() == f.getFullPathName())
                continue;

            DBG ("  [DefaultSamples] migrate: " << f.getFileName() << " -> " << rel);
            if (! dest.existsAsFile())  f.moveFileTo (dest);
            else                         f.deleteFile();
            ++moved;
        }
        DBG ("[DefaultSamples] migrated " << moved << " flat file(s)");
    }

    void ensureInstalled()
    {
        DBG ("[DefaultSamples] namedResourceListSize=" << BinaryData::namedResourceListSize);

        int copied = 0;
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const char* resNameC = BinaryData::namedResourceList[i];
            if (! resNameC) continue;

            int dataSize = 0;
            const void* data = BinaryData::getNamedResource (resNameC, dataSize);
            if (! data || dataSize <= 0) continue;

            const String original (BinaryData::getNamedResourceOriginalFilename (resNameC));

            if (! original.endsWithIgnoreCase (".wav"))
                continue;

            const auto rel = categorizeRelative (original);
            DBG ("  [DefaultSamples] copying: " << original << " -> " << rel);
            ensureSample (rel, data, dataSize);
            ++copied;
        }

        migrateFlatFiles();

        DBG ("[DefaultSamples] installed " << copied << " wav(s) to "
             << installRoot().getFullPathName());
    }

    Array<File> listAll()
    {
        Array<File> files;
        auto root = installRoot();
        root.findChildFiles (files, File::findFiles, true, "*.wav");
        DBG ("[DefaultSamples] listAll: found " << files.size()
             << " file(s) under " << root.getFullPathName());
        for (auto& f : files) DBG ("    - " << f.getFullPathName());
        return files;
    }
}
