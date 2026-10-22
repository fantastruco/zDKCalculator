#pragma once
#include "zDKUpdateMetadata.h"
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <future>
#include <memory>
#include <vector>

namespace zDK
{
struct zDKUpdateHttpCloser
{
    void operator()(void* zDKUpdateHandle) const
    {
        if (zDKUpdateHandle)
            WinHttpCloseHandle(zDKUpdateHandle);
    }
};

using zDKUpdateHttpHandle = std::unique_ptr<void, zDKUpdateHttpCloser>;

inline std::vector<unsigned char> zDKUpdateHttpGet(const std::wstring& zDKUpdateUrl, size_t zDKUpdateLimit)
{
    URL_COMPONENTS zDKUpdateParts{};
    zDKUpdateParts.dwStructSize = sizeof(zDKUpdateParts);
    zDKUpdateParts.dwHostNameLength = zDKUpdateParts.dwUrlPathLength = zDKUpdateParts.dwExtraInfoLength =
        DWORD(-1);
    if (!WinHttpCrackUrl(zDKUpdateUrl.c_str(), DWORD(zDKUpdateUrl.size()), 0, &zDKUpdateParts) ||
        zDKUpdateParts.nScheme != INTERNET_SCHEME_HTTPS ||
        zDKUpdateParts.nPort != INTERNET_DEFAULT_HTTPS_PORT)
        throw std::runtime_error("Updates require a valid HTTPS URL.");
    const std::wstring zDKUpdateHost(zDKUpdateParts.lpszHostName, zDKUpdateParts.dwHostNameLength);
    if (zDKUpdateHost != L"api.github.com" && zDKUpdateHost != L"github.com")
        throw std::runtime_error("Unexpected update host.");
    std::wstring zDKUpdatePath(zDKUpdateParts.lpszUrlPath, zDKUpdateParts.dwUrlPathLength);
    if (zDKUpdateParts.dwExtraInfoLength)
        zDKUpdatePath.append(zDKUpdateParts.lpszExtraInfo, zDKUpdateParts.dwExtraInfoLength);
    zDKUpdateHttpHandle zDKUpdateSession(WinHttpOpen(
        L"DokkanDefenseCalculator-Updater/1",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    ));
    if (!zDKUpdateSession)
        throw std::runtime_error("Unable to initialize the update connection.");
    WinHttpSetTimeouts(zDKUpdateSession.get(), 10000, 10000, 15000, 20000);
    zDKUpdateHttpHandle zDKUpdateConnection(
        WinHttpConnect(zDKUpdateSession.get(), zDKUpdateHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0)
    );
    zDKUpdateHttpHandle zDKUpdateRequest(
        zDKUpdateConnection ? WinHttpOpenRequest(
                                  zDKUpdateConnection.get(),
                                  L"GET",
                                  zDKUpdatePath.c_str(),
                                  nullptr,
                                  WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  WINHTTP_FLAG_SECURE
                              )
                            : nullptr
    );
    if (!zDKUpdateRequest)
        throw std::runtime_error("Unable to create the update request.");
    DWORD zDKUpdateRedirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP;
    WinHttpSetOption(
        zDKUpdateRequest.get(),
        WINHTTP_OPTION_REDIRECT_POLICY,
        &zDKUpdateRedirectPolicy,
        sizeof(zDKUpdateRedirectPolicy)
    );
    const wchar_t* zDKUpdateHeaders =
        L"Accept: application/vnd.github+json\r\nX-GitHub-Api-Version: 2022-11-28\r\n";
    if (!WinHttpSendRequest(
            zDKUpdateRequest.get(), zDKUpdateHeaders, DWORD(-1L), WINHTTP_NO_REQUEST_DATA, 0, 0, 0
        ) ||
        !WinHttpReceiveResponse(zDKUpdateRequest.get(), nullptr))
        throw std::runtime_error("Unable to reach GitHub. Check your connection and try again.");
    DWORD zDKUpdateStatus = 0, zDKUpdateStatusSize = sizeof(zDKUpdateStatus);
    if (!WinHttpQueryHeaders(
            zDKUpdateRequest.get(),
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &zDKUpdateStatus,
            &zDKUpdateStatusSize,
            WINHTTP_NO_HEADER_INDEX
        ))
        throw std::runtime_error("Unable to read the GitHub response.");
    if (zDKUpdateStatus == 404)
        throw std::runtime_error("No public release is available yet.");
    if (zDKUpdateStatus == 403 || zDKUpdateStatus == 429)
        throw std::runtime_error("GitHub's request limit was reached. Try again later.");
    if (zDKUpdateStatus != 200)
        throw std::runtime_error("GitHub returned HTTP " + std::to_string(zDKUpdateStatus) + ".");
    std::vector<unsigned char> zDKUpdateBytes;
    const ULONGLONG zDKUpdateStarted = GetTickCount64();
    unsigned char zDKUpdateBuffer[65536];
    for (;;)
    {
        if (GetTickCount64() - zDKUpdateStarted > 120000)
            throw std::runtime_error("The update download timed out. Try again.");
        DWORD zDKUpdateRead = 0;
        if (!WinHttpReadData(
                zDKUpdateRequest.get(), zDKUpdateBuffer, sizeof(zDKUpdateBuffer), &zDKUpdateRead
            ))
            throw std::runtime_error("The update download was interrupted. Try again.");
        if (!zDKUpdateRead)
            break;
        if (zDKUpdateBytes.size() + zDKUpdateRead > zDKUpdateLimit)
            throw std::runtime_error("The server response exceeds the supported update size.");
        zDKUpdateBytes.insert(zDKUpdateBytes.end(), zDKUpdateBuffer, zDKUpdateBuffer + zDKUpdateRead);
    }
    return zDKUpdateBytes;
}

inline std::string zDKUpdateHash(const std::vector<unsigned char>& zDKUpdateBytes)
{
    BCRYPT_ALG_HANDLE zDKUpdateAlgorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&zDKUpdateAlgorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
        throw std::runtime_error("Unable to initialize SHA-256 verification.");
    unsigned char zDKUpdateDigest[32];
    const auto zDKUpdateStatus = BCryptHash(
        zDKUpdateAlgorithm,
        nullptr,
        0,
        const_cast<PUCHAR>(zDKUpdateBytes.data()),
        ULONG(zDKUpdateBytes.size()),
        zDKUpdateDigest,
        sizeof(zDKUpdateDigest)
    );
    BCryptCloseAlgorithmProvider(zDKUpdateAlgorithm, 0);
    if (zDKUpdateStatus < 0)
        throw std::runtime_error("Unable to verify the update digest.");
    constexpr char zDKUpdateHex[] = "0123456789abcdef";
    std::string zDKUpdateHashText;
    for (unsigned char zDKUpdateByte : zDKUpdateDigest)
    {
        zDKUpdateHashText += zDKUpdateHex[zDKUpdateByte >> 4];
        zDKUpdateHashText += zDKUpdateHex[zDKUpdateByte & 15];
    }
    return zDKUpdateHashText;
}

inline std::filesystem::path zDKUpdateCurrentExecutable()
{
    std::wstring zDKUpdateBuffer(32768, L'\0');
    DWORD zDKUpdateLength =
        GetModuleFileNameW(nullptr, zDKUpdateBuffer.data(), DWORD(zDKUpdateBuffer.size()));
    if (!zDKUpdateLength || zDKUpdateLength >= zDKUpdateBuffer.size())
        throw std::runtime_error("Unable to locate the running executable.");
    zDKUpdateBuffer.resize(zDKUpdateLength);
    return zDKUpdateBuffer;
}

inline std::filesystem::path zDKUpdateDownload(const zDKUpdateRelease& zDKUpdateReleaseData)
{
    const auto zDKUpdateBytes = zDKUpdateHttpGet(
        std::wstring(zDKUpdateReleaseData.zDKUpdateUrl.begin(), zDKUpdateReleaseData.zDKUpdateUrl.end()),
        size_t(zDKUpdateReleaseData.zDKUpdateSize)
    );
    if (zDKUpdateBytes.size() != zDKUpdateReleaseData.zDKUpdateSize ||
        zDKUpdateHash(zDKUpdateBytes) != zDKUpdateReleaseData.zDKUpdateSha256)
        throw std::runtime_error("Update verification failed. The current application has not been changed.");
    PWSTR zDKUpdateFolder = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &zDKUpdateFolder)))
        throw std::runtime_error("Unable to locate the update folder.");
    std::filesystem::path zDKUpdateDirectory(zDKUpdateFolder);
    CoTaskMemFree(zDKUpdateFolder);
    zDKUpdateDirectory /= L"DokkanDefenseLab\\updates";
    zDKUpdateDirectory /= zDKUpdateReleaseData.zDKUpdateVersion;
    std::filesystem::create_directories(zDKUpdateDirectory);
    const auto zDKUpdatePath = zDKUpdateDirectory / L"DokkanDefenseCalculator.exe";
    if (std::filesystem::equivalent(zDKUpdatePath.parent_path(), zDKUpdateCurrentExecutable().parent_path()))
        throw std::runtime_error("Move the application outside its temporary update folder before updating.");
    std::ofstream zDKUpdateOutput(zDKUpdatePath, std::ios::binary | std::ios::trunc);
    zDKUpdateOutput.write(
        reinterpret_cast<const char*>(zDKUpdateBytes.data()), std::streamsize(zDKUpdateBytes.size())
    );
    zDKUpdateOutput.close();
    if (!zDKUpdateOutput)
        throw std::runtime_error("Unable to save the downloaded update.");
    return zDKUpdatePath;
}

inline bool zDKUpdateLaunch(
    const std::filesystem::path& zDKUpdateExecutable, const std::wstring& zDKUpdateArguments
)
{
    std::wstring zDKUpdateCommand = L"\"" + zDKUpdateExecutable.wstring() + L"\" " + zDKUpdateArguments;
    STARTUPINFOW zDKUpdateStartup{sizeof(zDKUpdateStartup)};
    PROCESS_INFORMATION zDKUpdateProcess{};
    if (!CreateProcessW(
            zDKUpdateExecutable.c_str(),
            zDKUpdateCommand.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            zDKUpdateExecutable.parent_path().c_str(),
            &zDKUpdateStartup,
            &zDKUpdateProcess
        ))
        return false;
    CloseHandle(zDKUpdateProcess.hThread);
    CloseHandle(zDKUpdateProcess.hProcess);
    return true;
}

inline void zDKUpdateStartInstall(
    const std::filesystem::path& zDKUpdateDownloaded, const zDKUpdateRelease& zDKUpdateReleaseData
)
{
    const auto zDKUpdateTarget = zDKUpdateCurrentExecutable();
    const auto zDKUpdateProbe =
        zDKUpdateTarget.parent_path() / (L".zdk-write-" + std::to_wstring(GetCurrentProcessId()) + L".tmp");
    HANDLE zDKUpdateProbeHandle = CreateFileW(
        zDKUpdateProbe.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
        nullptr
    );
    if (zDKUpdateProbeHandle == INVALID_HANDLE_VALUE)
        throw std::runtime_error(
            "This folder is not writable. Move the app to your own folder or download the update manually."
        );
    CloseHandle(zDKUpdateProbeHandle);
    const auto zDKUpdateConfiguration =
        zDKUpdateDownloaded.parent_path() / (L"install-" + std::to_wstring(GetCurrentProcessId()) + L".json");
    const zDKUpdateJson zDKUpdateDocument = {
        {"target", zDKUpdateTarget.u8string()},
        {"pid", GetCurrentProcessId()},
        {"sha256", zDKUpdateReleaseData.zDKUpdateSha256},
        {"size", zDKUpdateReleaseData.zDKUpdateSize}
    };
    std::ofstream zDKUpdateOutput(zDKUpdateConfiguration, std::ios::binary);
    zDKUpdateOutput << zDKUpdateDocument.dump();
    zDKUpdateOutput.close();
    if (!zDKUpdateOutput ||
        !zDKUpdateLaunch(
            zDKUpdateDownloaded, L"--apply-update \"" + zDKUpdateConfiguration.wstring() + L"\""
        ))
        throw std::runtime_error(
            "Unable to start the update installer. The current application is unchanged."
        );
}

inline int zDKUpdateApply(const std::filesystem::path& zDKUpdateConfiguration, bool zDKUpdateQuiet = false)
{
    std::filesystem::path zDKUpdateBackup, zDKUpdateTarget;
    bool zDKUpdateReplacing = false;
    try
    {
        std::ifstream zDKUpdateInput(zDKUpdateConfiguration, std::ios::binary);
        const auto zDKUpdateDocument = zDKUpdateJson::parse(zDKUpdateInput);
        zDKUpdateInput.close();
        const auto zDKUpdateSelf = zDKUpdateCurrentExecutable();
        zDKUpdateTarget = std::filesystem::u8path(zDKUpdateDocument.at("target").get<std::string>());
        if (!zDKUpdateTarget.is_absolute() || zDKUpdateTarget.extension() != L".exe" ||
            !std::filesystem::is_regular_file(zDKUpdateTarget) ||
            std::filesystem::equivalent(zDKUpdateTarget, zDKUpdateSelf) ||
            zDKUpdateConfiguration.parent_path() != zDKUpdateSelf.parent_path())
            throw std::runtime_error("Invalid update destination.");
        std::ifstream zDKUpdateBinary(zDKUpdateSelf, std::ios::binary);
        const std::vector<unsigned char> zDKUpdateBytes(
            (std::istreambuf_iterator<char>(zDKUpdateBinary)), std::istreambuf_iterator<char>()
        );
        if (zDKUpdateBytes.size() != zDKUpdateDocument.at("size").get<size_t>() ||
            zDKUpdateHash(zDKUpdateBytes) != zDKUpdateDocument.at("sha256").get<std::string>())
            throw std::runtime_error("The staged update no longer matches its verified digest.");
        HANDLE zDKUpdateParent = OpenProcess(SYNCHRONIZE, FALSE, zDKUpdateDocument.at("pid").get<DWORD>());
        if (zDKUpdateParent)
        {
            const DWORD zDKUpdateWait = WaitForSingleObject(zDKUpdateParent, 45000);
            CloseHandle(zDKUpdateParent);
            if (zDKUpdateWait != WAIT_OBJECT_0)
                throw std::runtime_error("Close the previous application before updating.");
        }
        zDKUpdateBackup = zDKUpdateTarget;
        zDKUpdateBackup += L".zdk-backup-" + std::to_wstring(GetTickCount64());
        if (!CopyFileW(zDKUpdateTarget.c_str(), zDKUpdateBackup.c_str(), TRUE))
            throw std::runtime_error("Unable to back up the previous application.");
        zDKUpdateReplacing = true;
        bool zDKUpdateCopied = false;
        for (int zDKUpdateAttempt = 0; zDKUpdateAttempt < 20 && !zDKUpdateCopied; ++zDKUpdateAttempt)
        {
            zDKUpdateCopied = CopyFileW(zDKUpdateSelf.c_str(), zDKUpdateTarget.c_str(), FALSE) != FALSE;
            if (!zDKUpdateCopied)
                Sleep(250);
        }
        if (!zDKUpdateCopied || !zDKUpdateLaunch(zDKUpdateTarget, L""))
            throw std::runtime_error(
                "Unable to replace or restart the application; the previous version will be restored."
            );
        zDKUpdateReplacing = false;
        std::error_code zDKUpdateIgnored;
        std::filesystem::remove(zDKUpdateBackup, zDKUpdateIgnored);
        std::filesystem::remove(zDKUpdateConfiguration, zDKUpdateIgnored);
        return 0;
    }
    catch (const std::exception& zDKUpdateError)
    {
        if (zDKUpdateReplacing)
            CopyFileW(zDKUpdateBackup.c_str(), zDKUpdateTarget.c_str(), FALSE);
        if (!zDKUpdateQuiet)
            MessageBoxA(
                nullptr, zDKUpdateError.what(), "Dokkan Defense Calculator update", MB_OK | MB_ICONERROR
            );
        return 1;
    }
}

inline zDKUpdateRelease zDKUpdateCheck()
{
    const std::string zDKUpdateEndpoint =
        std::string("https://api.github.com/repos/") + zDKUpdateRepository + "/releases/latest";
    const auto zDKUpdateResponse =
        zDKUpdateHttpGet(std::wstring(zDKUpdateEndpoint.begin(), zDKUpdateEndpoint.end()), 2 * 1024 * 1024);
    try
    {
        return zDKUpdateParseRelease(std::string(zDKUpdateResponse.begin(), zDKUpdateResponse.end()));
    }
    catch (const std::exception&)
    {
        throw std::runtime_error(
            "GitHub returned incomplete or unsupported release information. Try again later."
        );
    }
}
} // namespace zDK
