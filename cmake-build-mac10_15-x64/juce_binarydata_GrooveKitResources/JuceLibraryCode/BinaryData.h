/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   HH_MPCVB_Fat1_wav;
    const int            HH_MPCVB_Fat1_wavSize = 25624;

    extern const char*   HH_MPCVB_Fat2_wav;
    const int            HH_MPCVB_Fat2_wavSize = 25690;

    extern const char*   HH_MPCVB_Fat3_wav;
    const int            HH_MPCVB_Fat3_wavSize = 25546;

    extern const char*   HH_MPCVB_Fat4_wav;
    const int            HH_MPCVB_Fat4_wavSize = 23758;

    extern const char*   BD_MPCVB_Fat001_wav;
    const int            BD_MPCVB_Fat001_wavSize = 177010;

    extern const char*   BD_MPCVB_Fat002_wav;
    const int            BD_MPCVB_Fat002_wavSize = 178768;

    extern const char*   BD_MPCVB_Fat003_wav;
    const int            BD_MPCVB_Fat003_wavSize = 175504;

    extern const char*   BD_MPCVB_Fat004_wav;
    const int            BD_MPCVB_Fat004_wavSize = 194096;

    extern const char*   BD_MPCVB_Fat005_wav;
    const int            BD_MPCVB_Fat005_wavSize = 86250;

    extern const char*   BD_MPCVB_FatSat_001_wav;
    const int            BD_MPCVB_FatSat_001_wavSize = 173374;

    extern const char*   BD_MF_Valve11_wav;
    const int            BD_MF_Valve11_wavSize = 38508;

    extern const char*   SD_MPCVB_Fat001_wav;
    const int            SD_MPCVB_Fat001_wavSize = 26572;

    extern const char*   SD_MPCVB_Fat002_wav;
    const int            SD_MPCVB_Fat002_wavSize = 30352;

    extern const char*   SD_MPCVB_Fat003_wav;
    const int            SD_MPCVB_Fat003_wavSize = 30406;

    extern const char*   SD_MPCVB_Fat004_wav;
    const int            SD_MPCVB_Fat004_wavSize = 31258;

    extern const char*   SD_MPCVB_Fat005_wav;
    const int            SD_MPCVB_Fat005_wavSize = 28198;

    extern const char*   Tom_MPCVB_Fat001_wav;
    const int            Tom_MPCVB_Fat001_wavSize = 230752;

    extern const char*   Tom_MPCVB_Fat002_wav;
    const int            Tom_MPCVB_Fat002_wavSize = 188596;

    extern const char*   Tom_MPCVB_Fat003_wav;
    const int            Tom_MPCVB_Fat003_wavSize = 162724;

    extern const char*   Tom_MPCVB_Fat004_wav;
    const int            Tom_MPCVB_Fat004_wavSize = 184646;

    extern const char*   Tom_MPCVB_Fat005_wav;
    const int            Tom_MPCVB_Fat005_wavSize = 151568;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 21;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
