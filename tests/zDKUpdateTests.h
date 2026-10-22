#pragma once
#include "zDKUpdateMetadata.h"

void zDKTestUpdates()
{
    zDKTestExpect(
        zDKUpdateParseVersion("v3.11.0") > zDKUpdateParseVersion("3.9.99"),
        "Update versions compare numerically"
    );
    for (const std::string zDKTestVersion :
         {"", "3.11", "3.11.0-beta", "03.11.0", "3.11.0/evil", "v", "9999999999999.1.0"})
    {
        bool zDKTestRejected = false;
        try
        {
            zDKUpdateParseVersion(zDKTestVersion);
        }
        catch (const std::exception&)
        {
            zDKTestRejected = true;
        }
        zDKTestExpect(zDKTestRejected, "Malformed or unstable update version is rejected");
    }
    const zDKUpdateJson zDKTestRelease = {
        {"tag_name", "v3.11.0"},
        {"draft", false},
        {"prerelease", false},
        {"body", "Release notes"},
        {"assets",
         zDKUpdateJson::array(
             {{{"name", "DokkanDefenseCalculator.exe"},
               {"browser_download_url",
                std::string("https://github.com/") + zDKUpdateRepository +
                    "/releases/download/v3.11.0/DokkanDefenseCalculator.exe"},
               {"digest", "sha256:" + std::string(64, 'a')},
               {"size", 12345}}}
         )}
    };
    const auto zDKTestParsed = zDKUpdateParseRelease(zDKTestRelease.dump());
    zDKTestExpect(
        zDKTestParsed.zDKUpdateVersion == "3.11.0" && zDKTestParsed.zDKUpdateSize == 12345 &&
            zDKTestParsed.zDKUpdateSha256 == std::string(64, 'a'),
        "Published release metadata parses"
    );
    for (int zDKTestCase = 0; zDKTestCase < 10; ++zDKTestCase)
    {
        auto zDKTestInvalid = zDKTestRelease;
        switch (zDKTestCase)
        {
            case 0:
                zDKTestInvalid["draft"] = true;
                break;
            case 1:
                zDKTestInvalid["prerelease"] = true;
                break;
            case 2:
                zDKTestInvalid["assets"][0]["digest"] = nullptr;
                break;
            case 3:
                zDKTestInvalid["assets"][0]["digest"] = "sha256:" + std::string(64, 'x');
                break;
            case 4:
                zDKTestInvalid["assets"][0]["size"] = 0;
                break;
            case 5:
                zDKTestInvalid["assets"][0]["size"] = 300000000;
                break;
            case 6:
                zDKTestInvalid["assets"][0]["browser_download_url"] = "https://evil.example/app.exe";
                break;
            case 7:
                zDKTestInvalid["assets"][0]["name"] = "other.exe";
                break;
            case 8:
                zDKTestInvalid["assets"].push_back(zDKTestRelease["assets"][0]);
                break;
            case 9:
                zDKTestInvalid["tag_name"] = "v3.11.0-beta";
                break;
        }
        bool zDKTestRejected = false;
        try
        {
            zDKUpdateParseRelease(zDKTestInvalid.dump());
        }
        catch (const std::exception&)
        {
            zDKTestRejected = true;
        }
        zDKTestExpect(zDKTestRejected, "Unsafe or incomplete release metadata is rejected");
    }
}
