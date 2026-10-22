#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include "zDKUpdater.h"
#include <iostream>

namespace zDK
{
void zDKTestUpdateRequire(bool zDKTestCondition, const char* zDKTestMessage)
{
    if (!zDKTestCondition)
        throw std::runtime_error(zDKTestMessage);
}

DWORD zDKTestRunInstaller(
    const std::filesystem::path& zDKTestExecutable, const std::filesystem::path& zDKTestConfig
)
{
    std::wstring zDKTestCommand =
        L"\"" + zDKTestExecutable.wstring() + L"\" --apply-update \"" + zDKTestConfig.wstring() + L"\"";
    STARTUPINFOW zDKTestStartup{sizeof(zDKTestStartup)};
    PROCESS_INFORMATION zDKTestProcess{};
    zDKTestUpdateRequire(
        CreateProcessW(
            zDKTestExecutable.c_str(),
            zDKTestCommand.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &zDKTestStartup,
            &zDKTestProcess
        ) != FALSE,
        "Installer starts"
    );
    CloseHandle(zDKTestProcess.hThread);
    const auto zDKTestWait = WaitForSingleObject(zDKTestProcess.hProcess, 15000);
    DWORD zDKTestExit = 1;
    GetExitCodeProcess(zDKTestProcess.hProcess, &zDKTestExit);
    if (zDKTestWait != WAIT_OBJECT_0)
        TerminateProcess(zDKTestProcess.hProcess, 2);
    CloseHandle(zDKTestProcess.hProcess);
    zDKTestUpdateRequire(zDKTestWait == WAIT_OBJECT_0, "Installer finishes within the test deadline");
    return zDKTestExit;
}

int zDKTestRunUpdater(int zDKTestCount, wchar_t** zDKTestArguments)
{
    if (zDKTestCount == 3 && std::wstring(zDKTestArguments[1]) == L"--apply-update")
        return zDKUpdateApply(zDKTestArguments[2], true);
    const auto zDKTestSelf = zDKUpdateCurrentExecutable();
    if (zDKTestSelf.filename() == L"InstalledFixture.exe")
    {
        std::ofstream(zDKTestSelf.parent_path() / L"launched.txt") << "updated";
        return 0;
    }
    try
    {
        if (zDKTestCount == 2 && std::wstring(zDKTestArguments[1]) == L"--verify-release")
        {
            const auto zDKTestRelease = zDKUpdateCheck();
            const auto zDKTestDownload = zDKUpdateDownload(zDKTestRelease);
            std::cout << "Verified public release " << zDKTestRelease.zDKUpdateVersion << ": " << zDKTestDownload << '\n';
            return 0;
        }
        zDKTestUpdateRequire(
            zDKUpdateHash({'a', 'b', 'c'}) ==
                "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
            "SHA-256 known vector"
        );
        zDKTestUpdateRequire(
            zDKUpdateHash({}) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
            "SHA-256 empty vector"
        );
        const auto zDKTestRoot =
            zDKTestSelf.parent_path() / (L"update-tests-" + std::to_wstring(GetCurrentProcessId()));
        const auto zDKTestStage = zDKTestRoot / L"staged";
        const auto zDKTestTarget = zDKTestRoot / L"target" / L"InstalledFixture.exe";
        std::filesystem::create_directories(zDKTestStage);
        std::filesystem::create_directories(zDKTestTarget.parent_path());
        const auto zDKTestStagedExe = zDKTestStage / L"UpdaterFixture.exe";
        std::filesystem::copy_file(zDKTestSelf, zDKTestStagedExe);
        std::ofstream(zDKTestTarget, std::ios::binary) << "previous version";
        std::ifstream zDKTestInput(zDKTestSelf, std::ios::binary);
        const std::vector<unsigned char> zDKTestBytes((std::istreambuf_iterator<char>(zDKTestInput)), {});
        const auto zDKTestConfig = zDKTestStage / L"install.json";
        zDKUpdateJson zDKTestDocument = {
            {"target", zDKTestTarget.u8string()},
            {"pid", 0},
            {"size", zDKTestBytes.size()},
            {"sha256", std::string(64, '0')}
        };
        std::ofstream(zDKTestConfig) << zDKTestDocument.dump();
        zDKTestUpdateRequire(
            zDKTestRunInstaller(zDKTestStagedExe, zDKTestConfig) == 1, "Tampered staged digest is rejected"
        );
        zDKTestUpdateRequire(
            std::filesystem::file_size(zDKTestTarget) == 16,
            "Rejected update leaves the original executable intact"
        );
        zDKTestDocument["sha256"] = zDKUpdateHash(zDKTestBytes);
        std::ofstream(zDKTestConfig) << zDKTestDocument.dump();
        zDKTestUpdateRequire(
            zDKTestRunInstaller(zDKTestStagedExe, zDKTestConfig) == 0,
            "Verified installer replaces the isolated fixture"
        );
        for (int zDKTestAttempt = 0;
             zDKTestAttempt < 40 && !std::filesystem::exists(zDKTestTarget.parent_path() / L"launched.txt");
             ++zDKTestAttempt)
            Sleep(50);
        zDKTestUpdateRequire(
            std::filesystem::exists(zDKTestTarget.parent_path() / L"launched.txt"),
            "Updated fixture restarts successfully"
        );
        zDKTestUpdateRequire(
            !std::filesystem::exists(zDKTestConfig), "Successful install removes its configuration"
        );
        std::cout << "Updater hashing, tamper rejection, replacement and restart checks passed.\n";
        return 0;
    }
    catch (const std::exception& zDKTestError)
    {
        std::cerr << "FAIL: " << zDKTestError.what() << '\n';
        return 1;
    }
}
} // namespace zDK

int wmain(int zDKTestCount, wchar_t** zDKTestArguments)
{
    return zDK::zDKTestRunUpdater(zDKTestCount, zDKTestArguments);
}
