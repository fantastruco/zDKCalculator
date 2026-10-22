#pragma once

// Internal, opt-in interaction checks. Runs against hidden ImGui frames with no session writes.
namespace zDKUI
{
struct zDKTestInteractionChecks
{
    int zDKTestStep = 0, zDKTestTicks = 0, zDKTestChecks = 0;
    bool zDKTestFailed = false, zDKTestDone = false;
    bool zDKTestNativeTopmost = true;
    std::ostringstream zDKTestLog;
    ImVec2 zDKTestDragStart{}, zDKTestDragEnd{};
    RECT zDKTestResizeOriginal{};

    void zDKTestExpect(bool zDKTestConditionPassed, const char* zDKTestMessage)
    {
        ++zDKTestChecks;
        zDKTestLog << (zDKTestConditionPassed ? "PASS: " : "FAIL: ") << zDKTestMessage << '\n';
        if (!zDKTestConditionPassed)
        {
            zDKTestFailed = true;
            zDKTestDone = true;
        }
    }

    bool zDKTestWait(int zDKTestFrames)
    {
        return ++zDKTestTicks >= zDKTestFrames;
    }

    void zDKTestNext()
    {
        ++zDKTestStep;
        zDKTestTicks = 0;
    }

    bool zDKTestClick(const std::string& zDKTestKey)
    {
        auto zDKTestFound = zDKUIControlProbes.find(zDKTestKey);
        if (zDKTestFound == zDKUIControlProbes.end() ||
            zDKTestFound->second.zDKUIFrame < ImGui::GetFrameCount() - 1)
        {
            zDKTestExpect(false, ("Visible control missing: " + zDKTestKey).c_str());
            return false;
        }
        auto& zDKTestInputOutput = ImGui::GetIO();
        zDKTestInputOutput.AddMousePosEvent(
            zDKTestFound->second.zDKUICenter.x, zDKTestFound->second.zDKUICenter.y
        );
        ++zDKTestTicks;
        if (zDKTestTicks == 2)
            zDKTestInputOutput.AddMouseButtonEvent(0, true);
        if (zDKTestTicks == 3)
            zDKTestInputOutput.AddMouseButtonEvent(0, false);
        return zDKTestTicks >= 4;
    }

    bool zDKTestEditNumber(const std::string& zDKTestKey, const char* zDKTestValue)
    {
        auto zDKTestFound = zDKUIControlProbes.find(zDKTestKey);
        if (zDKTestFound == zDKUIControlProbes.end())
        {
            zDKTestExpect(false, ("Input missing: " + zDKTestKey).c_str());
            return false;
        }
        auto& zDKTestInputOutput = ImGui::GetIO();
        zDKTestInputOutput.AddMousePosEvent(
            zDKTestFound->second.zDKUICenter.x, zDKTestFound->second.zDKUICenter.y
        );
        ++zDKTestTicks;
        if (zDKTestTicks == 2)
            zDKTestInputOutput.AddMouseButtonEvent(0, true);
        if (zDKTestTicks == 3)
            zDKTestInputOutput.AddMouseButtonEvent(0, false);
        if (zDKTestTicks == 4)
        {
            zDKTestInputOutput.AddKeyEvent(ImGuiMod_Ctrl, true);
            zDKTestInputOutput.AddKeyEvent(ImGuiKey_A, true);
        }
        if (zDKTestTicks == 5)
        {
            zDKTestInputOutput.AddKeyEvent(ImGuiKey_A, false);
            zDKTestInputOutput.AddKeyEvent(ImGuiMod_Ctrl, false);
        }
        if (zDKTestTicks == 6)
            zDKTestInputOutput.AddInputCharactersUTF8(zDKTestValue);
        if (zDKTestTicks == 7)
            zDKTestInputOutput.AddKeyEvent(ImGuiKey_Enter, true);
        if (zDKTestTicks == 8)
            zDKTestInputOutput.AddKeyEvent(ImGuiKey_Enter, false);
        return zDKTestTicks >= 10;
    }

    bool zDKTestDrag(ImVec2 zDKTestStart, ImVec2 zDKTestEnd)
    {
        if (zDKTestTicks == 0)
        {
            zDKTestDragStart = zDKTestStart;
            zDKTestDragEnd = zDKTestEnd;
        }
        ++zDKTestTicks;
        float zDKTestAmount = std::clamp((zDKTestTicks - 10) / 8.f, 0.f, 1.f);
        auto& zDKTestInputOutput = ImGui::GetIO();
        zDKTestInputOutput.AddMousePosEvent(
            zDKTestDragStart.x + (zDKTestDragEnd.x - zDKTestDragStart.x) * zDKTestAmount,
            zDKTestDragStart.y + (zDKTestDragEnd.y - zDKTestDragStart.y) * zDKTestAmount
        );
        if (zDKTestTicks == 2)
            zDKTestInputOutput.AddMouseButtonEvent(0, true);
        if (zDKTestTicks == 19)
            zDKTestInputOutput.AddMouseButtonEvent(0, false);
        return zDKTestTicks >= 21;
    }

    void zDKTestCheckWindow()
    {
        wchar_t zDKTestTitle[128]{};
        GetWindowTextW(zDKWinApplicationWindow, zDKTestTitle, 128);
        zDKTestExpect(
            std::wstring(zDKTestTitle) == L"Dokkan Defense Calculator",
            "Windows title uses the new application name"
        );
        auto zDKTestStyle = GetWindowLongPtrW(zDKWinApplicationWindow, GWL_STYLE);
        zDKTestExpect(
            (zDKTestStyle & WS_THICKFRAME) && !(zDKTestStyle & WS_MAXIMIZEBOX),
            "Window supports border resizing while retaining portrait-only maximize behavior"
        );
        zDKTestExpect(
            GetClassLongPtrW(zDKWinApplicationWindow, GCLP_HICON) != 0, "Windows application icon is retained"
        );
        auto zDKTestIconSize = [](HICON zDKTestIcon)
        {
            SIZE zDKTestSize{};
            ICONINFO zDKTestInfo{};
            if (GetIconInfo(zDKTestIcon, &zDKTestInfo))
            {
                BITMAP zDKTestBitmap{};
                if (GetObjectW(zDKTestInfo.hbmColor, sizeof(zDKTestBitmap), &zDKTestBitmap))
                    zDKTestSize = {zDKTestBitmap.bmWidth, zDKTestBitmap.bmHeight};
                if (zDKTestInfo.hbmColor)
                    DeleteObject(zDKTestInfo.hbmColor);
                if (zDKTestInfo.hbmMask)
                    DeleteObject(zDKTestInfo.hbmMask);
            }
            return zDKTestSize;
        };
        for (UINT zDKTestDpi : {96U, 120U, 144U, 168U, 192U, 240U, 288U, 336U, 384U, 480U, 576U, 672U, 768U})
        {
            const bool zDKTestLoaded = zDKWinUpdateWindowIcons(zDKWinApplicationWindow, zDKTestDpi);
            const auto zDKTestLarge = zDKTestIconSize(
                reinterpret_cast<HICON>(SendMessageW(zDKWinApplicationWindow, WM_GETICON, ICON_BIG, 0))
            );
            const auto zDKTestSmall = zDKTestIconSize(
                reinterpret_cast<HICON>(SendMessageW(zDKWinApplicationWindow, WM_GETICON, ICON_SMALL, 0))
            );
            zDKTestExpect(
                zDKTestLoaded && zDKTestLarge.cx == GetSystemMetricsForDpi(SM_CXICON, zDKTestDpi) &&
                    zDKTestLarge.cy == GetSystemMetricsForDpi(SM_CYICON, zDKTestDpi) &&
                    zDKTestSmall.cx == GetSystemMetricsForDpi(SM_CXSMICON, zDKTestDpi) &&
                    zDKTestSmall.cy == GetSystemMetricsForDpi(SM_CYSMICON, zDKTestDpi),
                ("Windows small and large icons match DPI " + std::to_string(zDKTestDpi)).c_str()
            );
        }
        zDKWinUpdateWindowIcons(zDKWinApplicationWindow, GetDpiForWindow(zDKWinApplicationWindow));
        RECT zDKTestBefore{}, zDKTestAfter{};
        GetWindowRect(zDKWinApplicationWindow, &zDKTestBefore);
        zDKWinResizeStart = zDKTestBefore;
        const auto zDKTestFrame = zDKWinFrameSize(zDKWinApplicationWindow);
        for (WPARAM zDKTestEdge = WMSZ_LEFT; zDKTestEdge <= WMSZ_BOTTOMRIGHT; ++zDKTestEdge)
        {
            RECT zDKTestBounds = zDKTestBefore;
            if (zDKTestEdge == WMSZ_LEFT || zDKTestEdge == WMSZ_TOPLEFT || zDKTestEdge == WMSZ_BOTTOMLEFT)
                zDKTestBounds.left -= 80;
            else if (zDKTestEdge != WMSZ_TOP && zDKTestEdge != WMSZ_BOTTOM)
                zDKTestBounds.right += 80;
            if (zDKTestEdge == WMSZ_TOP || zDKTestEdge == WMSZ_TOPLEFT || zDKTestEdge == WMSZ_TOPRIGHT)
                zDKTestBounds.top -= 100;
            else if (zDKTestEdge != WMSZ_LEFT && zDKTestEdge != WMSZ_RIGHT)
                zDKTestBounds.bottom += 100;
            RECT zDKTestRequested = zDKTestBounds;
            SendMessageW(
                zDKWinApplicationWindow, WM_SIZING, zDKTestEdge, reinterpret_cast<LPARAM>(&zDKTestBounds)
            );
            const LONG zDKTestWidth = zDKTestBounds.right - zDKTestBounds.left - zDKTestFrame.cx;
            const LONG zDKTestHeight = zDKTestBounds.bottom - zDKTestBounds.top - zDKTestFrame.cy;
            bool zDKTestAnchor = true;
            if (zDKTestEdge == WMSZ_LEFT || zDKTestEdge == WMSZ_TOPLEFT || zDKTestEdge == WMSZ_BOTTOMLEFT)
                zDKTestAnchor = zDKTestBounds.right == zDKTestRequested.right;
            if (zDKTestEdge == WMSZ_RIGHT || zDKTestEdge == WMSZ_TOPRIGHT || zDKTestEdge == WMSZ_BOTTOMRIGHT)
                zDKTestAnchor = zDKTestBounds.left == zDKTestRequested.left;
            if (zDKTestEdge == WMSZ_TOP || zDKTestEdge == WMSZ_TOPLEFT || zDKTestEdge == WMSZ_TOPRIGHT)
                zDKTestAnchor = zDKTestAnchor && zDKTestBounds.bottom == zDKTestRequested.bottom;
            if (zDKTestEdge == WMSZ_BOTTOM || zDKTestEdge == WMSZ_BOTTOMLEFT ||
                zDKTestEdge == WMSZ_BOTTOMRIGHT)
                zDKTestAnchor = zDKTestAnchor && zDKTestBounds.top == zDKTestRequested.top;
            zDKTestExpect(
                zDKTestAnchor && std::abs(zDKTestHeight - zDKTestWidth * 980.0 / 540) <= 1,
                ("Portrait client aspect and opposite anchor survive resize edge " +
                 std::to_string(zDKTestEdge))
                    .c_str()
            );
        }
        RECT zDKTestSmall{0, 0, 20, 20}, zDKTestHuge{0, 0, 30000, 30000};
        zDKWinConstrainResize(zDKWinApplicationWindow, WMSZ_BOTTOMRIGHT, zDKTestSmall);
        zDKWinConstrainResize(zDKWinApplicationWindow, WMSZ_BOTTOMRIGHT, zDKTestHuge);
        const auto zDKTestMinimum = zDKWinPortraitLimit(zDKWinApplicationWindow, true);
        const auto zDKTestMaximum = zDKWinPortraitLimit(zDKWinApplicationWindow, false);
        zDKTestExpect(
            zDKTestSmall.right - zDKTestSmall.left == zDKTestMinimum.cx &&
                zDKTestSmall.bottom - zDKTestSmall.top == zDKTestMinimum.cy,
            "Resizing clamps at a usable portrait minimum"
        );
        zDKTestExpect(
            zDKTestHuge.right - zDKTestHuge.left == zDKTestMaximum.cx &&
                zDKTestHuge.bottom - zDKTestHuge.top == zDKTestMaximum.cy,
            "Resizing keeps the portrait window within the monitor work area"
        );
        SetWindowPos(
            zDKWinApplicationWindow,
            nullptr,
            0,
            0,
            zDKTestMinimum.cx,
            zDKTestMinimum.cy,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
        );
        GetWindowRect(zDKWinApplicationWindow, &zDKTestAfter);
        zDKTestExpect(
            zDKTestAfter.right - zDKTestAfter.left == zDKTestMinimum.cx &&
                zDKTestAfter.bottom - zDKTestAfter.top == zDKTestMinimum.cy,
            "Native window actually accepts the new proportional size"
        );
        SetWindowPos(
            zDKWinApplicationWindow,
            nullptr,
            zDKTestBefore.left,
            zDKTestBefore.top,
            zDKTestBefore.right - zDKTestBefore.left,
            zDKTestBefore.bottom - zDKTestBefore.top,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
        SendMessageW(zDKWinApplicationWindow, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        zDKTestExpect(!IsZoomed(zDKWinApplicationWindow), "Maximize requests are rejected");
    }

    void zDKTestBeforeFrame()
    {
        if (zDKTestDone)
            return;
        switch (zDKTestStep)
        {
            case 0:
                if (zDKTestWait(8))
                {
                    zDKTestCheckWindow();
                    zDKTestExpect(
                        zDKUIControlProbes["Unit"].zDKUICenter.y <
                            zDKUIScrollProbes["unit-stats-panel"].zDKUICenter.y,
                        "Primary Unit/Boss/Info navigation sits above the content"
                    );
                    zDKTestExpect(
                        zDKUIControlProbes["Setup"].zDKUICenter.y >
                            zDKUIScrollProbes["unit-stats-panel"].zDKUICenter.y,
                        "Secondary navigation stays below the content"
                    );
                    zDKTestExpect(
                        zDKUIControlProbes.count("+ Add") == 0,
                        "Unit Stats is separate from the Effects editor"
                    );
                    zDKTestStep = 300;
                    zDKTestTicks = 0;
                }
                break;
            case 300:
                if (zDKTestClick("Setup"))
                    zDKTestNext();
                break;
            case 301:
                if (zDKTestWait(28))
                {
                    zDKTestExpect(zDKUIDisplayedSection == 2, "Unit Setup opens as its own subview");
                    zDKTestExpect(
                        zDKUIControlProbes.count("Round down") == 0,
                        "Mandatory integer rounding has no off switch in Unit Setup"
                    );
                    zDKTestStep = 200;
                    zDKTestTicks = 0;
                }
                break;
            case 200:
                if (zDKTestClick("tdb-choice"))
                    zDKTestNext();
                break;
            case 201:
                if (zDKTestClick("option-tdb-choice-0"))
                    zDKTestNext();
                break;
            case 202:
                if (zDKTestWait(2))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel == 6,
                        "55% selects Type DEF Boost Lv. 6"
                    );
                    zDKTestNext();
                }
                break;
            case 203:
                if (zDKTestClick("tdb-choice"))
                    zDKTestNext();
                break;
            case 204:
                if (zDKTestClick("option-tdb-choice-1"))
                    zDKTestNext();
                break;
            case 205:
                if (zDKTestWait(2))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel == 11,
                        "100% selects Type DEF Boost Lv. 11"
                    );
                    zDKTestNext();
                }
                break;
            case 206:
                if (zDKTestClick("tdb-choice"))
                    zDKTestNext();
                break;
            case 207:
                if (zDKTestClick("option-tdb-choice-2"))
                    zDKTestNext();
                break;
            case 208:
                if (zDKTestWait(2))
                {
                    zDKTestExpect(
                        zDKUIControlProbes.count("Custom Lv.") > 0, "Custom opens a manual level field"
                    );
                    zDKTestNext();
                }
                break;
            case 209:
                if (zDKTestEditNumber("Custom Lv.", "17"))
                    zDKTestNext();
                break;
            case 210:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel == 17,
                        "Custom level can be entered through the input field"
                    );
                    zDKTestStep = 302;
                    zDKTestTicks = 0;
                }
                break;
            case 302:
                if (zDKTestClick("Effects"))
                    zDKTestNext();
                break;
            case 303:
                if (zDKTestWait(28))
                {
                    zDKTestExpect(
                        zDKUIDisplayedSection == 1, "Effects opens from the lower secondary navigation"
                    );
                    zDKTestStep = 1;
                    zDKTestTicks = 0;
                }
                break;
            case 1:
                if (zDKTestClick("+ Add"))
                    zDKTestNext();
                break;
            case 2:
                if (zDKTestTicks == 0)
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIChoosing &&
                            zDKUIEffectModalState.zDKUIWindowSize.x == ImGui::GetIO().DisplaySize.x &&
                            zDKUIEffectModalState.zDKUIWindowSize.y == ImGui::GetIO().DisplaySize.y,
                        "Add opens a full-screen custom effect picker"
                    );
                if (zDKTestClick("add-category-3"))
                    zDKTestNext();
                break;
            case 3:
                if (zDKTestWait(20))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectSuperAttackStack &&
                            zDKUIEffectModalState.zDKUIIndex < 0,
                        "Add opens the SA Stack modal"
                    );
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects.size() == 2, "An uncommitted effect is not added"
                    );
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 70;
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectCount = 3;
                    zDKTestNext();
                }
                break;
            case 4:
                if (zDKTestClick("Cancel"))
                    zDKTestNext();
                break;
            case 5:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects.size() == 2, "Cancel discards the new effect"
                    );
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIProgress == 0, "Modal close animation completes"
                    );
                    zDKTestNext();
                }
                break;
            case 6:
                if (zDKTestClick("+ Add"))
                    zDKTestNext();
                break;
            case 7:
                if (zDKTestClick("add-category-0"))
                    zDKTestNext();
                break;
            case 8:
                if (zDKTestClick("skill-effect"))
                {
                    zDKTestStep = 801;
                    zDKTestTicks = 0;
                }
                break;
            case 801:
                if (zDKTestClick("stat-option-12"))
                    zDKTestNext();
                break;
            case 802:
                if (zDKTestWait(10))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectBasic &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectStat == zDKEffectStat::zDKEffectGuard,
                        "Guard is selected as a skill effect inside Basic Effect"
                    );
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIWindowSize.y == ImGui::GetIO().DisplaySize.y,
                        "Guard editor keeps the same full-screen dimensions"
                    );
                    zDKTestStep = 9;
                    zDKTestTicks = 0;
                }
                break;
            case 9:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 10:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects.size() == 3 &&
                            zDKUICurrentUnit.zDKEffectEffects.back().zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectBasic &&
                            zDKEffectIsGuard(zDKUICurrentUnit.zDKEffectEffects.back()),
                        "Apply adds exactly one guard under Basic Effect"
                    );
                    zDKTestNext();
                }
                break;
            case 11:
                if (zDKTestClick("DEF +200% at start of turn###effect"))
                    zDKTestNext();
                break;
            case 12:
                if (zDKTestWait(20))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIIndex == 0 &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectValue == 200,
                        "Clicking an effect opens its current values"
                    );
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = -1;
                    zDKTestNext();
                }
                break;
            case 13:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 14:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects[0].zDKEffectValue == 200,
                        "Invalid edits leave the saved effect unchanged"
                    );
                    zDKTestExpect(
                        !zDKUIEffectModalState.zDKUIError.empty() && !zDKUIEffectModalState.zDKUIClosing,
                        "Invalid edits remain open with an error"
                    );
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 250;
                    zDKTestNext();
                }
                break;
            case 15:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 16:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects[0].zDKEffectValue == 250 &&
                            zDKUICurrentUnit.zDKEffectEffects.size() == 3,
                        "Valid edits replace one effect without duplicates"
                    );
                    zDKTestNext();
                }
                break;
            case 17:
                if (zDKTestClick("Boss"))
                    zDKTestNext();
                break;
            case 18:
                zDKTestExpect(
                    zDKUIPageProgress > 0 && zDKUIPageProgress < 1, "Tab change enters a gradual transition"
                );
                zDKTestNext();
                break;
            case 19:
                if (zDKTestClick("Info"))
                    zDKTestNext();
                break;
            case 20:
                if (zDKTestWait(28))
                {
                    zDKTestExpect(
                        zDKUIActiveTab == 2 && zDKUIDisplayedTab == 2 && zDKUIPageProgress == 1,
                        "Rapid tab changes settle on the last tab"
                    );
                    zDKTestNext();
                }
                break;
            case 21:
                zDKUIAnimationsEnabled = false;
                if (zDKTestClick("Unit"))
                    zDKTestNext();
                break;
            case 22:
                if (zDKTestWait(2))
                {
                    zDKTestExpect(
                        zDKUIDisplayedTab == 0 && zDKUIPageProgress == 1,
                        "Disabling animations changes tabs immediately"
                    );
                    zDKTestExpect(
                        zDKUIDisplayedSection == 1, "Returning to Unit remembers the Effects subview"
                    );
                    zDKUIAnimationsEnabled = true;
                    zDKTestNext();
                }
                break;
            case 23:
                if (zDKTestClick("Boss"))
                    zDKTestNext();
                break;
            case 24:
                if (zDKTestWait(28))
                {
                    zDKTestStep = 220;
                    zDKTestTicks = 0;
                }
                break;
            case 220:
                if (zDKTestEditNumber("Normal ATK", "1200000"))
                    zDKTestNext();
                break;
            case 221:
                if (zDKTestEditNumber("Super Attack ATK", "4800000"))
                    zDKTestNext();
                break;
            case 222:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 1200000 &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttack == 4800000,
                        "Normal and Super Attack ATK can be entered independently"
                    );
                    zDKTestNext();
                }
                break;
            case 223:
                if (zDKTestEditNumber("Normal ATK", "0"))
                    zDKTestNext();
                break;
            case 224:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 0 &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttack == 4800000,
                        "Zeroing Normal ATK preserves the Super Attack value"
                    );
                    zDKTestStep = 304;
                    zDKTestTicks = 0;
                }
                break;
            case 304:
                if (zDKTestClick("Effects"))
                    zDKTestNext();
                break;
            case 305:
                if (zDKTestWait(28))
                {
                    zDKTestStep = 25;
                    zDKTestTicks = 0;
                }
                break;
            case 25:
                if (zDKTestClick("+ Add"))
                    zDKTestNext();
                break;
            case 26:
                if (zDKTestClick("add-category-2"))
                    zDKTestNext();
                break;
            case 27:
                if (zDKTestWait(24))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIIsBoss &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectStat == zDKEffectStat::zDKEffectAttack,
                        "Enemy effects use the enemy editor"
                    );
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectLastTurn == 3,
                        "Entrance Animation starts with its own turn range"
                    );
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 100;
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectFirstTurn = 3;
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectLastTurn = 0;
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectHealthRule =
                        zDKEffectHealthRule::zDKEffectBelow;
                    zDKTestStep = 100;
                    zDKTestTicks = 0;
                }
                break;
            case 100:
            {
                const auto& zDKTestScroll = zDKUIScrollProbes["effect-form"];
                auto& zDKTestInputOutput = ImGui::GetIO();
                zDKTestInputOutput.AddMousePosEvent(zDKTestScroll.zDKUICenter.x, zDKTestScroll.zDKUICenter.y);
                ++zDKTestTicks;
                if (zDKTestTicks == 2)
                    zDKTestInputOutput.AddMouseWheelEvent(0, -1);
                if (zDKTestTicks >= 6)
                {
                    zDKTestExpect(
                        zDKTestScroll.zDKUIMaximum > 0, "Long modal forms provide a scrollable body"
                    );
                    zDKTestExpect(
                        zDKTestScroll.zDKUIPosition > 0 &&
                            zDKTestScroll.zDKUIPosition < zDKTestScroll.zDKUIDestination,
                        "Mouse-wheel scrolling eases toward its destination"
                    );
                    zDKTestStep = 110;
                    zDKTestTicks = 0;
                }
                break;
            }
            case 110:
            {
                auto zDKTestFrame = zDKUIScrollProbes["effect-form"];
                if (zDKTestDrag(
                        {zDKTestFrame.zDKUIMin.x + 5, zDKTestFrame.zDKUIMax.y - 65},
                        {zDKTestFrame.zDKUIMin.x + 5, zDKTestFrame.zDKUIMin.y + 65}
                    ))
                {
                    zDKTestStep = 111;
                    zDKTestTicks = 0;
                }
                break;
            }
            case 111:
                if (zDKTestWait(6))
                {
                    auto zDKTestFrame = zDKUIScrollProbes["effect-form"];
                    zDKTestExpect(
                        zDKTestFrame.zDKUIPosition > zDKTestFrame.zDKUIMaximum * .8f,
                        "Holding and dragging upward scrolls the full-screen form"
                    );
                    zDKTestNext();
                }
                break;
            case 112:
            {
                auto zDKTestFrame = zDKUIScrollProbes["effect-form"];
                if (zDKTestDrag(
                        {zDKTestFrame.zDKUIMin.x + 5, zDKTestFrame.zDKUIMin.y + 65},
                        {zDKTestFrame.zDKUIMin.x + 5, zDKTestFrame.zDKUIMax.y - 65}
                    ))
                    zDKTestNext();
                break;
            }
            case 113:
                if (zDKTestWait(6))
                {
                    auto zDKTestFrame = zDKUIScrollProbes["effect-form"];
                    zDKTestExpect(
                        zDKTestFrame.zDKUIPosition < zDKTestFrame.zDKUIMaximum * .2f,
                        "Holding and dragging downward scrolls back to the top"
                    );
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectValue == 100 &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectFirstTurn == 3,
                        "Drag scrolling preserves effect inputs"
                    );
                    zDKTestStep = 101;
                    zDKTestTicks = 0;
                }
                break;
            case 101:
            {
                ++zDKTestTicks;
                auto& zDKTestInputOutput = ImGui::GetIO();
                auto zDKTestPoint =
                    zDKUIControlProbes[zDKTestTicks <= 2 ? "scroll-thumb" : "scroll-end"].zDKUICenter;
                zDKTestInputOutput.AddMousePosEvent(zDKTestPoint.x, zDKTestPoint.y);
                if (zDKTestTicks == 2)
                    zDKTestInputOutput.AddMouseButtonEvent(0, true);
                if (zDKTestTicks == 6)
                    zDKTestInputOutput.AddMouseButtonEvent(0, false);
                if (zDKTestTicks >= 9)
                {
                    const auto& zDKTestScroll = zDKUIScrollProbes["effect-form"];
                    zDKTestExpect(
                        zDKTestScroll.zDKUIPosition > zDKTestScroll.zDKUIMaximum * .8f,
                        "The custom scrollbar thumb can be dragged to the bottom"
                    );
                    zDKTestStep = 28;
                    zDKTestTicks = 0;
                }
                break;
            }
            case 28:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 29:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectEffects.size() == 1 &&
                            zDKUICurrentBoss.zDKEffectEffects[0].zDKEffectFirstTurn == 3,
                        "Enemy conditions are saved"
                    );
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects.size() == 3 &&
                            zDKUICurrentUnit.zDKEffectEffects[0].zDKEffectValue == 250,
                        "Enemy edits preserve unit effects"
                    );
                    zDKTestNext();
                }
                break;
            case 30:
                if (zDKTestClick("ATK +100% on Entrance Animation###effect"))
                    zDKTestNext();
                break;
            case 31:
                if (zDKTestWait(20))
                {
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 999;
                    zDKTestNext();
                }
                break;
            case 32:
                ++zDKTestTicks;
                if (zDKTestTicks == 1)
                    ImGui::GetIO().AddKeyEvent(ImGuiKey_Escape, true);
                if (zDKTestTicks == 2)
                    ImGui::GetIO().AddKeyEvent(ImGuiKey_Escape, false);
                if (zDKTestTicks >= 4)
                    zDKTestNext();
                break;
            case 33:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectEffects[0].zDKEffectValue == 100 &&
                            zDKUIEffectModalState.zDKUIProgress == 0,
                        "Escape closes the modal and discards changes to an existing enemy effect"
                    );
                    zDKTestStep = 306;
                    zDKTestTicks = 0;
                }
                break;
            case 306:
                if (zDKTestClick("Skills"))
                    zDKTestNext();
                break;
            case 307:
                if (zDKTestWait(28))
                {
                    zDKTestExpect(
                        zDKUIDisplayedTab == 1 && zDKUIDisplayedSection == 2,
                        "Enemy Skills has a dedicated subview"
                    );
                    zDKTestNext();
                }
                break;
            case 308:
                if (zDKTestClick("Info"))
                    zDKTestNext();
                break;
            case 309:
                if (zDKTestWait(28))
                    zDKTestNext();
                break;
            case 310:
                if (zDKTestClick("Tanking"))
                    zDKTestNext();
                break;
            case 311:
                if (zDKTestWait(28))
                {
                    zDKTestExpect(
                        zDKUIDisplayedTab == 2 && zDKUIDisplayedSection == 1,
                        "Info separates Tanking from Damage"
                    );
                    zDKTestNext();
                }
                break;
            case 312:
                if (zDKTestClick("Calcs"))
                    zDKTestNext();
                break;
            case 313:
                if (zDKTestWait(28))
                {
                    zDKTestExpect(
                        zDKUIDisplayedTab == 2 && zDKUIDisplayedSection == 2 &&
                            zDKUIControlProbes.count("Copy calc") > 0,
                        "Info Calcs contains the calculation report"
                    );
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel == 17 &&
                            zDKUICurrentUnit.zDKEffectEffects.size() == 3 &&
                            zDKUICurrentBoss.zDKEffectEffects.size() == 1,
                        "Subview changes preserve unit and enemy data"
                    );
                    zDKTestStep = 400;
                    zDKTestTicks = 0;
                }
                break;
            case 400:
            {
                auto zDKTestFrame = zDKUIScrollProbes["calcs-panel"];
                if (zDKTestDrag(
                        {zDKTestFrame.zDKUIMin.x + 8, zDKTestFrame.zDKUIMin.y + 65},
                        {zDKTestFrame.zDKUIMin.x + 158, zDKTestFrame.zDKUIMin.y + 65}
                    ))
                    zDKTestNext();
                break;
            }
            case 401:
                if (zDKTestWait(30))
                {
                    zDKTestExpect(
                        zDKUIActiveTab == 2 && zDKUIDisplayedTab == 2 && zDKUIDisplayedSection == 2,
                        "Horizontal content drags preserve the current tab and subview"
                    );
                    zDKTestNext();
                }
                break;
            case 402:
            {
                auto zDKTestPoint = zDKUIControlProbes["Boss"].zDKUICenter;
                if (zDKTestDrag(zDKTestPoint, {zDKTestPoint.x - 150, zDKTestPoint.y}))
                    zDKTestNext();
                break;
            }
            case 403:
                if (zDKTestWait(30))
                {
                    zDKTestExpect(
                        zDKUIActiveTab == 2 && zDKUIDisplayedTab == 2 && zDKUIDisplayedSection == 2,
                        "Dragging across the top navigation does not select a tab"
                    );
                    zDKTestNext();
                }
                break;
            case 404:
            {
                auto zDKTestPoint = zDKUIControlProbes["Tanking"].zDKUICenter;
                if (zDKTestDrag(zDKTestPoint, {zDKTestPoint.x + 150, zDKTestPoint.y}))
                    zDKTestNext();
                break;
            }
            case 405:
                if (zDKTestWait(30))
                {
                    zDKTestExpect(
                        zDKUIActiveTab == 2 && zDKUIDisplayedSection == 2,
                        "Dragging across the lower navigation does not select a subview"
                    );
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel == 17 &&
                            zDKUICurrentUnit.zDKEffectEffects.size() == 3 &&
                            zDKUICurrentBoss.zDKEffectEffects[0].zDKEffectValue == 100,
                        "Drag gestures preserve calculator data"
                    );
                    zDKTestStep = 418;
                    zDKTestTicks = 0;
                }
                break;
            case 418:
                if (zDKTestClick("DEF calc"))
                    zDKTestNext();
                break;
            case 419:
                if (zDKTestWait(28))
                    zDKTestNext();
                break;
            case 420:
                if (zDKTestClick("Guard & DR"))
                    zDKTestNext();
                break;
            case 421:
                if (zDKTestWait(28))
                    zDKTestNext();
                break;
            case 422:
            {
                auto zDKTestFrame = zDKUIScrollProbes["calcs-panel"];
                if (zDKTestDrag(
                        {zDKTestFrame.zDKUIMin.x + 5, zDKTestFrame.zDKUIMax.y - 65},
                        {zDKTestFrame.zDKUIMin.x + 155, zDKTestFrame.zDKUIMin.y + 65}
                    ))
                    zDKTestNext();
                break;
            }
            case 423:
                if (zDKTestWait(6))
                {
                    zDKTestExpect(
                        zDKUIScrollProbes["calcs-panel"].zDKUIPosition > 0,
                        "Diagonal page drags scroll the content vertically"
                    );
                    zDKTestExpect(
                        zDKUIActiveTab == 2 && zDKUIDisplayedTab == 2 && zDKUIDisplayedSection == 2,
                        "Vertical page scrolling preserves the current tab and subview"
                    );
                    zDKTestStep = 406;
                    zDKTestTicks = 0;
                }
                break;
            case 406:
                if (zDKTestClick("Boss"))
                    zDKTestNext();
                break;
            case 407:
                if (zDKTestWait(30))
                    zDKTestNext();
                break;
            case 408:
                if (zDKTestClick("Stats"))
                    zDKTestNext();
                break;
            case 409:
                if (zDKTestWait(30))
                    zDKTestNext();
                break;
            case 410:
            {
                auto zDKTestPoint = zDKUIControlProbes["Super Attack ATK"].zDKUICenter;
                if (zDKTestDrag(zDKTestPoint, {zDKTestPoint.x + 120, zDKTestPoint.y}))
                    zDKTestNext();
                break;
            }
            case 411:
                if (zDKTestWait(5))
                {
                    zDKTestExpect(
                        zDKUIActiveTab == 1 && zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttack == 4800000,
                        "Dragging inside an input does not swipe tabs or change its value"
                    );
                    zDKTestNext();
                }
                break;
            case 412:
                if (zDKTestClick("Effects"))
                    zDKTestNext();
                break;
            case 413:
                if (zDKTestWait(30))
                    zDKTestNext();
                break;
            case 414:
                if (zDKTestClick("+ Add"))
                    zDKTestNext();
                break;
            case 415:
                if (zDKTestWait(20))
                    zDKTestNext();
                break;
            case 416:
                if (zDKTestClick("Cancel"))
                    zDKTestNext();
                break;
            case 417:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectEffects.size() == 1 &&
                            zDKUIEffectModalState.zDKUIProgress == 0,
                        "Cancel closes the full-screen picker without adding an effect"
                    );
                    zDKTestStep = 430;
                    zDKTestTicks = 0;
                }
                break;
            case 430:
                zDKUIActiveTab = 0;
                zDKUISelectedSubviews[0] = 1;
                zDKUIRequestEffect(false, -1, zDKEffectCategory::zDKEffectBasic);
                zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 200;
                zDKTestNext();
                break;
            case 431:
                if (zDKTestWait(28))
                    zDKTestNext();
                break;
            case 432:
                if (zDKTestClick("DEF boost phase"))
                    zDKTestNext();
                break;
            case 433:
                if (zDKTestClick("option-DEF boost phase-9"))
                    zDKTestNext();
                break;
            case 434:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 435:
                if (zDKTestWait(20))
                {
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects.back().zDKEffectGroup ==
                            zDKEffectDefensePhase::zDKEffectWhenReceivingSuper,
                        "Effect modal exposes receiving-Super-Attack phase"
                    );
                    const auto zDKTestEvaluation =
                        zDKEffectEvaluate(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle);
                    zDKTestExpect(
                        zDKTestEvaluation.zDKEffectSuperDefense.zDKCalcSelected >
                            zDKTestEvaluation.zDKEffectNormalDefense.zDKCalcSelected,
                        "Applied receiving-SA effect separates normal and Super DEF"
                    );
                    zDKUIActiveTab = 1;
                    zDKUISelectedSubviews[1] = 0;
                    zDKTestNext();
                }
                break;
            case 436:
                if (zDKTestWait(28))
                    zDKTestNext();
                break;
            case 437:
                if (zDKTestClick("Enemy ATK input"))
                    zDKTestNext();
                break;
            case 438:
                if (zDKTestClick("option-Enemy ATK input-1"))
                    zDKTestNext();
                break;
            case 439:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        !zDKUICurrentBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats,
                        "Boss Stats exposes base-ATK and SA-multiplier mode"
                    );
                    zDKTestNext();
                }
                break;
            case 440:
                if (zDKTestEditNumber("SA multiplier (x)", "3"))
                    zDKTestNext();
                break;
            case 441:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttackMultiplier == 3,
                        "Enemy SA multiplier can be entered through the interface"
                    );
                    zDKTestNext();
                }
                break;
            case 442:
                if (zDKTestClick("Enemy ATK input"))
                    zDKTestNext();
                break;
            case 443:
                if (zDKTestClick("option-Enemy ATK input-0"))
                    zDKTestNext();
                break;
            case 444:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUICurrentBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttack == 4800000,
                        "Returning to direct ATK preserves the independent SA value"
                    );
                    zDKTestStep = 450;
                    zDKTestTicks = 0;
                }
                break;
            case 450:
            {
                auto& zDKTestInputOutput = ImGui::GetIO();
                auto zDKTestPoint = zDKUIControlProbes["Normal ATK"].zDKUICenter;
                zDKTestInputOutput.AddMousePosEvent(zDKTestPoint.x, zDKTestPoint.y);
                ++zDKTestTicks;
                if (zDKTestTicks == 2)
                    zDKTestInputOutput.AddMouseButtonEvent(0, true);
                if (zDKTestTicks == 3)
                    zDKTestInputOutput.AddMouseButtonEvent(0, false);
                if (zDKTestTicks == 4 || zDKTestTicks == 20)
                {
                    zDKTestInputOutput.AddKeyEvent(ImGuiMod_Ctrl, true);
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_A, true);
                }
                if (zDKTestTicks == 5 || zDKTestTicks == 21)
                {
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_A, false);
                    zDKTestInputOutput.AddKeyEvent(ImGuiMod_Ctrl, false);
                }
                if (zDKTestTicks == 6)
                    zDKTestInputOutput.AddInputCharactersUTF8("1");
                if (zDKTestTicks >= 7 && zDKTestTicks <= 12)
                    zDKTestInputOutput.AddInputCharactersUTF8("0");
                if (zDKTestTicks == 10)
                    zDKTestExpect(
                        zDKUIControlProbes["Normal ATK"].zDKUIInputText == "1,000",
                        "Typing the fourth digit inserts a comma before leaving the field"
                    );
                if (zDKTestTicks == 13)
                    zDKTestExpect(
                        zDKUIControlProbes["Normal ATK"].zDKUIInputText == "1,000,000" &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 1000000,
                        "Live million grouping preserves the numeric calculation value"
                    );
                if (zDKTestTicks == 14 || zDKTestTicks == 24)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Home, true);
                if (zDKTestTicks == 15 || zDKTestTicks == 25)
                {
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Home, false);
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_RightArrow, true);
                }
                if (zDKTestTicks == 16 || zDKTestTicks == 26 || zDKTestTicks == 28)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_RightArrow, false);
                if (zDKTestTicks == 17)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Delete, true);
                if (zDKTestTicks == 18)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Delete, false);
                if (zDKTestTicks == 19)
                    zDKTestExpect(
                        zDKUIControlProbes["Normal ATK"].zDKUIInputText == "100,000" &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 100000,
                        "Delete beside a generated comma removes the adjacent digit"
                    );
                if (zDKTestTicks == 22)
                    zDKTestInputOutput.AddInputCharactersUTF8("1,234,567");
                if (zDKTestTicks == 23)
                    zDKTestExpect(
                        zDKUIControlProbes["Normal ATK"].zDKUIInputText == "1,234,567" &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 1234567,
                        "Pasting a grouped number keeps its complete numeric value"
                    );
                if (zDKTestTicks == 27)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_RightArrow, true);
                if (zDKTestTicks == 29)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Backspace, true);
                if (zDKTestTicks == 30)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Backspace, false);
                if (zDKTestTicks == 31)
                    zDKTestExpect(
                        zDKUIControlProbes["Normal ATK"].zDKUIInputText == "234,567" &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 234567,
                        "Backspace beside a generated comma removes the adjacent digit"
                    );
                if (zDKTestTicks == 32)
                {
                    zDKTestInputOutput.AddKeyEvent(ImGuiMod_Ctrl, true);
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Z, true);
                }
                if (zDKTestTicks == 33)
                {
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Z, false);
                    zDKTestInputOutput.AddKeyEvent(ImGuiMod_Ctrl, false);
                }
                if (zDKTestTicks == 34)
                    zDKTestExpect(
                        zDKUIControlProbes["Normal ATK"].zDKUIInputText == "1,234,567" &&
                            zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack == 1234567,
                        "Undo restores the number across automatic separator edits"
                    );
                if (zDKTestTicks == 35)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Enter, true);
                if (zDKTestTicks == 36)
                    zDKTestInputOutput.AddKeyEvent(ImGuiKey_Enter, false);
                if (zDKTestTicks >= 37)
                {
                    zDKTestStep = 500;
                    zDKTestTicks = 0;
                }
                break;
            }
            case 500:
                if (zDKTestClick("Unit"))
                    zDKTestNext();
                break;
            case 501:
                if (zDKTestWait(28))
                    zDKTestNext();
                break;
            case 502:
                if (zDKTestClick("Effects"))
                    zDKTestNext();
                break;
            case 503:
                if (zDKTestWait(28))
                {
                    zDKUICurrentUnit = zDKEffectUnitProfile{};
                    zDKUICurrentUnit.zDKEffectBaseDefense = 1000;
                    auto zDKTestBasic = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
                    zDKTestBasic.zDKEffectValue = 200;
                    zDKUICurrentUnit.zDKEffectEffects.push_back(zDKTestBasic);
                    zDKUICurrentBattle.zDKEffectAfterUnitAttack = true;
                    zDKTestNext();
                }
                break;
            case 504:
                if (zDKTestClick("+ Add"))
                    zDKTestNext();
                break;
            case 505:
                if (zDKTestClick("add-category-5"))
                    zDKTestNext();
                break;
            case 506:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectSupport &&
                            zDKUIControlProbes.count("Support source") != 0,
                        "Ally Support opens an editor with an explicit source phase"
                    );
                    zDKTestNext();
                }
                break;
            case 507:
                if (zDKTestEditNumber("Effect amount (%)", "40"))
                    zDKTestNext();
                break;
            case 508:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 509:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKEffectEvaluate(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle)
                                .zDKEffectResult.zDKCalcDefense.zDKCalcSelected == 3400,
                        "Support entered through the UI adds to the ordinary SoT passive"
                    );
                    zDKTestNext();
                }
                break;
            case 510:
                if (zDKTestClick("effect-row-1"))
                    zDKTestNext();
                break;
            case 511:
                if (zDKTestWait(15))
                    zDKTestNext();
                break;
            case 512:
                if (zDKTestClick("Support source"))
                    zDKTestNext();
                break;
            case 513:
                if (zDKTestClick("option-Support source-2"))
                    zDKTestNext();
                break;
            case 514:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectGroup ==
                                zDKEffectDefensePhase::zDKEffectSuperAttack &&
                            zDKUIControlProbes.count("count") != 0,
                        "SA support exposes a separate number of active raises"
                    );
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 30;
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectCount = 2;
                    zDKTestNext();
                }
                break;
            case 515:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 516:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKEffectEvaluate(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle)
                                .zDKEffectResult.zDKCalcDefense.zDKCalcSelected == 4800,
                        "Editing support to an SA source moves its boost to the SA bracket"
                    );
                    zDKTestNext();
                }
                break;
            case 517:
                if (zDKTestClick("+ Add"))
                    zDKTestNext();
                break;
            case 518:
                if (zDKTestClick("add-category-8"))
                    zDKTestNext();
                break;
            case 519:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                            zDKEffectCategory::zDKEffectUltraSuperAttack,
                        "Ultra Super Attack has its own editor and raise count"
                    );
                    zDKUIEffectModalState.zDKUIDraft.zDKEffectValue = 50;
                    zDKTestNext();
                }
                break;
            case 520:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 521:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKEffectEvaluate(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle)
                                .zDKEffectResult.zDKCalcDefense.zDKCalcSelected == 6300,
                        "SA support and Ultra SA raises combine correctly through the interface"
                    );
                    zDKTestExpect(
                        zDKUIControlProbes["effect-section-1"].zDKUICenter.y <
                            zDKUIControlProbes["effect-section-4"].zDKUICenter.y,
                        "Basic Effect is visibly separated above the Super Attack section"
                    );
                    zDKTestNext();
                }
                break;
            case 522:
                if (zDKTestClick("effect-row-2"))
                    zDKTestNext();
                break;
            case 523:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKUIEffectModalState.zDKUIIndex == 2 &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectUltraSuperAttack,
                        "Editing a grouped Ultra SA row preserves its original effect identity"
                    );
                    zDKTestNext();
                }
                break;
            case 524:
                if (zDKTestClick("skill-effect"))
                    zDKTestNext();
                break;
            case 525:
                if (zDKTestClick("stat-option-12"))
                    zDKTestNext();
                break;
            case 526:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKEffectIsGuard(zDKUIEffectModalState.zDKUIDraft) &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectUltraSuperAttack,
                        "Guard can be selected inside Ultra Super Attack"
                    );
                    zDKTestNext();
                }
                break;
            case 527:
                if (zDKTestClick("Effect type"))
                    zDKTestNext();
                break;
            case 528:
                if (zDKTestClick("option-Effect type-2"))
                    zDKTestNext();
                break;
            case 529:
                if (zDKTestWait(3))
                {
                    zDKTestExpect(
                        zDKEffectIsGuard(zDKUIEffectModalState.zDKUIDraft) &&
                            zDKUIEffectModalState.zDKUIDraft.zDKEffectCategory ==
                                zDKEffectCategory::zDKEffectWhenAttacking,
                        "Changing effect type retains the selected guard skill"
                    );
                    zDKTestNext();
                }
                break;
            case 530:
                if (zDKTestClick("Apply"))
                    zDKTestNext();
                break;
            case 531:
                if (zDKTestWait(15))
                {
                    zDKTestExpect(
                        zDKEffectEvaluate(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle)
                            .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
                        "When-attacking guard entered through the UI activates after attacking"
                    );
                    zDKUICurrentBattle.zDKEffectAfterUnitAttack = false;
                    zDKTestExpect(
                        !zDKEffectEvaluate(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle)
                             .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
                        "The same guard is inactive before attacking"
                    );
                    zDKTestStep = 600;
                    zDKTestTicks = 0;
                }
                break;
            case 600:
            {
                GetWindowRect(zDKWinApplicationWindow, &zDKTestResizeOriginal);
                const auto zDKTestFrame = zDKWinFrameSize(zDKWinApplicationWindow);
                const LONG zDKTestWidth = LONG(
                    std::lround(
                        (zDKTestResizeOriginal.right - zDKTestResizeOriginal.left - zDKTestFrame.cx) * .8
                    )
                );
                zDKWinDiagnosticWindow = false;
                zDKUIScaleOverride = 0;
                SendMessageW(zDKWinApplicationWindow, WM_ENTERSIZEMOVE, 0, 0);
                SetWindowPos(
                    zDKWinApplicationWindow,
                    nullptr,
                    0,
                    0,
                    zDKTestWidth + zDKTestFrame.cx,
                    LONG(std::lround(zDKTestWidth * 980.0 / 540)) + zDKTestFrame.cy,
                    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
                );
                zDKTestNext();
                break;
            }
            case 601:
                if (zDKTestWait(4))
                {
                    RECT zDKTestClient{};
                    GetClientRect(zDKWinApplicationWindow, &zDKTestClient);
                    zDKTestExpect(
                        std::abs(zDKUIDisplayScale - zDKTestClient.right / 540.f) < .002f &&
                            std::abs(ImGui::GetIO().FontGlobalScale - zDKUIDisplayScale / zDKWinFontScale) <
                                .002f,
                        "Live border resizing scales fonts, controls and content together"
                    );
                    zDKTestExpect(
                        std::abs(zDKWinFontScale - 1) < .002f,
                        "Dragging reuses the font atlas instead of rebuilding it for every pixel"
                    );
                    SendMessageW(zDKWinApplicationWindow, WM_EXITSIZEMOVE, 0, 0);
                    zDKTestNext();
                }
                break;
            case 602:
                if (zDKTestWait(4))
                {
                    zDKTestExpect(
                        std::abs(zDKWinFontScale - zDKUIDisplayScale) < .002f &&
                            std::abs(ImGui::GetIO().FontGlobalScale - 1) < .002f,
                        "Releasing the border rebuilds sharp fonts at the final size"
                    );
                    zDKTestExpect(
                        zDKUICurrentUnit.zDKEffectEffects.size() == 3 &&
                            zDKEffectIsGuard(zDKUICurrentUnit.zDKEffectEffects[2]),
                        "Proportional resizing preserves entered unit effects"
                    );
                    zDKWinDiagnosticWindow = true;
                    zDKUIScaleOverride = 1;
                    SetWindowPos(
                        zDKWinApplicationWindow,
                        nullptr,
                        zDKTestResizeOriginal.left,
                        zDKTestResizeOriginal.top,
                        zDKTestResizeOriginal.right - zDKTestResizeOriginal.left,
                        zDKTestResizeOriginal.bottom - zDKTestResizeOriginal.top,
                        SWP_NOZORDER | SWP_NOACTIVATE
                    );
                    zDKUIPendingDisplayScale = 1;
                    zDKTestNext();
                }
                break;
            case 603:
                if (zDKTestWait(4))
                {
                    HWND zDKTestProbe = CreateWindowExW(
                        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                        L"STATIC",
                        L"zDK topmost capability check",
                        WS_POPUP | WS_VISIBLE,
                        -10000,
                        -10000,
                        32,
                        32,
                        nullptr,
                        nullptr,
                        GetModuleHandleW(nullptr),
                        nullptr
                    );
                    if (!zDKTestProbe)
                        zDKTestExpect(false, "Native topmost capability probe can be created");
                    else
                    {
                        SetWindowPos(
                            zDKTestProbe, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
                        );
                        zDKTestNativeTopmost =
                            (GetWindowLongPtrW(zDKTestProbe, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
                        DestroyWindow(zDKTestProbe);
                    }
                    zDKTestStep = 700;
                    zDKTestTicks = 0;
                }
                break;
            case 700:
                if (!zDKTestNativeTopmost)
                {
                    zDKTestLog << "SKIP: This Windows session does not apply native topmost state, including "
                                  "to an independent Win32 probe.\n";
                    zDKTestStep = 710;
                    zDKTestTicks = 0;
                    break;
                }
                if (zDKTestClick("Settings"))
                {
                    zDKTestStep = 707;
                    zDKTestTicks = 0;
                }
                break;
            case 707:
                if (zDKTestWait(6))
                {
                    zDKTestStep = 701;
                    zDKTestTicks = 0;
                }
                break;
            case 701:
                if (zDKTestClick("Always on Top"))
                    zDKTestNext();
                break;
            case 702:
                if (zDKTestWait(4))
                {
                    zDKTestExpect(
                        zDKUIIsAlwaysOnTop(zDKWinApplicationWindow),
                        "Settings enables the native Windows topmost state"
                    );
                    zDKTestExpect(
                        zDKUIControlProbes.count("topmost-indicator") != 0 &&
                            zDKUIControlProbes["topmost-indicator"].zDKUIFrame >=
                                ImGui::GetFrameCount() - 1 &&
                            zDKUIControlProbes["topmost-indicator"].zDKUICenter.x <
                                zDKUIControlProbes["Settings"].zDKUICenter.x,
                        "Enabled Always on Top displays a pin beside the Settings gear"
                    );
                    SendMessageW(zDKWinApplicationWindow, WM_ENTERSIZEMOVE, 0, 0);
                    const auto zDKTestFrame = zDKWinFrameSize(zDKWinApplicationWindow);
                    SetWindowPos(
                        zDKWinApplicationWindow,
                        nullptr,
                        0,
                        0,
                        432 + zDKTestFrame.cx,
                        784 + zDKTestFrame.cy,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
                    );
                    SendMessageW(zDKWinApplicationWindow, WM_EXITSIZEMOVE, 0, 0);
                    zDKTestNext();
                }
                break;
            case 703:
                if (zDKTestWait(4))
                {
                    zDKTestExpect(
                        zDKUIIsAlwaysOnTop(zDKWinApplicationWindow),
                        "Proportional resizing preserves Always on Top"
                    );
                    SetWindowPos(
                        zDKWinApplicationWindow,
                        nullptr,
                        zDKTestResizeOriginal.left,
                        zDKTestResizeOriginal.top,
                        zDKTestResizeOriginal.right - zDKTestResizeOriginal.left,
                        zDKTestResizeOriginal.bottom - zDKTestResizeOriginal.top,
                        SWP_NOZORDER | SWP_NOACTIVATE
                    );
                    zDKTestNext();
                }
                break;
            case 704:
                if (zDKTestClick("Always on Top"))
                    zDKTestNext();
                break;
            case 705:
                if (zDKTestWait(4))
                {
                    zDKTestExpect(
                        !zDKUIIsAlwaysOnTop(zDKWinApplicationWindow),
                        "Settings disables the native Windows topmost state"
                    );
                    zDKTestExpect(
                        zDKUIControlProbes["topmost-indicator"].zDKUIFrame < ImGui::GetFrameCount() - 1,
                        "Disabling Always on Top hides its header pin"
                    );
                    ImGui::GetIO().AddKeyEvent(ImGuiKey_Escape, true);
                    zDKTestNext();
                }
                break;
            case 706:
                ImGui::GetIO().AddKeyEvent(ImGuiKey_Escape, false);
                zDKTestStep = 710;
                zDKTestTicks = 0;
                break;
            case 710:
                if (zDKTestClick("Settings"))
                    zDKTestNext();
                break;
            case 711:
                if (zDKTestWait(6))
                    zDKTestNext();
                break;
            case 712:
                if (zDKTestClick("App updates"))
                    zDKTestNext();
                break;
            case 713:
                if (zDKTestWait(6))
                {
                    zDKTestExpect(
                        ImGui::IsPopupOpen("App updates", ImGuiPopupFlags_AnyPopupId),
                        "Settings opens the full-screen updater"
                    );
                    zDKTestExpect(
                        zDKUIUpdateState == zDKUIUpdatePhase::zDKUIIdle,
                        "Opening updates does not start an unwanted network request"
                    );
                    zDKTestNext();
                }
                break;
            case 714:
                if (zDKTestClick("Back to calculator"))
                    zDKTestNext();
                break;
            case 715:
                if (!zDKTestWait(4))
                    break;
                zDKTestExpect(
                    !ImGui::IsPopupOpen("App updates", ImGuiPopupFlags_AnyPopupId),
                    "The updater returns to the calculator"
                );
                zDKTestLog << zDKTestChecks << " UI checks passed.\n";
                zDKTestDone = true;
                break;
        }
    }
};
} // namespace zDKUI
