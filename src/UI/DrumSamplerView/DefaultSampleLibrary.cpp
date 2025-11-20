#include <juce_core/juce_core.h>
#include "../../DrumSamplerEngine/DefaultSampleLibrary.h"
#include "BinaryData.h"

using namespace juce;

namespace DefaultSampleLibrary
{
    //==============================================================================
    File installRoot()
    {
        auto root = File::getSpecialLocation (File::userApplicationDataDirectory)
                        .getChildFile ("GrooveKit")
                        .getChildFile ("Samples");

        root.createDirectory();
        return root;
    }

    //------------------------------------------------------------------------------
    // Internal helpers (file/path utilities)

    static String baseNameFrom (String s)
    {
        s = s.replaceCharacter ('\\', '/');

        if (const int idx = s.lastIndexOfChar ('/'); idx >= 0)
            return s.substring ((size_t) idx + 1);

        return s;
    }

    static String categorizeRelative (const String& originalPathOrName)
    {
        const auto lower = originalPathOrName.toLowerCase();

        // If original path already contains "/samples/", preserve relative structure.
        if (const int idx = lower.lastIndexOf ("/samples/"); idx >= 0)
        {
            auto rel = originalPathOrName.substring (
                (size_t) idx + String ("/samples/").length());

            return rel.replaceCharacter ('\\', '/');  // e.g. "Kicks/foo.wav"
        }

        const auto name = baseNameFrom (originalPathOrName);

        if (lower.contains ("kick")  || lower.contains ("_bd") || lower.contains (" bd")
            || lower.contains ("bd"))
            return "Kicks/"   + name;

        if (lower.contains ("snare") || lower.startsWithIgnoreCase ("sd ")
            || lower.contains ("_sd"))
            return "Snares/"  + name;

        if (lower.contains ("hihat") || lower.contains ("hi-hat")
            || lower.contains (" hh"))
            return "HiHats/"  + name;

        if (lower.contains ("tom"))
            return "Toms/"    + name;

        // Catch-all for anything else
        return "UserImports/" + name;
    }

    static File ensureSample (const String& relPath, const void* data, int dataSize)
    {
        auto dst = installRoot().getChildFile (relPath);
        dst.getParentDirectory().createDirectory();

        if (! dst.existsAsFile() || dst.getSize() != (int64) dataSize)
        {
            (void) dst.replaceWithData (data, (size_t) dataSize);
        }

        return dst;
    }

    static void migrateFlatFiles()
    {
        auto root = installRoot();

        // Non-recursive search for wavs sitting directly under the root.
        Array<File> flat;
        root.findChildFiles (flat, File::findFiles, false, "*.wav;*.WAV");

        for (auto& f : flat)
        {
            const auto rel  = categorizeRelative (f.getFileName());
            auto dest = root.getChildFile (rel);
            dest.getParentDirectory().createDirectory();

            if (dest.getFullPathName() == f.getFullPathName())
                continue;

            if (! dest.existsAsFile())
                f.moveFileTo (dest);
            else
                f.deleteFile();
        }
    }

    //==============================================================================
    void ensureInstalled()
    {
        int copied = 0;

        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const char* resNameC = BinaryData::namedResourceList[i];
            if (resNameC == nullptr)
                continue;

            int dataSize = 0;
            const void* data = BinaryData::getNamedResource (resNameC, dataSize);
            if (data == nullptr || dataSize <= 0)
                continue;

            const String original (BinaryData::getNamedResourceOriginalFilename (resNameC));

            if (! original.endsWithIgnoreCase (".wav"))
                continue;

            const auto rel = categorizeRelative (original);
            ensureSample (rel, data, dataSize);
            ++copied;
        }

        // Move any old-style flat files into categorized folders.
        migrateFlatFiles();

        // `copied` is kept for potential future logging or debugging.
        (void) copied;
    }

    Array<File> listAll()
    {
        Array<File> files;
        auto root = installRoot();

        root.findChildFiles (files, File::findFiles, true, "*.wav");
        return files;
    }
}
