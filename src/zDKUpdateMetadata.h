#pragma once
#include "zDKVersion.h"
#include "../third_party/json/json.hpp"
#include <array>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace zDK
{
using zDKUpdateJson = nlohmann::json;

inline std::array<unsigned, 3> zDKUpdateParseVersion(const std::string& zDKUpdateText)
{
    std::array<unsigned, 3> zDKUpdateParts{};
    if (zDKUpdateText.empty())
        throw std::invalid_argument("Invalid release version.");
    size_t zDKUpdatePosition = zDKUpdateText[0] == 'v' ? 1 : 0;
    for (int zDKUpdatePart = 0; zDKUpdatePart < 3; ++zDKUpdatePart)
    {
        const size_t zDKUpdateStart = zDKUpdatePosition;
        while (zDKUpdatePosition < zDKUpdateText.size() &&
               std::isdigit(static_cast<unsigned char>(zDKUpdateText[zDKUpdatePosition])))
        {
            if (zDKUpdateParts[zDKUpdatePart] > 100000)
                throw std::invalid_argument("Invalid release version.");
            zDKUpdateParts[zDKUpdatePart] =
                zDKUpdateParts[zDKUpdatePart] * 10 + unsigned(zDKUpdateText[zDKUpdatePosition++] - '0');
        }
        if (zDKUpdatePosition == zDKUpdateStart ||
            (zDKUpdatePosition - zDKUpdateStart > 1 && zDKUpdateText[zDKUpdateStart] == '0'))
            throw std::invalid_argument("Invalid release version.");
        if (zDKUpdatePart < 2 &&
            (zDKUpdatePosition >= zDKUpdateText.size() || zDKUpdateText[zDKUpdatePosition++] != '.'))
            throw std::invalid_argument("Invalid release version.");
    }
    if (zDKUpdatePosition != zDKUpdateText.size())
        throw std::invalid_argument("Only stable releases are supported.");
    return zDKUpdateParts;
}

struct zDKUpdateRelease
{
    std::string zDKUpdateVersion, zDKUpdateUrl, zDKUpdateSha256, zDKUpdateNotes;
    std::uint64_t zDKUpdateSize = 0;
};

inline zDKUpdateRelease zDKUpdateParseRelease(const std::string& zDKUpdateResponse)
{
    const auto zDKUpdateDocument = zDKUpdateJson::parse(zDKUpdateResponse);
    if (zDKUpdateDocument.at("draft").get<bool>() || zDKUpdateDocument.at("prerelease").get<bool>())
        throw std::invalid_argument("This release is not a stable published version.");
    const std::string zDKUpdateTag = zDKUpdateDocument.at("tag_name").get<std::string>();
    zDKUpdateParseVersion(zDKUpdateTag);
    zDKUpdateRelease zDKUpdateResult;
    zDKUpdateResult.zDKUpdateVersion = zDKUpdateTag[0] == 'v' ? zDKUpdateTag.substr(1) : zDKUpdateTag;
    if (zDKUpdateDocument.contains("body") && zDKUpdateDocument["body"].is_string())
        zDKUpdateResult.zDKUpdateNotes = zDKUpdateDocument["body"].get<std::string>().substr(0, 16000);
    for (const auto& zDKUpdateAsset : zDKUpdateDocument.at("assets"))
    {
        if (zDKUpdateAsset.at("name").get<std::string>() != "DokkanDefenseCalculator.exe")
            continue;
        if (!zDKUpdateResult.zDKUpdateUrl.empty())
            throw std::invalid_argument("Ambiguous update assets.");
        zDKUpdateResult.zDKUpdateUrl = zDKUpdateAsset.at("browser_download_url").get<std::string>();
        const std::string zDKUpdateExpected = std::string("https://github.com/") + zDKUpdateRepository +
            "/releases/download/" + zDKUpdateTag + "/DokkanDefenseCalculator.exe";
        if (zDKUpdateResult.zDKUpdateUrl != zDKUpdateExpected)
            throw std::invalid_argument(
                "The update asset does not belong to the configured release repository."
            );
        const auto zDKUpdateDigest = zDKUpdateAsset.at("digest").get<std::string>();
        if (zDKUpdateDigest.size() != 71 || zDKUpdateDigest.substr(0, 7) != "sha256:" ||
            !std::all_of(
                zDKUpdateDigest.begin() + 7,
                zDKUpdateDigest.end(),
                [](unsigned char zDKUpdateChar)
                { return std::isdigit(zDKUpdateChar) || (zDKUpdateChar >= 'a' && zDKUpdateChar <= 'f'); }
            ))
            throw std::invalid_argument("GitHub did not supply a valid SHA-256 digest for this update.");
        zDKUpdateResult.zDKUpdateSha256 = zDKUpdateDigest.substr(7);
        zDKUpdateResult.zDKUpdateSize = zDKUpdateAsset.at("size").get<std::uint64_t>();
        if (zDKUpdateResult.zDKUpdateSize == 0 || zDKUpdateResult.zDKUpdateSize > 256ULL * 1024 * 1024)
            throw std::invalid_argument("The update size is outside the supported range.");
    }
    if (zDKUpdateResult.zDKUpdateUrl.empty())
        throw std::invalid_argument("This release has no Windows executable yet.");
    return zDKUpdateResult;
}
} // namespace zDK
