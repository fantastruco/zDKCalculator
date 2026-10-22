#pragma once
#include <algorithm>
#include <charconv>
#include <cmath>
#include <string>
#include <string_view>

namespace zDKUI
{
inline std::string zDKUIRemoveNumberGrouping(std::string_view zDKUIText)
{
    std::string zDKUIRaw;
    for (char zDKUICharacter : zDKUIText)
        if (zDKUICharacter != ',')
            zDKUIRaw += zDKUICharacter;
    return zDKUIRaw;
}

// Preserve the typed mantissa, decimal suffix and exponent, including incomplete
// input such as "-" or "1.". Only the integer part receives thousands separators.
inline std::string zDKUIGroupNumberText(std::string_view zDKUIText)
{
    std::string zDKUIRaw = zDKUIRemoveNumberGrouping(zDKUIText);
    const size_t zDKUIStart = !zDKUIRaw.empty() && (zDKUIRaw[0] == '-' || zDKUIRaw[0] == '+') ? 1 : 0;
    size_t zDKUIEnd = zDKUIStart;
    while (zDKUIEnd < zDKUIRaw.size() && zDKUIRaw[zDKUIEnd] >= '0' && zDKUIRaw[zDKUIEnd] <= '9')
        ++zDKUIEnd;
    for (size_t zDKUIPosition = zDKUIEnd; zDKUIPosition > zDKUIStart + 3;)
    {
        zDKUIPosition -= 3;
        zDKUIRaw.insert(zDKUIPosition, 1, ',');
    }
    return zDKUIRaw;
}

inline int zDKUINumberRawPosition(std::string_view zDKUIText, int zDKUIPosition)
{
    int zDKUICount = 0;
    const int zDKUIEnd = std::clamp(zDKUIPosition, 0, int(zDKUIText.size()));
    for (int zDKUIIndex = 0; zDKUIIndex < zDKUIEnd; ++zDKUIIndex)
        if (zDKUIText[zDKUIIndex] != ',')
            ++zDKUICount;
    return zDKUICount;
}

inline int zDKUINumberGroupedPosition(std::string_view zDKUIText, int zDKUIRawPosition)
{
    if (zDKUIRawPosition <= 0)
        return 0;
    int zDKUICount = 0;
    for (int zDKUIIndex = 0; zDKUIIndex < int(zDKUIText.size()); ++zDKUIIndex)
        if (zDKUIText[zDKUIIndex] != ',' && ++zDKUICount == zDKUIRawPosition)
            return zDKUIIndex + 1;
    return int(zDKUIText.size());
}

inline bool zDKUIParseNumber(std::string_view zDKUIText, double& zDKUIValue)
{
    const std::string zDKUIRaw = zDKUIRemoveNumberGrouping(zDKUIText);
    if (zDKUIRaw.empty())
    {
        zDKUIValue = 0;
        return true;
    }
    const char* zDKUIFirst = zDKUIRaw.data() + (zDKUIRaw[0] == '+' ? 1 : 0);
    const char* zDKUILast = zDKUIRaw.data() + zDKUIRaw.size();
    double zDKUIParsed = 0;
    const auto zDKUIResult = std::from_chars(zDKUIFirst, zDKUILast, zDKUIParsed);
    if (zDKUIResult.ec != std::errc{} || zDKUIResult.ptr != zDKUILast || !std::isfinite(zDKUIParsed))
        return false;
    zDKUIValue = zDKUIParsed;
    return true;
}

inline bool zDKUIParseInteger(std::string_view zDKUIText, int& zDKUIValue)
{
    const std::string zDKUIRaw = zDKUIRemoveNumberGrouping(zDKUIText);
    if (zDKUIRaw.empty())
    {
        zDKUIValue = 0;
        return true;
    }
    const char* zDKUIFirst = zDKUIRaw.data() + (zDKUIRaw[0] == '+' ? 1 : 0);
    const char* zDKUILast = zDKUIRaw.data() + zDKUIRaw.size();
    int zDKUIParsed = 0;
    const auto zDKUIResult = std::from_chars(zDKUIFirst, zDKUILast, zDKUIParsed);
    if (zDKUIResult.ec != std::errc{} || zDKUIResult.ptr != zDKUILast)
        return false;
    zDKUIValue = zDKUIParsed;
    return true;
}
} // namespace zDKUI
