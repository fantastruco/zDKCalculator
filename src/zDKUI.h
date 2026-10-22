#pragma once
#include "zDKEffects.h"
#include "zDKTurnTimeline.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <cstring>
#include <map>
#include "zDKUISkin.h"
#include "zDKUINumberFormat.h"

namespace zDKUI
{
using namespace zDK;
extern ComPtr<ID3D11ShaderResourceView> zDKUIBackgroundTexture;
extern float zDKUIDisplayScale;
extern ImFont *zDKUIBodyFont, *zDKUIHeadingFont, *zDKUITitleFont, *zDKUIMetricFont, *zDKUICaptionFont,
    *zDKUILabelFont;
void zDKUIDrawScrollbar();
zDKEffectUnitProfile zDKUICurrentUnit = zDKEffectCreateEmptyUnitProfile();
zDKEffectBossProfile zDKUICurrentBoss;
zDKEffectBattleState zDKUICurrentBattle;
bool zDKUIAnimationsEnabled = true;
int zDKUISelectedAttackKind = 1, zDKUIRequestedUnitEffectIndex = -1, zDKUIRequestedBossEffectIndex = -1,
    zDKUIActiveTab = 0;
int zDKUIDisplayedTab = -1, zDKUIPageDirection = 1;
int zDKUISelectedSubviews[3] = {0, 0, 0}, zDKUIDisplayedSection = 0;
float zDKUIPageProgress = 1;

struct zDKUIDragGesture
{
    std::string zDKUISurface;
    float zDKUIOriginY = 0, zDKUILastY = 0;
    bool zDKUICandidate = false, zDKUIDragging = false;
    float zDKUIVelocity = 0;
} zDKUIDragGestureState;

bool zDKUIGesturePressBlocked = false;

void zDKUIBlockGestureInput()
{
    zDKUIGesturePressBlocked = true;
}

bool zDKUISuppressGestureClick()
{
    const float zDKUIDragThreshold = 10 * zDKUIDisplayScale;
    return zDKUIDragGestureState.zDKUIDragging ||
        (ImGui::IsMouseReleased(0) &&
         ImGui::GetIO().MouseDragMaxDistanceSqr[0] > zDKUIDragThreshold * zDKUIDragThreshold);
}

void zDKUIBlockGestureField()
{
    if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered())
        zDKUIBlockGestureInput();
}

struct zDKUIControlProbe
{
    ImVec2 zDKUICenter;
    int zDKUIFrame;
    std::string zDKUIInputText;
};

struct zDKUIScrollProbe
{
    ImVec2 zDKUICenter;
    float zDKUIPosition, zDKUIDestination, zDKUIMaximum;
    ImVec2 zDKUIMin{}, zDKUIMax{};
};

bool zDKUICollectInteractionProbes = false;
std::map<std::string, zDKUIControlProbe> zDKUIControlProbes;
std::map<std::string, zDKUIScrollProbe> zDKUIScrollProbes;

void zDKUIRecordControlProbe(const std::string& zDKUIKey)
{
    if (zDKUICollectInteractionProbes && ImGui::IsItemVisible())
    {
        auto zDKUITopLeft = ImGui::GetItemRectMin(), zDKUIBottomRight = ImGui::GetItemRectMax();
        zDKUIControlProbes[zDKUIKey] = {
            {(zDKUITopLeft.x + zDKUIBottomRight.x) * .5f, (zDKUITopLeft.y + zDKUIBottomRight.y) * .5f},
            ImGui::GetFrameCount()
        };
    }
}

std::string zDKUIStatus;
double zDKUIStatusUntil = 0;
const ImVec4 zDKUIPink(1, .24f, .61f, 1), zDKUIMuted(.79f, .76f, .82f, 1), zDKUIWhite(.98f, .97f, 1, 1),
    zDKUIGreen(.37f, .89f, .68f, 1);

float zDKUIScalePixels(float zDKUINumber)
{
    return zDKUINumber * zDKUIDisplayScale;
}

ImVec2 zDKUIScaleSize(float zDKUIHorizontalPosition, float zDKUIVerticalPosition)
{
    return {
        zDKUIHorizontalPosition > 0 ? zDKUIScalePixels(zDKUIHorizontalPosition) : zDKUIHorizontalPosition,
        zDKUIVerticalPosition > 0 ? zDKUIScalePixels(zDKUIVerticalPosition) : zDKUIVerticalPosition
    };
}

ImVec2 zDKUIAddVectors(ImVec2 zDKUITopLeft, ImVec2 zDKUIBottomRight)
{
    return {zDKUITopLeft.x + zDKUIBottomRight.x, zDKUITopLeft.y + zDKUIBottomRight.y};
}

ImU32 zDKUIPackColor(ImVec4 zDKUIColorData)
{
    return ImGui::GetColorU32(zDKUIColorData);
}

std::string zDKUIFormatInteger(double zDKUIValue)
{
    char zDKUIFormatBuffer[80];
    if (!std::isfinite(zDKUIValue) || std::abs(zDKUIValue) >= 9e18)
    {
        snprintf(zDKUIFormatBuffer, sizeof(zDKUIFormatBuffer), "%.3e", zDKUIValue);
        return zDKUIFormatBuffer;
    }
    std::string zDKUIText = std::to_string(static_cast<long long>(std::floor(zDKUIValue)));
    for (int zDKUIIndex = int(zDKUIText.size()) - 3; zDKUIIndex > (zDKUIValue < 0 ? 1 : 0); zDKUIIndex -= 3)
        zDKUIText.insert(zDKUIIndex, ",");
    return zDKUIText;
}

std::string zDKUIFormatDecimal(double zDKUIValue, int zDKUIPrecision = 1)
{
    char zDKUISecondValue[80];
    snprintf(zDKUISecondValue, sizeof(zDKUISecondValue), "%.*f", zDKUIPrecision, zDKUIValue);
    return zDKUISecondValue;
}

std::string zDKUIFormatMultiplier(double zDKUIValue)
{
    return "×" + zDKUIFormatDecimal(zDKUIValue, 3);
}

void zDKUIDrawFontText(ImFont* zDKUIFont, ImVec4 zDKUIColor, const std::string& zDKUIText)
{
    ImGui::PushFont(zDKUIFont);
    ImGui::TextColored(zDKUIColor, "%s", zDKUIText.c_str());
    ImGui::PopFont();
}

void zDKUIDrawWrappedText(const std::string& zDKUIText, ImVec4 zDKUIColor = zDKUIMuted)
{
    ImGui::PushStyleColor(ImGuiCol_Text, zDKUIColor);
    ImGui::TextWrapped("%s", zDKUIText.c_str());
    ImGui::PopStyleColor();
}

void zDKUIAddSpacing(float zDKUINumber = 6)
{
    ImGui::Dummy(zDKUIScaleSize(0, zDKUINumber));
}

void zDKUIDrawDivider()
{
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    zDKUIDrawSkin(
        ImGui::GetWindowDrawList(),
        zDKUISkinKind::zDKUIDivider,
        {zDKUITopLeft.x, zDKUITopLeft.y + zDKUIScalePixels(3)},
        {zDKUITopLeft.x + ImGui::GetContentRegionAvail().x, zDKUITopLeft.y + zDKUIScalePixels(12)},
        0,
        .65f
    );
    ImGui::Dummy({0, zDKUIScalePixels(16)});
}

void zDKUIShowTooltip(const char* zDKUIText)
{
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(zDKUIScalePixels(330));
        ImGui::TextUnformatted(zDKUIText);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void zDKUIShowNotice(std::string zDKUIText)
{
    zDKUIStatus = std::move(zDKUIText);
    zDKUIStatusUntil = ImGui::GetTime() + 5;
}

bool zDKUIDrawButton(
    const char* zDKUILabel,
    ImVec2 zDKUISize = {0, 0},
    bool zDKUISelected = false,
    zDKUISkinKind zDKUIKind = zDKUISkinKind::zDKUIButton
)
{
    if (zDKUISize.y == 0)
        zDKUISize.y = zDKUIScalePixels(50);
    if (zDKUISize.x == 0)
        zDKUISize.x =
            std::max(zDKUIScalePixels(76), ImGui::CalcTextSize(zDKUILabel).x + zDKUIScalePixels(48));
    zDKUISkinLayer zDKUILayer;
    zDKUIBeginTransparentControl();
    ImGui::PushID(zDKUILabel);
    bool zDKUIHit = ImGui::Button("##button", zDKUISize);
    ImGui::PopID();
    zDKUIEndTransparentControl();
    ImVec2 zDKUITopLeft = ImGui::GetItemRectMin(), zDKUIBottomRight = ImGui::GetItemRectMax();
    zDKUIRecordControlProbe(zDKUILabel);
    zDKUILayer.zDKUIPaint(
        zDKUIKind,
        zDKUITopLeft,
        zDKUIBottomRight,
        ImGui::IsItemHovered(),
        zDKUISelected || ImGui::IsItemActive() || ImGui::IsItemFocused()
    );
    zDKUILayer.zDKUIDrawChannels.SetCurrentChannel(zDKUILayer.zDKUIDrawList, 1);
    zDKUIDrawControlText(
        zDKUILabel,
        zDKUITopLeft,
        zDKUIBottomRight,
        std::min(zDKUIScalePixels(25), (zDKUIBottomRight.x - zDKUITopLeft.x) * .18f)
    );
    return zDKUIHit && !zDKUISuppressGestureClick();
}

void zDKUIDrawLabel(const char* zDKUIText)
{
    float zDKUIWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    ImVec2 zDKUIMeasured = zDKUILabelFont->CalcTextSizeA(
        (zDKUILabelFont->FontSize * ImGui::GetIO().FontGlobalScale),
        FLT_MAX,
        zDKUIWidth - zDKUIScalePixels(4),
        zDKUIText
    );
    ImGui::Dummy({zDKUIWidth, zDKUIMeasured.y + zDKUIScalePixels(3)});
    zDKUIDrawOutlinedText(
        ImGui::GetWindowDrawList(),
        zDKUILabelFont,
        (zDKUILabelFont->FontSize * ImGui::GetIO().FontGlobalScale),
        {zDKUITopLeft.x + zDKUIScalePixels(3), zDKUITopLeft.y},
        zDKUIWhite,
        zDKUIText,
        zDKUIWidth - zDKUIScalePixels(4)
    );
}

struct zDKUINumberEditContext
{
    ImGuiID zDKUIId;
    bool zDKUIInteger;
};

struct zDKUINumberEditState
{
    ImGuiID zDKUIId = 0;
    std::string zDKUIText;
    bool zDKUIHasSelection = false;
} zDKUIActiveNumberEdit;

int zDKUIFormatNumberEdit(ImGuiInputTextCallbackData* zDKUIData)
{
    const auto& zDKUIContext = *static_cast<zDKUINumberEditContext*>(zDKUIData->UserData);
    if (zDKUIData->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
    {
        const ImWchar zDKUICharacter = zDKUIData->EventChar;
        return (zDKUICharacter >= '0' && zDKUICharacter <= '9') || zDKUICharacter == ',' ||
                zDKUICharacter == '-' || zDKUICharacter == '+' ||
                (!zDKUIContext.zDKUIInteger &&
                 (zDKUICharacter == '.' || zDKUICharacter == 'e' || zDKUICharacter == 'E'))
            ? 0
            : 1;
    }
    const std::string zDKUIBefore(zDKUIData->Buf, zDKUIData->BufTextLen);
    std::string zDKUIRaw = zDKUIRemoveNumberGrouping(zDKUIBefore);
    int zDKUICursor = zDKUINumberRawPosition(zDKUIBefore, zDKUIData->CursorPos);
    int zDKUISelectionStart = zDKUINumberRawPosition(zDKUIBefore, zDKUIData->SelectionStart);
    int zDKUISelectionEnd = zDKUINumberRawPosition(zDKUIBefore, zDKUIData->SelectionEnd);
    // Backspace/Delete beside a generated comma should remove the adjacent digit
    // in one press, rather than repeatedly deleting and reinserting that comma.
    if (zDKUIData->EventFlag == ImGuiInputTextFlags_CallbackEdit &&
        zDKUIActiveNumberEdit.zDKUIId == zDKUIContext.zDKUIId && !zDKUIActiveNumberEdit.zDKUIHasSelection &&
        zDKUIBefore.size() + 1 == zDKUIActiveNumberEdit.zDKUIText.size() &&
        zDKUIRaw == zDKUIRemoveNumberGrouping(zDKUIActiveNumberEdit.zDKUIText))
    {
        const bool zDKUIBackspace = ImGui::IsKeyPressed(ImGuiKey_Backspace);
        const int zDKUIErasePosition = zDKUICursor - (zDKUIBackspace ? 1 : 0);
        if ((zDKUIBackspace || ImGui::IsKeyPressed(ImGuiKey_Delete)) && zDKUIErasePosition >= 0 &&
            zDKUIErasePosition < int(zDKUIRaw.size()) && zDKUIRaw[zDKUIErasePosition] >= '0' &&
            zDKUIRaw[zDKUIErasePosition] <= '9')
        {
            zDKUIRaw.erase(zDKUIErasePosition, 1);
            zDKUICursor -= zDKUIBackspace ? 1 : 0;
            zDKUISelectionStart = zDKUISelectionEnd = zDKUICursor;
        }
    }
    const std::string zDKUIFormatted = zDKUIGroupNumberText(zDKUIRaw);
    if (zDKUIFormatted != zDKUIBefore && zDKUIFormatted.size() < size_t(zDKUIData->BufSize))
    {
        std::memcpy(zDKUIData->Buf, zDKUIFormatted.c_str(), zDKUIFormatted.size() + 1);
        zDKUIData->BufTextLen = int(zDKUIFormatted.size());
        zDKUIData->BufDirty = true;
        zDKUIData->CursorPos = zDKUINumberGroupedPosition(zDKUIFormatted, zDKUICursor);
        zDKUIData->SelectionStart = zDKUINumberGroupedPosition(zDKUIFormatted, zDKUISelectionStart);
        zDKUIData->SelectionEnd = zDKUINumberGroupedPosition(zDKUIFormatted, zDKUISelectionEnd);
    }
    zDKUIActiveNumberEdit = {zDKUIContext.zDKUIId, zDKUIData->Buf, zDKUIData->HasSelection()};
    return 0;
}

bool zDKUIEditGroupedNumber(char* zDKUIBuffer, size_t zDKUICapacity, bool zDKUIInteger)
{
    zDKUINumberEditContext zDKUIContext{ImGui::GetID("##value"), zDKUIInteger};
    return ImGui::InputText(
        "##value",
        zDKUIBuffer,
        zDKUICapacity,
        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter |
            ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways,
        zDKUIFormatNumberEdit,
        &zDKUIContext
    );
}

void zDKUIDrawNumberField(const char* zDKUIId, double& zDKUIValue, const char* zDKUIFormat = "%.0f")
{
    const bool zDKUIWholeStat = strcmp(zDKUIId, "Base DEF") == 0 || strcmp(zDKUIId, "DEF stat") == 0 ||
        strcmp(zDKUIId, "Normal ATK") == 0 || strcmp(zDKUIId, "Super Attack ATK") == 0 ||
        strcmp(zDKUIId, "Enemy DEF") == 0 || strcmp(zDKUIId, "Base ATK") == 0 ||
        strcmp(zDKUIId, "Max HP") == 0 || strcmp(zDKUIId, "Remaining HP") == 0;
    if (zDKUIWholeStat && std::isfinite(zDKUIValue))
        zDKUIValue = std::floor(zDKUIValue);
    ImGui::PushID(zDKUIId);
    ImGui::SetNextItemWidth(-1);
    zDKUISkinLayer zDKUILayer;
    zDKUIBeginTransparentControl();
    char zDKUIBuffer[512];
    snprintf(zDKUIBuffer, sizeof(zDKUIBuffer), zDKUIFormat, zDKUIValue);
    const std::string zDKUIFormatted = zDKUIGroupNumberText(zDKUIBuffer);
    strncpy_s(zDKUIBuffer, zDKUIFormatted.c_str(), _TRUNCATE);
    if (zDKUIEditGroupedNumber(zDKUIBuffer, sizeof(zDKUIBuffer), zDKUIWholeStat))
    {
        zDKUIParseNumber(zDKUIBuffer, zDKUIValue);
        if (zDKUIWholeStat && std::isfinite(zDKUIValue))
            zDKUIValue = std::floor(zDKUIValue);
    }
    zDKUIBlockGestureField();
    zDKUIRecordControlProbe(zDKUIId);
    if (zDKUICollectInteractionProbes)
        zDKUIControlProbes[zDKUIId].zDKUIInputText = zDKUIBuffer;
    zDKUIEndTransparentControl();
    zDKUISkinLastItem(zDKUILayer, zDKUISkinKind::zDKUIField);
    ImGui::PopID();
}

void zDKUIDrawNumberInput(
    const char* zDKUIId, double& zDKUIValue, const char* zDKUIFormat = "%.0f", const char* zDKUIHelp = nullptr
)
{
    zDKUIDrawLabel(zDKUIId);
    if (zDKUIHelp)
        zDKUIShowTooltip(zDKUIHelp);
    zDKUIDrawNumberField(zDKUIId, zDKUIValue, zDKUIFormat);
}

void zDKUIDrawIntegerField(const char* zDKUIId, int& zDKUIValue)
{
    ImGui::PushID(zDKUIId);
    ImGui::SetNextItemWidth(-1);
    zDKUISkinLayer zDKUILayer;
    zDKUIBeginTransparentControl();
    char zDKUIBuffer[64];
    const std::string zDKUIFormatted = zDKUIGroupNumberText(std::to_string(zDKUIValue));
    strncpy_s(zDKUIBuffer, zDKUIFormatted.c_str(), _TRUNCATE);
    if (zDKUIEditGroupedNumber(zDKUIBuffer, sizeof(zDKUIBuffer), true))
        zDKUIParseInteger(zDKUIBuffer, zDKUIValue);
    zDKUIBlockGestureField();
    zDKUIRecordControlProbe(zDKUIId);
    if (zDKUICollectInteractionProbes)
        zDKUIControlProbes[zDKUIId].zDKUIInputText = zDKUIBuffer;
    zDKUIEndTransparentControl();
    zDKUISkinLastItem(zDKUILayer, zDKUISkinKind::zDKUIField);
    ImGui::PopID();
}

void zDKUIDrawTextField(const char* zDKUIId, std::string& zDKUIValue)
{
    char zDKUISecondValue[192]{};
    strncpy_s(zDKUISecondValue, zDKUIValue.c_str(), _TRUNCATE);
    ImGui::PushID(zDKUIId);
    ImGui::SetNextItemWidth(-1);
    zDKUISkinLayer zDKUILayer;
    zDKUIBeginTransparentControl();
    if (ImGui::InputText("##text", zDKUISecondValue, sizeof(zDKUISecondValue)))
        zDKUIValue = zDKUISecondValue;
    zDKUIBlockGestureField();
    zDKUIEndTransparentControl();
    zDKUISkinLastItem(zDKUILayer, zDKUISkinKind::zDKUIField);
    ImGui::PopID();
}

void zDKUIDrawNameInput(const char* zDKUIId, std::string& zDKUIValue)
{
    zDKUIDrawLabel(zDKUIId);
    zDKUIDrawTextField(zDKUIId, zDKUIValue);
}

void zDKUIDrawFieldPair(
    const char* zDKUIFirstValue,
    const char* zDKUISecondValue,
    const std::function<void()>& zDKUILeft,
    const std::function<void()>& zDKUIRight,
    bool zDKUICompact = false,
    float zDKUIRightWeight = 1
)
{
    if (!zDKUICompact && ImGui::GetContentRegionAvail().x < zDKUIScalePixels(390))
    {
        zDKUIDrawLabel(zDKUIFirstValue);
        zDKUILeft();
        zDKUIDrawLabel(zDKUISecondValue);
        zDKUIRight();
        return;
    }
    ImGui::PushID(zDKUIFirstValue);
    if (ImGui::BeginTable("fields", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("left", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableSetupColumn("right", ImGuiTableColumnFlags_WidthStretch, zDKUIRightWeight);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        zDKUIDrawLabel(zDKUIFirstValue);
        ImGui::TableNextColumn();
        zDKUIDrawLabel(zDKUISecondValue);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        zDKUILeft();
        ImGui::TableNextColumn();
        zDKUIRight();
        ImGui::EndTable();
    }
    ImGui::PopID();
}

void zDKUIDrawNumberPair(
    const char* zDKUIFirstValue,
    double& zDKUILeftValue,
    const char* zDKUISecondValue,
    double& zDKUIRightValue,
    const char* zDKUIFormat = "%.0f"
)
{
    zDKUIDrawFieldPair(
        zDKUIFirstValue,
        zDKUISecondValue,
        [&] { zDKUIDrawNumberField(zDKUIFirstValue, zDKUILeftValue, zDKUIFormat); },
        [&] { zDKUIDrawNumberField(zDKUISecondValue, zDKUIRightValue, zDKUIFormat); }
    );
}

bool zDKUIDrawComboField(const char* zDKUIId, int& zDKUIValue, const char* const* zDKUINames, int zDKUICount)
{
    ImGui::PushID(zDKUIId);
    bool zDKUIChanged = false;
    bool zDKUIOpen = zDKUIBeginCombo("##combo", zDKUINames[zDKUIValue]);
    zDKUIRecordControlProbe(zDKUIId);
    if (zDKUIOpen)
    {
        for (int zDKUIIndex = 0; zDKUIIndex < zDKUICount; ++zDKUIIndex)
        {
            if (ImGui::Selectable(zDKUINames[zDKUIIndex], zDKUIValue == zDKUIIndex))
            {
                zDKUIValue = zDKUIIndex;
                zDKUIChanged = true;
            }
            zDKUIRecordControlProbe(std::string("option-") + zDKUIId + "-" + std::to_string(zDKUIIndex));
            zDKUIShowTooltip(zDKUINames[zDKUIIndex]);
        }
        zDKUIDrawScrollbar();
        ImGui::EndCombo();
    }
    ImGui::PopID();
    return zDKUIChanged;
}

bool zDKUIDrawComboInput(const char* zDKUIId, int& zDKUIValue, const char* const* zDKUINames, int zDKUICount)
{
    zDKUIDrawLabel(zDKUIId);
    return zDKUIDrawComboField(zDKUIId, zDKUIValue, zDKUINames, zDKUICount);
}

void zDKUIDrawTypeAndClass(zDKCalcUnitType& zDKUIElementType, zDKCalcUnitClass& zDKUIClassIndex)
{
    static const char* zDKUITypes[] = {"AGL", "TEQ", "INT", "STR", "PHY"};
    static const char* zDKUIClasses[] = {"Super", "Extreme"};
    int zDKUITypeIndex = int(zDKUIElementType), zDKUIColor = int(zDKUIClassIndex);
    static const ImVec4 zDKUIColors[] = {
        {.28f, .69f, 1, 1}, {.30f, .95f, .43f, 1}, {.79f, .43f, 1, 1}, {1, .33f, .36f, 1}, {1, .77f, .26f, 1}
    };
    zDKUIDrawFieldPair(
        "Type",
        "Class",
        [&]
        {
            ImGui::PushID("type");
            ImGui::PushFont(zDKUILabelFont);
            ImGui::PushStyleColor(ImGuiCol_Text, zDKUIColors[zDKUITypeIndex]);
            bool zDKUIOpen = zDKUIBeginCombo("##type", zDKUITypes[zDKUITypeIndex]);
            ImGui::PopStyleColor();
            if (zDKUIOpen)
            {
                for (int zDKUIIndex = 0; zDKUIIndex < 5; ++zDKUIIndex)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, zDKUIColors[zDKUIIndex]);
                    if (ImGui::Selectable(zDKUITypes[zDKUIIndex], zDKUITypeIndex == zDKUIIndex))
                        zDKUITypeIndex = zDKUIIndex;
                    ImGui::PopStyleColor();
                }
                zDKUIDrawScrollbar();
                ImGui::EndCombo();
            }
            ImGui::PopFont();
            ImGui::PopID();
        },
        [&] { zDKUIDrawComboField("class", zDKUIColor, zDKUIClasses, 2); },
        ImGui::GetContentRegionAvail().x >= zDKUIScalePixels(320)
    );
    zDKUIElementType = zDKCalcUnitType(zDKUITypeIndex);
    zDKUIClassIndex = zDKCalcUnitClass(zDKUIColor);
}

void zDKUIDrawStatisticRow(
    const char* zDKUIName, const std::string& zDKUIValue, ImVec4 zDKUIColor = zDKUIWhite
)
{
    if (ImGui::GetContentRegionAvail().x < zDKUIScalePixels(350) ||
        ImGui::CalcTextSize(zDKUIValue.c_str()).x > ImGui::GetContentRegionAvail().x * .53f)
    {
        zDKUIDrawWrappedText(zDKUIName);
        zDKUIDrawWrappedText(zDKUIValue, zDKUIColor);
        return;
    }
    ImGui::PushID(zDKUIName);
    if (ImGui::BeginTable("stat", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(
            "value",
            ImGuiTableColumnFlags_WidthFixed,
            ImGui::CalcTextSize(zDKUIValue.c_str()).x + zDKUIScalePixels(2)
        );
        ImGui::TableNextColumn();
        zDKUIDrawWrappedText(zDKUIName);
        ImGui::TableNextColumn();
        ImGui::TextColored(zDKUIColor, "%s", zDKUIValue.c_str());
        ImGui::EndTable();
    }
    ImGui::PopID();
}

void zDKUIDrawTypeDefenseBoost(double& zDKUIValue)
{
    zDKUIDrawLabel("Type DEF Boost");
    auto* zDKUIStorage = ImGui::GetStateStorage();
    ImGuiID zDKUIKey = ImGui::GetID("tdb-choice");
    int zDKUIInferred = zDKUIValue == 6 ? 0 : zDKUIValue == 11 ? 1 : 2;
    int zDKUIMode = zDKUIStorage->GetInt(zDKUIKey, zDKUIInferred);
    if (zDKUIStorage->GetFloat(zDKUIKey ^ 0x145a1u, float(zDKUIValue)) != float(zDKUIValue))
        zDKUIMode = zDKUIInferred;
    const char* zDKUIOptions[] = {"Hidden Potential 55%", "Hidden Potential 100%", "Custom"};
    if (zDKUIDrawComboField("tdb-choice", zDKUIMode, zDKUIOptions, 3))
    {
        if (zDKUIMode == 0)
            zDKUIValue = 6;
        else if (zDKUIMode == 1)
            zDKUIValue = 11;
    }
    if (zDKUIMode == 2)
        zDKUIDrawNumberInput("Custom Lv.", zDKUIValue, "%.0f");
    zDKUIStorage->SetInt(zDKUIKey, zDKUIMode);
    zDKUIStorage->SetFloat(zDKUIKey ^ 0x145a1u, float(zDKUIValue));
}

void zDKUIDrawScrollbar()
{
    float zDKUIMaximum = ImGui::GetScrollMaxY();
    if (zDKUIMaximum <= 0)
        return;
    ImVec2 zDKUIPosition = ImGui::GetWindowPos(), zDKUISize = ImGui::GetWindowSize();
    float zDKUIHorizontalPosition = zDKUIPosition.x + zDKUISize.x - zDKUIScalePixels(11),
          zDKUITop = zDKUIPosition.y + zDKUIScalePixels(10), zDKUIHeight = zDKUISize.y - zDKUIScalePixels(20);
    float zDKUIGrip = std::clamp(
        zDKUIHeight * zDKUISize.y / (zDKUISize.y + zDKUIMaximum), zDKUIScalePixels(34), zDKUIHeight
    );
    float zDKUIVerticalPosition =
        zDKUITop + (zDKUIHeight - zDKUIGrip) * std::clamp(ImGui::GetScrollY() / zDKUIMaximum, 0.f, 1.f);
    if (zDKUICollectInteractionProbes)
    {
        zDKUIControlProbes["scroll-thumb"] = {
            {zDKUIHorizontalPosition, zDKUIVerticalPosition + zDKUIGrip * .5f}, ImGui::GetFrameCount()
        };
        zDKUIControlProbes["scroll-end"] = {
            {zDKUIHorizontalPosition, zDKUITop + zDKUIHeight - zDKUIScalePixels(8)}, ImGui::GetFrameCount()
        };
    }
    auto* zDKUIDrawList = ImGui::GetWindowDrawList();
    zDKUIDrawList->PushClipRect(
        zDKUIPosition, {zDKUIPosition.x + zDKUISize.x, zDKUIPosition.y + zDKUISize.y}, false
    );
    zDKUIDrawList->AddRectFilled(
        {zDKUIHorizontalPosition - zDKUIScalePixels(4), zDKUITop},
        {zDKUIHorizontalPosition + zDKUIScalePixels(4), zDKUITop + zDKUIHeight},
        zDKUIPackColor({.025f, .01f, .03f, 1}),
        zDKUIScalePixels(4)
    );
    zDKUIDrawList->AddLine(
        {zDKUIHorizontalPosition, zDKUITop + zDKUIScalePixels(3)},
        {zDKUIHorizontalPosition, zDKUITop + zDKUIHeight - zDKUIScalePixels(3)},
        zDKUIPackColor({.32f, .08f, .22f, 1}),
        zDKUIScalePixels(1)
    );
    bool zDKUIHovered = ImGui::IsMouseHoveringRect(
        {zDKUIHorizontalPosition - zDKUIScalePixels(9), zDKUITop},
        {zDKUIHorizontalPosition + zDKUIScalePixels(9), zDKUITop + zDKUIHeight}
    );
    float zDKUIGlow = zDKUIAnimateValue(ImGui::GetID("scroll-glow"), zDKUIHovered ? 1.f : 0.f);
    zDKUIDrawList->AddRectFilled(
        {zDKUIHorizontalPosition - zDKUIScalePixels(6), zDKUIVerticalPosition},
        {zDKUIHorizontalPosition + zDKUIScalePixels(6), zDKUIVerticalPosition + zDKUIGrip},
        zDKUIPackColor({1, .10f, .49f, .10f + .14f * zDKUIGlow}),
        zDKUIScalePixels(5)
    );
    zDKUIDrawList->AddRectFilled(
        {zDKUIHorizontalPosition - zDKUIScalePixels(3), zDKUIVerticalPosition},
        {zDKUIHorizontalPosition + zDKUIScalePixels(3), zDKUIVerticalPosition + zDKUIGrip},
        zDKUIPackColor({.79f + .21f * zDKUIGlow, .12f + .15f * zDKUIGlow, .43f + .16f * zDKUIGlow, 1}),
        zDKUIScalePixels(3)
    );
    zDKUIDrawList->AddLine(
        {zDKUIHorizontalPosition - zDKUIScalePixels(1), zDKUIVerticalPosition + zDKUIScalePixels(6)},
        {zDKUIHorizontalPosition - zDKUIScalePixels(1),
         zDKUIVerticalPosition + zDKUIGrip - zDKUIScalePixels(6)},
        zDKUIPackColor({1, .72f, .86f, .8f}),
        zDKUIScalePixels(1)
    );
    for (int zDKUIIndex = -1; zDKUIIndex <= 1; ++zDKUIIndex)
        zDKUIDrawList->AddLine(
            {zDKUIHorizontalPosition - zDKUIScalePixels(2),
             zDKUIVerticalPosition + zDKUIGrip * .5f + zDKUIScalePixels(float(zDKUIIndex) * 4)},
            {zDKUIHorizontalPosition + zDKUIScalePixels(2),
             zDKUIVerticalPosition + zDKUIGrip * .5f + zDKUIScalePixels(float(zDKUIIndex) * 4)},
            zDKUIPackColor({1, .86f, .93f, .9f}),
            zDKUIScalePixels(1)
        );
    zDKUIDrawList->PopClipRect();
}

struct zDKUIScrollContext
{
    ImGuiStorage* zDKUIStorage;
    ImGuiID zDKUIKey;
    std::string zDKUIName;
};

std::vector<zDKUIScrollContext> zDKUIScrollContexts;

void zDKUIHandleScrollDrag(
    const std::string& zDKUIId, ImVec2 zDKUITopLeft, ImVec2 zDKUIBottomRight, zDKUIScrollContext& zDKUIScroll
)
{
    auto& zDKUIInputOutput = ImGui::GetIO();
    bool zDKUIHovered = ImGui::IsWindowHovered(
                            ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
                        ) &&
        ImGui::IsMouseHoveringRect(zDKUITopLeft, zDKUIBottomRight);
    if (ImGui::IsMouseClicked(0) && zDKUIHovered && !zDKUIGesturePressBlocked)
    {
        zDKUIDragGestureState = {};
        zDKUIDragGestureState.zDKUISurface = zDKUIId;
        zDKUIDragGestureState.zDKUIOriginY = zDKUIDragGestureState.zDKUILastY = zDKUIInputOutput.MousePos.y;
        zDKUIDragGestureState.zDKUICandidate = true;
    }
    if (!zDKUIDragGestureState.zDKUICandidate || zDKUIDragGestureState.zDKUISurface != zDKUIId)
        return;
    const float zDKUIVerticalDistance =
        std::abs(zDKUIInputOutput.MousePos.y - zDKUIDragGestureState.zDKUIOriginY);
    if (zDKUIInputOutput.MouseDown[0])
    {
        if (!zDKUIDragGestureState.zDKUIDragging && zDKUIVerticalDistance > zDKUIScalePixels(10) &&
            (zDKUIInputOutput.MouseDownDuration[0] > .10f || zDKUIVerticalDistance > zDKUIScalePixels(28)))
        {
            zDKUIDragGestureState.zDKUIDragging = true;
        }
        if (zDKUIDragGestureState.zDKUIDragging)
        {
            float zDKUIDelta = zDKUIInputOutput.MousePos.y - zDKUIDragGestureState.zDKUILastY;
            float zDKUIPosition = std::clamp(ImGui::GetScrollY() - zDKUIDelta, 0.f, ImGui::GetScrollMaxY());
            zDKUIScroll.zDKUIStorage->SetFloat(zDKUIScroll.zDKUIKey ^ 0x1192abu, zDKUIPosition);
            zDKUIScroll.zDKUIStorage->SetFloat(zDKUIScroll.zDKUIKey ^ 0x84f0c1u, zDKUIPosition);
            ImGui::SetScrollY(zDKUIPosition);
            zDKUIDragGestureState.zDKUIVelocity = .6f * zDKUIDragGestureState.zDKUIVelocity -
                .4f * zDKUIDelta / std::max(.001f, zDKUIInputOutput.DeltaTime);
        }
        zDKUIDragGestureState.zDKUILastY = zDKUIInputOutput.MousePos.y;
    }
    else if (ImGui::IsMouseReleased(0))
    {
        if (zDKUIDragGestureState.zDKUIDragging)
        {
            float zDKUIDestination = std::clamp(
                ImGui::GetScrollY() +
                    (zDKUIAnimationsEnabled ? zDKUIDragGestureState.zDKUIVelocity * .11f : 0),
                0.f,
                ImGui::GetScrollMaxY()
            );
            zDKUIScroll.zDKUIStorage->SetFloat(zDKUIScroll.zDKUIKey ^ 0x84f0c1u, zDKUIDestination);
        }
        zDKUIDragGestureState.zDKUICandidate = false;
    }
}

void zDKUIEndSmoothChild()
{
    auto zDKUIScrollArea = zDKUIScrollContexts.back();
    zDKUIScrollContexts.pop_back();
    ImVec2 zDKUITopLeft = ImGui::GetWindowPos(), zDKUISize = ImGui::GetWindowSize();
    bool zDKUIPage = zDKUIScrollArea.zDKUIName.find("-panel") != std::string::npos;
    if (!zDKUIPage || zDKUIPageProgress > .99f)
        zDKUIHandleScrollDrag(
            zDKUIScrollArea.zDKUIName,
            zDKUITopLeft,
            {zDKUITopLeft.x + zDKUISize.x - zDKUIScalePixels(22), zDKUITopLeft.y + zDKUISize.y},
            zDKUIScrollArea
        );
    zDKUIDrawScrollbar();
    ImGui::EndChild();
}

void zDKUISmoothChild(
    const char* zDKUIId,
    ImVec2 zDKUISize,
    ImGuiChildFlags zDKUIChildFlags,
    ImGuiWindowFlags zDKUIFlags,
    bool zDKUIReset = false
)
{
    auto* zDKUIStorage = ImGui::GetStateStorage();
    ImGuiID zDKUIKey = ImGui::GetID(zDKUIId);
    zDKUIScrollContexts.push_back({zDKUIStorage, zDKUIKey, zDKUIId});
    float zDKUIPosition = zDKUIStorage->GetFloat(zDKUIKey ^ 0x1192abu, 0),
          zDKUIDestination = zDKUIStorage->GetFloat(zDKUIKey ^ 0x84f0c1u, 0);
    if (zDKUIReset)
        zDKUIPosition = zDKUIDestination = 0;
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    float zDKUIWidth = zDKUISize.x > 0 ? zDKUISize.x : ImGui::GetContentRegionAvail().x;
    auto& zDKUIInputOutput = ImGui::GetIO();
    bool zDKUINativeScroll = zDKUIInputOutput.NavVisible ||
        (zDKUIInputOutput.MouseDown[0] &&
         zDKUIInputOutput.MousePos.x > zDKUITopLeft.x + zDKUIWidth - zDKUIScalePixels(18));
    if (!zDKUINativeScroll)
        ImGui::SetNextWindowScroll({-1, zDKUIPosition});
    ImGui::BeginChild(zDKUIId, zDKUISize, zDKUIChildFlags, zDKUIFlags);
    if (zDKUINativeScroll)
    {
        zDKUIPosition = zDKUIDestination = ImGui::GetScrollY();
    }
    else
    {
        zDKUIPosition = ImGui::GetScrollY();
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && zDKUIInputOutput.MouseWheel != 0)
            zDKUIDestination -= zDKUIInputOutput.MouseWheel * zDKUIScalePixels(86);
        zDKUIDestination = std::clamp(zDKUIDestination, 0.f, ImGui::GetScrollMaxY());
        zDKUIPosition = zDKUIAnimationsEnabled ? zDKUIPosition +
                (zDKUIDestination - zDKUIPosition) * (1 - std::exp(-zDKUIInputOutput.DeltaTime / .085f))
                                               : zDKUIDestination;
        if (std::abs(zDKUIPosition - zDKUIDestination) < .2f)
            zDKUIPosition = zDKUIDestination;
    }
    zDKUIStorage->SetFloat(zDKUIKey ^ 0x1192abu, zDKUIPosition);
    zDKUIStorage->SetFloat(zDKUIKey ^ 0x84f0c1u, zDKUIDestination);
    if (zDKUICollectInteractionProbes)
    {
        auto zDKUIProbePosition = ImGui::GetWindowPos(), zDKUIProbeSize = ImGui::GetWindowSize();
        zDKUIScrollProbes[zDKUIId] = {
            {zDKUIProbePosition.x + zDKUIProbeSize.x * .5f, zDKUIProbePosition.y + zDKUIProbeSize.y * .5f},
            ImGui::GetScrollY(),
            zDKUIDestination,
            ImGui::GetScrollMaxY(),
            zDKUIProbePosition,
            {zDKUIProbePosition.x + zDKUIProbeSize.x, zDKUIProbePosition.y + zDKUIProbeSize.y}
        };
    }
}

void zDKUIBeginPanel(const char* zDKUIId, ImVec2 zDKUISize)
{
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    float zDKUIWidth = zDKUISize.x > 0 ? zDKUISize.x : ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddRectFilled(
        {zDKUITopLeft.x + zDKUIScalePixels(16), zDKUITopLeft.y + zDKUIScalePixels(16)},
        {zDKUITopLeft.x + zDKUIWidth - zDKUIScalePixels(16),
         zDKUITopLeft.y + zDKUISize.y - zDKUIScalePixels(16)},
        zDKUIPackColor({.025f, .012f, .03f, .68f})
    );
    zDKUIDrawSkin(
        ImGui::GetWindowDrawList(),
        zDKUISkinKind::zDKUIPanel,
        zDKUITopLeft,
        {zDKUITopLeft.x + zDKUIWidth, zDKUITopLeft.y + zDKUISize.y}
    );
    ImGui::PushStyleColor(ImGuiCol_ChildBg, {0, 0, 0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(20, 16));
    ImGui::SetCursorScreenPos({zDKUITopLeft.x + zDKUIScalePixels(14), zDKUITopLeft.y + zDKUIScalePixels(14)});
    zDKUISmoothChild(
        zDKUIId,
        {zDKUIWidth - zDKUIScalePixels(28), zDKUISize.y - zDKUIScalePixels(28)},
        ImGuiChildFlags_None,
        ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground
    );
}

void zDKUIEndPanel()
{
    zDKUIAddSpacing(22);
    zDKUIEndSmoothChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

struct zDKUIRevealState
{
    ImGuiStorage* zDKUIStorage;
    ImGuiID zDKUIHeightKey;
};

std::vector<zDKUIRevealState> zDKUIRevealStack;

bool zDKUIBeginReveal(const char* zDKUITitle, bool zDKUIForceOpen = false)
{
    bool zDKUIOpen = zDKUIDisclosure(zDKUITitle, zDKUIForceOpen);
    zDKUIRecordControlProbe(zDKUITitle);
    ImGuiID zDKUIId = ImGui::GetID(zDKUITitle);
    float zDKUIAmount = zDKUIAnimateValue(zDKUIId ^ 0x934901u, zDKUIOpen ? 1.f : 0.f, .085f);
    if (zDKUIAmount < .002f)
        return false;
    auto* zDKUIStorage = ImGui::GetStateStorage();
    ImGuiID zDKUIHeightKey = zDKUIId ^ 0x392814u;
    float zDKUIHeight = zDKUIStorage->GetFloat(zDKUIHeightKey, zDKUIScalePixels(240));
    zDKUIRevealStack.push_back({zDKUIStorage, zDKUIHeightKey});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(2, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * zDKUIAmount);
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1);
    ImGui::PushID(zDKUITitle);
    ImGui::BeginChild(
        "reveal",
        {0, std::max(zDKUIScalePixels(1), zDKUIHeight * zDKUIAmount)},
        ImGuiChildFlags_AlwaysUseWindowPadding,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
    );
    ImGui::BeginDisabled(!zDKUIOpen || zDKUIAmount < .9f);
    return true;
}

void zDKUIEndReveal()
{
    auto zDKUIState = zDKUIRevealStack.back();
    zDKUIRevealStack.pop_back();
    zDKUIState.zDKUIStorage->SetFloat(
        zDKUIState.zDKUIHeightKey, ImGui::GetCursorPosY() + zDKUIScalePixels(6)
    );
    ImGui::EndDisabled();
    ImGui::EndChild();
    ImGui::PopID();
    ImGui::PopStyleVar(3);
}

struct zDKUIEffectModalState
{
    zDKEffectDefinition zDKUIDraft;
    bool zDKUIIsBoss = false, zDKUIRequest = false, zDKUIClosing = false, zDKUICreating = false,
         zDKUIScrollToError = false, zDKUIChoosing = false;
    int zDKUIIndex = -1;
    float zDKUIProgress = 0, zDKUIBodyProgress = 1;
    ImVec2 zDKUIWindowSize{};
    std::string zDKUIError;
    unsigned int zDKUIRevision = 0;
} zDKUIEffectModalState;

void zDKUIRequestEffect(
    bool zDKUIIsBoss, int zDKUIIndex = -1, zDKEffectCategory zDKUICategory = zDKEffectCategory::zDKEffectBasic
)
{
    auto& zDKUIState = zDKUIEffectModalState;
    auto& zDKUIEffects = zDKUIIsBoss ? zDKUICurrentBoss.zDKEffectEffects : zDKUICurrentUnit.zDKEffectEffects;
    if (zDKUIIndex >= int(zDKUIEffects.size()))
        return;
    zDKUIState.zDKUIDraft =
        zDKUIIndex < 0 ? zDKEffectCreate(zDKUICategory, zDKUIIsBoss) : zDKUIEffects[zDKUIIndex];
    zDKUIState.zDKUIIsBoss = zDKUIIsBoss;
    zDKUIState.zDKUIIndex = zDKUIIndex;
    zDKUIState.zDKUICreating = zDKUIIndex < 0;
    zDKUIState.zDKUIScrollToError = false;
    zDKUIState.zDKUIRequest = true;
    zDKUIState.zDKUIClosing = false;
    zDKUIState.zDKUIProgress = 0;
    zDKUIState.zDKUIBodyProgress = 1;
    zDKUIState.zDKUIChoosing = false;
    zDKUIState.zDKUIError.clear();
    ++zDKUIState.zDKUIRevision;
}

void zDKUIRequestNewEffect(bool zDKUIIsBoss)
{
    zDKUIRequestEffect(zDKUIIsBoss);
    zDKUIEffectModalState.zDKUIChoosing = true;
}

bool zDKUIApplyEffect()
{
    auto& zDKUIState = zDKUIEffectModalState;
    auto& zDKUIEffects =
        zDKUIState.zDKUIIsBoss ? zDKUICurrentBoss.zDKEffectEffects : zDKUICurrentUnit.zDKEffectEffects;
    try
    {
        zDKEffectValidateEffect(zDKUIState.zDKUIDraft, zDKUIState.zDKUIIsBoss);
        if (zDKUIState.zDKUIIndex < 0)
        {
            if (zDKUIEffects.size() >= 128)
                throw std::invalid_argument("The 128-effect limit has been reached.");
            zDKUIEffects.push_back(zDKUIState.zDKUIDraft);
            zDKUIState.zDKUIIndex = int(zDKUIEffects.size()) - 1;
        }
        else
        {
            if (zDKUIState.zDKUIIndex >= int(zDKUIEffects.size()))
                throw std::invalid_argument("This effect is no longer available.");
            zDKUIEffects[zDKUIState.zDKUIIndex] = zDKUIState.zDKUIDraft;
        }
        zDKUIState.zDKUIError.clear();
        zDKUIState.zDKUIClosing = true;
        return true;
    }
    catch (const std::exception& zDKUIException)
    {
        zDKUIState.zDKUIError = zDKUIException.what();
        zDKUIState.zDKUIScrollToError = true;
        return false;
    }
}

std::filesystem::path zDKUIDataDirectory()
{
    PWSTR zDKUIFolder = nullptr;
    std::filesystem::path zDKUIDirectoryPath;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &zDKUIFolder)))
        return {};
    zDKUIDirectoryPath = zDKUIFolder;
    CoTaskMemFree(zDKUIFolder);
    zDKUIDirectoryPath /= L"DokkanDefenseLab";
    std::error_code zDKUIFileError;
    std::filesystem::create_directories(zDKUIDirectoryPath, zDKUIFileError);
    return zDKUIDirectoryPath;
}

std::filesystem::path zDKUISessionPath()
{
    return zDKUIDataDirectory() / L"session-v6.dokkan";
}

bool zDKUIRestoreSavedSession();

bool zDKUIWriteFile(const std::filesystem::path& zDKUIPath, const std::string& zDKUIData)
{
    if (zDKUIPath.empty())
        return false;
    auto zDKUITemporarySessionPath = zDKUIPath;
    zDKUITemporarySessionPath += L".tmp";
    {
        std::ofstream zDKUIOutputStream(zDKUITemporarySessionPath, std::ios::binary);
        if (!zDKUIOutputStream)
            return false;
        zDKUIOutputStream << zDKUIData;
        zDKUIOutputStream.flush();
        if (!zDKUIOutputStream)
            return false;
    }
    return MoveFileExW(
               zDKUITemporarySessionPath.c_str(),
               zDKUIPath.c_str(),
               MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
           ) != 0;
}

bool zDKUIIsAlwaysOnTop(HWND zDKUIWindow)
{
    return (GetWindowLongPtrW(zDKUIWindow, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
}

bool zDKUISetAlwaysOnTop(HWND zDKUIWindow, bool zDKUIEnabled)
{
    const BOOL zDKUIResult = SetWindowPos(
        zDKUIWindow,
        zDKUIEnabled ? HWND_TOPMOST : HWND_NOTOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );
    if (!zDKUIResult)
        return false;
    if (!zDKWinDiagnosticWindow)
        zDKUIWriteFile(
            zDKUIDataDirectory() / L"window-settings.dokkan",
            std::string("DOKKAN_WINDOW 1\n") + (zDKUIEnabled ? "1\n" : "0\n")
        );
    return true;
}

void zDKUIRestoreWindowSettings(HWND zDKUIWindow)
{
    std::ifstream zDKUIInput(zDKUIDataDirectory() / L"window-settings.dokkan");
    std::string zDKUIMagic;
    int zDKUIVersion = 0, zDKUITopmost = 0;
    if ((zDKUIInput >> zDKUIMagic >> zDKUIVersion >> zDKUITopmost) && zDKUIMagic == "DOKKAN_WINDOW" &&
        zDKUIVersion == 1 && zDKUITopmost == 1)
        zDKUISetAlwaysOnTop(zDKUIWindow, true);
}

// Geometry matches asset/ui/svg/icons; draw directly to stay sharp at every interface scale.
void zDKUIDrawSettingsGlyph(
    ImDrawList* zDKUIDrawList, ImVec2 zDKUICenter, float zDKUIRadius, ImU32 zDKUIColor, bool zDKUIPin
)
{
    if (zDKUIPin)
    {
        const ImVec2 zDKUIHead[] = {
            {zDKUICenter.x - zDKUIRadius * .5f, zDKUICenter.y - zDKUIRadius * .75f},
            {zDKUICenter.x + zDKUIRadius * .5f, zDKUICenter.y - zDKUIRadius * .75f},
            {zDKUICenter.x + zDKUIRadius * .34f, zDKUICenter.y - zDKUIRadius * .58f},
            {zDKUICenter.x + zDKUIRadius * .34f, zDKUICenter.y - zDKUIRadius * .02f},
            {zDKUICenter.x + zDKUIRadius * .64f, zDKUICenter.y + zDKUIRadius * .25f},
            {zDKUICenter.x - zDKUIRadius * .64f, zDKUICenter.y + zDKUIRadius * .25f},
            {zDKUICenter.x - zDKUIRadius * .34f, zDKUICenter.y - zDKUIRadius * .02f},
            {zDKUICenter.x - zDKUIRadius * .34f, zDKUICenter.y - zDKUIRadius * .58f}
        };
        zDKUIDrawList->AddPolyline(zDKUIHead, 8, zDKUIColor, ImDrawFlags_Closed, zDKUIScalePixels(1.8f));
        zDKUIDrawList->AddLine(
            {zDKUICenter.x, zDKUICenter.y + zDKUIRadius * .25f},
            {zDKUICenter.x, zDKUICenter.y + zDKUIRadius * .85f},
            zDKUIColor,
            zDKUIScalePixels(2)
        );
        return;
    }
    ImVec2 zDKUIPoints[32];
    for (int zDKUIIndex = 0; zDKUIIndex < 32; ++zDKUIIndex)
    {
        const float zDKUIAngle = zDKUIIndex * 6.28318530718f / 32 - 1.57079632679f;
        const float zDKUISpoke = (zDKUIIndex % 4 == 1 || zDKUIIndex % 4 == 2) ? 1.f : .74f;
        zDKUIPoints[zDKUIIndex] = {
            zDKUICenter.x + std::cos(zDKUIAngle) * zDKUIRadius * zDKUISpoke,
            zDKUICenter.y + std::sin(zDKUIAngle) * zDKUIRadius * zDKUISpoke
        };
    }
    zDKUIDrawList->AddPolyline(zDKUIPoints, 32, zDKUIColor, ImDrawFlags_Closed, zDKUIScalePixels(1.7f));
    zDKUIDrawList->AddCircle(zDKUICenter, zDKUIRadius * .32f, zDKUIColor, 24, zDKUIScalePixels(1.7f));
}

void zDKUIConfigureFullScreenModal()
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos({0, 0});
}

void zDKUIDrawModalHeader(const char* zDKUITitle)
{
    auto* zDKUIDrawList = ImGui::GetWindowDrawList();
    ImVec2 zDKUITopLeft = ImGui::GetWindowPos(), zDKUISize = ImGui::GetWindowSize(),
           zDKUIBottomRight = {zDKUITopLeft.x + zDKUISize.x, zDKUITopLeft.y + zDKUISize.y};
    zDKUIDrawList->AddRectFilled(zDKUITopLeft, zDKUIBottomRight, zDKUIPackColor({.025f, .012f, .03f, 1}));
    zDKUIDrawSkin(zDKUIDrawList, zDKUISkinKind::zDKUIPanel, zDKUITopLeft, zDKUIBottomRight, 2);
    ImVec2 zDKUIPositionH = ImGui::GetCursorScreenPos();
    float zDKUIInner = ImGui::GetContentRegionAvail().x;
    zDKUIDrawSkin(
        zDKUIDrawList,
        zDKUISkinKind::zDKUISection,
        zDKUIPositionH,
        {zDKUIPositionH.x + zDKUIInner, zDKUIPositionH.y + zDKUIScalePixels(60)},
        2
    );
    ImGui::PushFont(zDKUIHeadingFont);
    zDKUIDrawControlText(
        zDKUITitle,
        zDKUIPositionH,
        {zDKUIPositionH.x + zDKUIInner, zDKUIPositionH.y + zDKUIScalePixels(60)},
        zDKUIScalePixels(25)
    );
    ImGui::PopFont();
    ImGui::Dummy({zDKUIInner, zDKUIScalePixels(60)});
    zDKUIAddSpacing(8);
}

struct zDKUIPreset
{
    std::string zDKUIName;
    std::filesystem::path zDKUIPath;
};

std::vector<zDKUIPreset> zDKUIPresets;
int zDKUISelectedPreset = -1;
std::string zDKUIPresetName;

void zDKUIRefreshPresets()
{
    zDKUIPresets.clear();
    std::error_code zDKUIFileError;
    auto zDKUIDirectory = zDKUIDataDirectory() / L"presets";
    std::filesystem::create_directories(zDKUIDirectory, zDKUIFileError);
    for (const auto& zDKUIDirectoryEntry :
         std::filesystem::directory_iterator(zDKUIDirectory, zDKUIFileError))
        if (zDKUIDirectoryEntry.is_regular_file() && zDKUIDirectoryEntry.path().extension() == L".dunit")
        {
            std::ifstream zDKUIInputStream(zDKUIDirectoryEntry.path());
            zDKEffectUnitProfile zDKUIUnitProfileData;
            if (zDKStoreLoadUnit(zDKUIInputStream, zDKUIUnitProfileData))
                zDKUIPresets.push_back({zDKUIUnitProfileData.zDKEffectName, zDKUIDirectoryEntry.path()});
        }
    std::sort(
        zDKUIPresets.begin(),
        zDKUIPresets.end(),
        [](const zDKUIPreset& zDKUIFirstValue, const zDKUIPreset& zDKUISecondValue)
        { return zDKUIFirstValue.zDKUIName < zDKUISecondValue.zDKUIName; }
    );
    zDKUISelectedPreset = -1;
}

void zDKUIShowPresetDialog(HWND zDKUIWindowHandle, bool zDKUISave)
{
    wchar_t zDKUIFile[MAX_PATH] = L"unit.dunit";
    OPENFILENAMEW zDKUIFileDialog{sizeof(zDKUIFileDialog)};
    zDKUIFileDialog.hwndOwner = zDKUIWindowHandle;
    zDKUIFileDialog.lpstrFile = zDKUIFile;
    zDKUIFileDialog.nMaxFile = MAX_PATH;
    zDKUIFileDialog.lpstrFilter = L"Unit presets (*.dunit)\0*.dunit\0\0";
    zDKUIFileDialog.lpstrDefExt = L"dunit";
    zDKUIFileDialog.Flags =
        OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | (zDKUISave ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    if (zDKUISave ? GetSaveFileNameW(&zDKUIFileDialog) : GetOpenFileNameW(&zDKUIFileDialog))
    {
        if (zDKUISave)
        {
            try
            {
                zDKUIShowNotice(
                    zDKUIWriteFile(zDKUIFile, zDKStoreSaveUnit(zDKUICurrentUnit)) ? "Unit preset exported."
                                                                                  : "Unable to write preset."
                );
            }
            catch (const std::exception& zDKUIException)
            {
                zDKUIShowNotice(zDKUIException.what());
            }
        }
        else
        {
            std::ifstream zDKUIInputStream(zDKUIFile);
            if (zDKStoreLoadUnit(zDKUIInputStream, zDKUICurrentUnit))
            {
                zDKUISelectedPreset = -1;
                zDKUIRequestedUnitEffectIndex = -1;
                zDKUIShowNotice("Unit preset imported.");
            }
            else
                zDKUIShowNotice("Invalid unit preset.");
        }
    }
}

void zDKUIDrawPresetControls(HWND zDKUIWindowHandle)
{
    bool zDKUIRequestSave = false, zDKUIRequestImport = false, zDKUIRequestExport = false;
    if (ImGui::BeginTable("preset-toolbar", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("preset", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed, zDKUIScalePixels(44));
        ImGui::TableNextColumn();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + zDKUIScalePixels(4));
        ImGui::SetNextItemWidth(-1);
        const char* zDKUIPreview = zDKUISelectedPreset >= 0 && zDKUISelectedPreset < int(zDKUIPresets.size())
            ? zDKUIPresets[zDKUISelectedPreset].zDKUIName.c_str()
            : (ImGui::GetContentRegionAvail().x < zDKUIScalePixels(260) ? "Presets" : "Select a unit preset");
        if (zDKUIBeginCombo("##presets", zDKUIPreview))
        {
            if (zDKUIPresets.empty())
                ImGui::TextDisabled("No saved presets");
            for (int zDKUIIndex = 0; zDKUIIndex < int(zDKUIPresets.size()); ++zDKUIIndex)
            {
                ImGui::PushID(zDKUIIndex);
                if (ImGui::Selectable(
                        zDKUIPresets[zDKUIIndex].zDKUIName.c_str(), zDKUISelectedPreset == zDKUIIndex
                    ))
                {
                    std::ifstream zDKUIInputStream(zDKUIPresets[zDKUIIndex].zDKUIPath);
                    if (zDKStoreLoadUnit(zDKUIInputStream, zDKUICurrentUnit))
                    {
                        zDKUISelectedPreset = zDKUIIndex;
                        zDKUIRequestedUnitEffectIndex = -1;
                    }
                    else
                        zDKUIShowNotice("Unable to read preset.");
                }
                ImGui::PopID();
            }
            zDKUIDrawScrollbar();
            ImGui::EndCombo();
        }
        ImGui::TableNextColumn();
        if (zDKUIDrawButton("...", zDKUIScaleSize(43, 42)))
            ImGui::OpenPopup("preset-actions");
        zDKUIShowTooltip("Unit preset actions");
        if (ImGui::BeginPopup("preset-actions"))
        {
            zDKUIRequestSave = ImGui::MenuItem("Save preset");
            zDKUIRequestImport = ImGui::MenuItem("Import preset");
            zDKUIRequestExport = ImGui::MenuItem("Export preset");
            ImGui::EndPopup();
        }
        ImGui::EndTable();
    }
    if (zDKUIRequestSave)
    {
        zDKUIPresetName = zDKUICurrentUnit.zDKEffectName;
        ImGui::OpenPopup("Save unit preset");
    }
    if (zDKUIRequestImport)
        zDKUIShowPresetDialog(zDKUIWindowHandle, false);
    if (zDKUIRequestExport)
        zDKUIShowPresetDialog(zDKUIWindowHandle, true);
    zDKUIConfigureFullScreenModal();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(28, 28));
    if (ImGui::BeginPopupModal(
            "Save unit preset",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground
        ))
    {
        zDKUIDrawModalHeader("Save Unit Preset");
        zDKUISmoothChild(
            "preset-form",
            {0, std::max(zDKUIScalePixels(100), ImGui::GetContentRegionAvail().y - zDKUIScalePixels(96))},
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoBackground
        );
        zDKUIDrawNameInput("Preset name", zDKUIPresetName);
        if (ImGui::GetTime() < zDKUIStatusUntil)
            zDKUIDrawWrappedText(zDKUIStatus, zDKUIPink);
        zDKUIEndSmoothChild();
        zDKUIAddSpacing(2);
        float zDKUIHalf = (ImGui::GetContentRegionAvail().x - zDKUIScalePixels(10)) / 2;
        if (zDKUIDrawButton("Cancel", {zDKUIHalf, zDKUIScalePixels(54)}) ||
            ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        ImGui::SameLine(0, zDKUIScalePixels(10));
        if (zDKUIDrawButton("Save", {zDKUIHalf, zDKUIScalePixels(54)}, true))
        {
            if (zDKUIPresetName.find_first_not_of(" \t\r\n") == std::string::npos)
                zDKUIShowNotice("Enter a preset name.");
            else
                try
                {
                    zDKEffectUnitProfile zDKUISaved = zDKUICurrentUnit;
                    zDKUISaved.zDKEffectName = zDKUIPresetName;
                    auto zDKUIDirectory = zDKUIDataDirectory() / L"presets";
                    std::error_code zDKUIFileError;
                    std::filesystem::create_directories(zDKUIDirectory, zDKUIFileError);
                    auto zDKUIPath = zDKUIDirectory /
                        (std::to_wstring(std::chrono::system_clock::now().time_since_epoch().count()) +
                         L".dunit");
                    for (const auto& zDKUIPosition : zDKUIPresets)
                        if (zDKUIPosition.zDKUIName == zDKUIPresetName)
                        {
                            zDKUIPath = zDKUIPosition.zDKUIPath;
                            break;
                        }
                    if (zDKUIWriteFile(zDKUIPath, zDKStoreSaveUnit(zDKUISaved)))
                    {
                        zDKUICurrentUnit.zDKEffectName = zDKUIPresetName;
                        zDKUIRefreshPresets();
                        for (int zDKUIIndex = 0; zDKUIIndex < int(zDKUIPresets.size()); ++zDKUIIndex)
                            if (zDKUIPresets[zDKUIIndex].zDKUIPath == zDKUIPath)
                                zDKUISelectedPreset = zDKUIIndex;
                        ImGui::CloseCurrentPopup();
                        zDKUIShowNotice("Unit preset saved.");
                    }
                    else
                        zDKUIShowNotice("Unable to save preset.");
                }
                catch (const std::exception& zDKUIException)
                {
                    zDKUIShowNotice(zDKUIException.what());
                }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

std::string zDKUIBuildUpDescription(const zDKEffectDefinition& zDKUIEffectData)
{
    static const char* zDKUITriggers[] = {
        "",
        "with each attack performed",
        "with each Super Attack performed",
        "with each Ultra Super Attack performed"
    };
    std::string zDKUIText = "DEF +" + zDKUIFormatDecimal(zDKUIEffectData.zDKEffectValue, 1) + "% " +
        zDKUITriggers[int(zDKUIEffectData.zDKEffectBuildUpTrigger)];
    if (zDKUIEffectData.zDKEffectBuildUpLimit)
        zDKUIText += " (up to +" +
            zDKUIFormatDecimal(zDKUIEffectData.zDKEffectValue * zDKUIEffectData.zDKEffectBuildUpLimit, 1) +
            "%)";
    zDKUIText += "; " + std::to_string(zDKUIEffectData.zDKEffectBuildUpCount) + " activations already gained";
    return zDKUIText;
}

std::string zDKUIBuildEffectSummary(const zDKEffectDefinition& zDKUIEffectData)
{
    if (!zDKUIEffectData.zDKEffectName.empty() &&
        zDKUIEffectData.zDKEffectCategory != zDKEffectCategory::zDKEffectSupport)
        return zDKUIEffectData.zDKEffectName;
    if (zDKEffectIsGuard(zDKUIEffectData))
        return "Guards all attacks";
    if (zDKUIEffectData.zDKEffectBuildUpTrigger != zDKTurnBuildUpTrigger::zDKTurnNone)
        return zDKUIBuildUpDescription(zDKUIEffectData);
    const std::string zDKUIAmount = zDKUIFormatDecimal(
        zDKUIEffectData.zDKEffectValue,
        zDKUIEffectData.zDKEffectValue == std::floor(zDKUIEffectData.zDKEffectValue) ? 0 : 1
    );
    const char* zDKUIShortStats[] = {
        "DEF",
        "DR",
        "Normal DR",
        "SA DR",
        "Type DEF Boost",
        "DEF Down",
        "ATK",
        "DEF",
        "Normal ATK",
        "SA ATK",
        "DEF ignored",
        "Damage received",
        "Guard"
    };
    std::string zDKUIText = std::string(zDKUIShortStats[int(zDKUIEffectData.zDKEffectStat)]) + " " +
        (zDKUIEffectData.zDKEffectValue >= 0 ? "+" : "") + zDKUIAmount +
        (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectTypeDefenseBoost ? " Lv." : "%");
    if (zDKEffectUsesSuperCount(zDKUIEffectData))
        zDKUIText = std::string(zDKUIEffectData.zDKEffectValue < 0 ? "Lowers " : "Raises ") +
            zDKUIShortStats[int(zDKUIEffectData.zDKEffectStat)] + " by " +
            zDKUIFormatDecimal(
                        std::abs(zDKUIEffectData.zDKEffectValue),
                        zDKUIEffectData.zDKEffectValue == std::floor(zDKUIEffectData.zDKEffectValue) ? 0 : 1
            ) +
            (zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack
                 ? "% per Ultra SA ("
                 : "% per SA (") +
            std::to_string(zDKUIEffectData.zDKEffectCount) +
            (zDKUIEffectData.zDKEffectCount == 1 ? " raise)" : " raises)");
    else if (zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectActive)
        zDKUIText += " from Active Skill";
    else if (zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectEntrance)
        zDKUIText += " on Entrance Animation";
    else if (
        zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefense &&
        zDKUIEffectData.zDKEffectCategory != zDKEffectCategory::zDKEffectSupport
    )
        zDKUIText += zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectStartOfTurn
            ? " at start of turn"
            : zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenAttacking
            ? " when attacking"
            : "";
    if (zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectSupport)
        zDKUIText += zDKUIEffectData.zDKEffectName.empty() ? " from ally support"
                                                           : " from " + zDKUIEffectData.zDKEffectName;
    return zDKUIText;
}

std::string zDKUIBuildEffectDescription(const zDKEffectDefinition& zDKUIEffectData, bool zDKUIIsBoss)
{
    const std::string zDKUIAmount = zDKUIFormatDecimal(
        zDKUIEffectData.zDKEffectValue,
        zDKUIEffectData.zDKEffectValue == std::floor(zDKUIEffectData.zDKEffectValue) ? 0 : 1
    );
    std::string zDKUIText;
    if (zDKEffectIsGuard(zDKUIEffectData))
        zDKUIText = "Guards all attacks";
    else if (zDKUIEffectData.zDKEffectBuildUpTrigger != zDKTurnBuildUpTrigger::zDKTurnNone)
        zDKUIText = zDKUIBuildUpDescription(zDKUIEffectData);
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDamageReduction)
        zDKUIText = "Reduces damage received by " + zDKUIAmount + "%";
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectNormalDamageReduction)
        zDKUIText = "Reduces damage received from normal attacks by " + zDKUIAmount + "%";
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectSuperDamageReduction)
        zDKUIText = "Reduces damage received from Super Attacks by " + zDKUIAmount + "%";
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefenseDebuff)
        zDKUIText = "Lowers DEF by " + zDKUIAmount + "%";
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDamageReceived)
        zDKUIText = "Increases damage received by " + zDKUIAmount + "% after DEF and guard";
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectPenetration)
        zDKUIText = "Ignores " + zDKUIAmount + "% of DEF";
    else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectTypeDefenseBoost)
        zDKUIText = "Type DEF Boost +" + zDKUIAmount + " Lv.";
    else if (
        zDKEffectUsesSuperCount(zDKUIEffectData) &&
        (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefense ||
         zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectAttack ||
         zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectBossDefense)
    )
        zDKUIText = std::string(zDKUIEffectData.zDKEffectValue < 0 ? "Lowers " : "Raises ") +
            (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectAttack ? "ATK" : "DEF") + " by " +
            zDKUIFormatDecimal(
                        std::abs(zDKUIEffectData.zDKEffectValue),
                        zDKUIEffectData.zDKEffectValue == std::floor(zDKUIEffectData.zDKEffectValue) ? 0 : 1
            ) +
            "%";
    else
        zDKUIText = std::string(zDKEffectStatNames[int(zDKUIEffectData.zDKEffectStat)]) + " " +
            (zDKUIEffectData.zDKEffectValue >= 0 ? "+" : "") + zDKUIAmount + "%";
    if (zDKEffectUsesSuperCount(zDKUIEffectData))
        zDKUIText += std::string(
                         zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack
                             ? " per Ultra Super Attack ("
                             : " per Super Attack ("
                     ) +
            std::to_string(zDKUIEffectData.zDKEffectCount) + " raises applied)";
    if (!zDKUIIsBoss && zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefense &&
        !zDKEffectIsGuard(zDKUIEffectData) && !zDKEffectUsesSuperCount(zDKUIEffectData) &&
        zDKUIEffectData.zDKEffectBuildUpTrigger == zDKTurnBuildUpTrigger::zDKTurnNone)
    {
        if (zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectStartOfTurn)
            zDKUIText += " at start of turn";
        else if (zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenAttacking)
            zDKUIText += " when attacking";
        else
            zDKUIText += " (" + std::string(zDKEffectGroupNames[int(zDKUIEffectData.zDKEffectGroup)]) + ")";
    }
    if (zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectSupport)
        zDKUIText += zDKUIEffectData.zDKEffectName.empty() ? " from an ally"
                                                           : " from " + zDKUIEffectData.zDKEffectName;
    if (zDKUIEffectData.zDKEffectFirstTurn > 1)
        zDKUIText += "; starting from battle turn " + std::to_string(zDKUIEffectData.zDKEffectFirstTurn);
    if (zDKUIEffectData.zDKEffectLastTurn > 0)
        zDKUIText += "; through battle turn " + std::to_string(zDKUIEffectData.zDKEffectLastTurn);
    if (zDKUIEffectData.zDKEffectHealthRule != zDKEffectHealthRule::zDKEffectAlways)
        zDKUIText += std::string("; when ") + (zDKUIIsBoss ? "enemy HP" : "HP") + " is " +
            zDKUIFormatDecimal(
                         zDKUIEffectData.zDKEffectHealthThresholdPercent,
                         zDKUIEffectData.zDKEffectHealthThresholdPercent ==
                                 std::floor(zDKUIEffectData.zDKEffectHealthThresholdPercent)
                             ? 0
                             : 1
            ) +
            "% or " +
            (zDKUIEffectData.zDKEffectHealthRule == zDKEffectHealthRule::zDKEffectBelow ? "less" : "more");
    if (zDKUIEffectData.zDKEffectTiming != zDKEffectAttackTiming::zDKEffectAlways)
        zDKUIText += std::string("; ") +
            (zDKUIEffectData.zDKEffectTiming == zDKEffectAttackTiming::zDKEffectBeforeAttack ? "before"
                                                                                             : "after") +
            " the unit attacks";
    if (zDKUIEffectData.zDKEffectMultiplicative)
        zDKUIText += "; separate multiplier";
    return zDKUIText + ".";
}

std::vector<zDKEffectCategory> zDKUIEffectCategories(bool zDKUIIsBoss)
{
    if (zDKUIIsBoss)
        return {
            zDKEffectCategory::zDKEffectBasic,
            zDKEffectCategory::zDKEffectActive,
            zDKEffectCategory::zDKEffectEntrance,
            zDKEffectCategory::zDKEffectSuperAttackStack
        };
    return {
        zDKEffectCategory::zDKEffectEntrance,
        zDKEffectCategory::zDKEffectBasic,
        zDKEffectCategory::zDKEffectWhenAttacking,
        zDKEffectCategory::zDKEffectWhenReceiving,
        zDKEffectCategory::zDKEffectSuperAttackStack,
        zDKEffectCategory::zDKEffectUltraSuperAttack,
        zDKEffectCategory::zDKEffectSupport,
        zDKEffectCategory::zDKEffectActive
    };
}

void zDKUIDrawEffectFields(
    zDKEffectDefinition& zDKUIEffectData, bool zDKUIIsBoss, bool zDKUIRevealConditions = false
)
{
    zDKUIDrawNameInput(
        zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectSupport
            ? "Supporting ally (optional)"
            : "Effect name (optional)",
        zDKUIEffectData.zDKEffectName
    );
    const auto zDKUICategories = zDKUIEffectCategories(zDKUIIsBoss);
    std::vector<const char*> zDKUICategoryLabels;
    int zDKUICategoryIndex = 0;
    for (int zDKUIIndex = 0; zDKUIIndex < int(zDKUICategories.size()); ++zDKUIIndex)
    {
        zDKUICategoryLabels.push_back(zDKEffectCategoryNames[int(zDKUICategories[zDKUIIndex])]);
        if (zDKUICategories[zDKUIIndex] == zDKUIEffectData.zDKEffectCategory)
            zDKUICategoryIndex = zDKUIIndex;
    }
    if (zDKUIDrawComboInput(
            "Effect type", zDKUICategoryIndex, zDKUICategoryLabels.data(), int(zDKUICategoryLabels.size())
        ))
    {
        zDKUIEffectData.zDKEffectCategory = zDKUICategories[zDKUICategoryIndex];
        zDKUIEffectData.zDKEffectGroup =
            zDKEffectCreate(zDKUIEffectData.zDKEffectCategory, zDKUIIsBoss).zDKEffectGroup;
        if (zDKEffectIsGuard(zDKUIEffectData))
            zDKUIEffectData.zDKEffectCount =
                zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectSuperAttackStack ||
                    zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack
                ? 0
                : 1;
        else if (!zDKEffectUsesSuperCount(zDKUIEffectData))
            zDKUIEffectData.zDKEffectCount = std::max(1, zDKUIEffectData.zDKEffectCount);
    }
    {
        std::vector<zDKEffectStat> zDKUIAllowed = zDKUIIsBoss
            ? std::vector<
                  zDKEffectStat>{zDKEffectStat::zDKEffectAttack, zDKEffectStat::zDKEffectBossDefense, zDKEffectStat::zDKEffectNormalAttack, zDKEffectStat::zDKEffectSuperAttack, zDKEffectStat::zDKEffectPenetration}
            : std::vector<zDKEffectStat>{
                  zDKEffectStat::zDKEffectDefense,
                  zDKEffectStat::zDKEffectDamageReduction,
                  zDKEffectStat::zDKEffectNormalDamageReduction,
                  zDKEffectStat::zDKEffectSuperDamageReduction,
                  zDKEffectStat::zDKEffectTypeDefenseBoost,
                  zDKEffectStat::zDKEffectDefenseDebuff,
                  zDKEffectStat::zDKEffectDamageReceived
              };
        if (!zDKUIIsBoss)
            zDKUIAllowed.push_back(zDKEffectStat::zDKEffectGuard);
        zDKUIDrawLabel("Skill effect");
        ImGui::SetNextItemWidth(-1);
        if (zDKUIBeginCombo("##stat", zDKEffectStatNames[int(zDKUIEffectData.zDKEffectStat)]))
        {
            for (auto zDKUIStatisticIndex : zDKUIAllowed)
            {
                if (ImGui::Selectable(
                        zDKEffectStatNames[int(zDKUIStatisticIndex)],
                        zDKUIEffectData.zDKEffectStat == zDKUIStatisticIndex
                    ))
                {
                    zDKUIEffectData.zDKEffectStat = zDKUIStatisticIndex;
                    if (zDKEffectIsGuard(zDKUIEffectData))
                    {
                        zDKUIEffectData.zDKEffectMultiplicative = false;
                        zDKUIEffectData.zDKEffectValue = 0;
                        zDKUIEffectData.zDKEffectCount = zDKUIEffectData.zDKEffectCategory ==
                                    zDKEffectCategory::zDKEffectSuperAttackStack ||
                                zDKUIEffectData.zDKEffectCategory ==
                                    zDKEffectCategory::zDKEffectUltraSuperAttack
                            ? 0
                            : 1;
                    }
                }
                zDKUIRecordControlProbe("stat-option-" + std::to_string(int(zDKUIStatisticIndex)));
            }
            zDKUIDrawScrollbar();
            ImGui::EndCombo();
        }
        zDKUIRecordControlProbe("skill-effect");
        if (!zDKUIIsBoss && zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectSupport)
        {
            static const char* zDKUISupportNames[] = {
                "Start-of-turn passive", "Mid-battle passive", "Super Attack effect", "Active Skill"
            };
            static const zDKEffectDefensePhase zDKUISupportPhases[] = {
                zDKEffectDefensePhase::zDKEffectStartOfTurn,
                zDKEffectDefensePhase::zDKEffectMidBattle,
                zDKEffectDefensePhase::zDKEffectSuperAttack,
                zDKEffectDefensePhase::zDKEffectActive
            };
            int zDKUISource = 0;
            for (int zDKUIIndex = 0; zDKUIIndex < 4; ++zDKUIIndex)
                if (zDKUIEffectData.zDKEffectGroup == zDKUISupportPhases[zDKUIIndex])
                    zDKUISource = zDKUIIndex;
            if (zDKUIDrawComboInput("Support source", zDKUISource, zDKUISupportNames, 4))
                zDKUIEffectData.zDKEffectGroup = zDKUISupportPhases[zDKUISource];
            zDKUIDrawWrappedText("Ally DEF support adds to the same phase as your unit's boost.");
            zDKUIShowTooltip(
                "Start-of-turn DEF +200% and ally DEF +40% use a 3.40 multiplier. "
                "An ally's SA raise adds to SA effects; Active Skill support adds to Active Skills. "
                "Enter only support that is active for this unit."
            );
        }
        else if (
            !zDKUIIsBoss && zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectWhenReceiving
        )
        {
            static const char* zDKUIReceivingNames[] = {
                "Any attack", "Super Attacks only", "Normal attacks only"
            };
            int zDKUIReceiving = std::clamp(int(zDKUIEffectData.zDKEffectGroup) - 8, 0, 2);
            if (zDKUIDrawComboInput("Incoming attack", zDKUIReceiving, zDKUIReceivingNames, 3))
                zDKUIEffectData.zDKEffectGroup = zDKEffectDefensePhase(zDKUIReceiving + 8);
        }
        else if (
            !zDKUIIsBoss && zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefense &&
            !zDKEffectUsesSuperCount(zDKUIEffectData) &&
            zDKUIEffectData.zDKEffectCategory != zDKEffectCategory::zDKEffectWhenAttacking
        )
        {
            int zDKUIDefensePhaseIndex = int(zDKUIEffectData.zDKEffectGroup);
            zDKUIDrawComboInput("DEF boost phase", zDKUIDefensePhaseIndex, zDKEffectGroupNames, 11);
            zDKUIEffectData.zDKEffectGroup = zDKEffectDefensePhase(zDKUIDefensePhaseIndex);
            zDKUIShowTooltip(
                "Phase 2 boosts add together. Receiving buffs apply before or after your unit attacks; "
                "Super-only buffs apply only against an enemy Super Attack."
            );
        }
        if (zDKUIIsBoss && zDKUIEffectData.zDKEffectStat != zDKEffectStat::zDKEffectPenetration)
        {
            static const char* zDKUIBossPhases[] = {
                "Start of turn (Phase 1)", "Mid-battle (Phase 2)", "Domain / Territory", "Support Item"
            };
            static const zDKEffectDefensePhase zDKUIBossPhaseValues[] = {
                zDKEffectDefensePhase::zDKEffectStartOfTurn,
                zDKEffectDefensePhase::zDKEffectMidBattle,
                zDKEffectDefensePhase::zDKEffectDomain,
                zDKEffectDefensePhase::zDKEffectItem
            };
            int zDKUIBossPhase = zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectMidBattle
                ? 1
                : zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectDomain ? 2
                : zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectItem   ? 3
                                                                                           : 0;
            if (zDKUIDrawComboInput("Enemy boost phase", zDKUIBossPhase, zDKUIBossPhases, 4))
                zDKUIEffectData.zDKEffectGroup = zDKUIBossPhaseValues[zDKUIBossPhase];
            zDKUIShowTooltip(
                "Boosts add within one phase; Phase 1, Phase 2 and Domain multiply. Enter the active buildup "
                "amount; encounter Turn and HP conditions remain editable."
            );
        }
        const bool zDKUIBuildUpEligible = !zDKUIIsBoss &&
            zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefense &&
            zDKUIEffectData.zDKEffectCategory != zDKEffectCategory::zDKEffectSupport &&
            (zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenAttacking ||
             zDKUIEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectMidBattle);
        if (zDKUIBuildUpEligible)
        {
            int zDKUITrigger = int(zDKUIEffectData.zDKEffectBuildUpTrigger);
            if (zDKUIDrawComboInput("DEF activation", zDKUITrigger, zDKTurnBuildUpNames, 4))
            {
                zDKUIEffectData.zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger(zDKUITrigger);
                if (!zDKUITrigger)
                    zDKUIEffectData.zDKEffectBuildUpCount = zDKUIEffectData.zDKEffectBuildUpLimit = 0;
            }
        }
        else
        {
            zDKUIEffectData.zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger::zDKTurnNone;
            zDKUIEffectData.zDKEffectBuildUpCount = zDKUIEffectData.zDKEffectBuildUpLimit = 0;
        }
        if (zDKEffectIsGuard(zDKUIEffectData))
            zDKUIDrawWrappedText("Guards all attacks while this skill effect is active.");
        else if (zDKUIEffectData.zDKEffectStat == zDKEffectStat::zDKEffectTypeDefenseBoost)
        {
            zDKUIDrawTypeDefenseBoost(zDKUIEffectData.zDKEffectValue);
            if (zDKEffectUsesSuperCount(zDKUIEffectData))
            {
                zDKUIDrawLabel("SA stacks applied");
                zDKUIDrawIntegerField("count", zDKUIEffectData.zDKEffectCount);
            }
        }
        else if (zDKUIEffectData.zDKEffectBuildUpTrigger != zDKTurnBuildUpTrigger::zDKTurnNone)
        {
            zDKUIDrawNumberInput("DEF per activation (%)", zDKUIEffectData.zDKEffectValue, "%.1f");
            zDKUIDrawFieldPair(
                "Activations already gained",
                "Maximum activations",
                [&] { zDKUIDrawIntegerField("build-count", zDKUIEffectData.zDKEffectBuildUpCount); },
                [&] { zDKUIDrawIntegerField("build-limit", zDKUIEffectData.zDKEffectBuildUpLimit); }
            );
            zDKUIDrawWrappedText(
                "Use 0 for an uncapped maximum. The attack plan adds matching activations up to this limit."
            );
        }
        else if (zDKEffectUsesSuperCount(zDKUIEffectData))
            zDKUIDrawFieldPair(
                zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack
                    ? "Boost per Ultra SA (%)"
                    : "Boost per SA effect (%)",
                "Raises already active",
                [&] { zDKUIDrawNumberField("amount", zDKUIEffectData.zDKEffectValue, "%.1f"); },
                [&] { zDKUIDrawIntegerField("count", zDKUIEffectData.zDKEffectCount); }
            );
        else
            zDKUIDrawNumberInput("Effect amount (%)", zDKUIEffectData.zDKEffectValue, "%.1f");
        if (zDKEffectUsesSuperCount(zDKUIEffectData))
            zDKUIDrawWrappedText(
                "For the turn preview, enter raises already active before attacking. Use 0 for a fresh SA "
                "effect."
            );
        if (!zDKEffectIsGuard(zDKUIEffectData) &&
            zDKUIEffectData.zDKEffectStat != zDKEffectStat::zDKEffectTypeDefenseBoost &&
            zDKUIEffectData.zDKEffectStat != zDKEffectStat::zDKEffectPenetration)
        {
            zDKUICheckbox("Multiplicative", &zDKUIEffectData.zDKEffectMultiplicative);
            zDKUIShowTooltip(
                "Off: add boosts in the same phase of the calc. On: apply this boost as a separate "
                "multiplier. For DR, multiply the damage remaining after other reductions."
            );
        }
    }
    if (zDKUIBeginReveal("Conditions", zDKUIRevealConditions))
    {
        zDKUIDrawFieldPair(
            "Starting from turn",
            "Last turn (0 = no limit)",
            [&] { zDKUIDrawIntegerField("from", zDKUIEffectData.zDKEffectFirstTurn); },
            [&] { zDKUIDrawIntegerField("through", zDKUIEffectData.zDKEffectLastTurn); }
        );
        zDKUIShowTooltip(
            "Battle turns, counted from the start of battle. For an Entrance Animation, enter "
            "its actual active turn range."
        );
        static const char* zDKUIRules[] = {"No HP condition", "HP is X% or less", "HP is X% or more"};
        int zDKUIRule = int(zDKUIEffectData.zDKEffectHealthRule);
        zDKUIDrawComboInput(zDKUIIsBoss ? "Enemy HP condition" : "HP condition", zDKUIRule, zDKUIRules, 3);
        zDKUIEffectData.zDKEffectHealthRule = zDKEffectHealthRule(zDKUIRule);
        zDKUIShowTooltip(
            "Uses the HP currently entered in the calculator. For a start-of-turn HP condition, "
            "enter the HP at that turn's start."
        );
        if (zDKUIRule)
            zDKUIDrawNumberInput("HP threshold (%)", zDKUIEffectData.zDKEffectHealthThresholdPercent, "%.1f");
        static const char* zDKUITiming[] = {
            "Before and after attacking", "Before the unit attacks", "After the unit attacks"
        };
        int zDKUITypeIndex = int(zDKUIEffectData.zDKEffectTiming);
        zDKUIDrawComboInput("Attack timing", zDKUITypeIndex, zDKUITiming, 3);
        zDKUIEffectData.zDKEffectTiming = zDKEffectAttackTiming(zDKUITypeIndex);
        zDKUIEndReveal();
    }
    zDKUIAddSpacing(3);
    zDKUIDrawFontText(zDKUICaptionFont, zDKUIMuted, "SKILL EFFECT PREVIEW");
    zDKUIDrawWrappedText(zDKUIBuildEffectDescription(zDKUIEffectData, zDKUIIsBoss), zDKUIWhite);
}

void zDKUIDrawEffectModal()
{
    auto& zDKUIState = zDKUIEffectModalState;
    if (zDKUIState.zDKUIRequest)
    {
        ImGui::OpenPopup("Skill effect###effect-modal");
        zDKUIState.zDKUIRequest = false;
    }
    if (!ImGui::IsPopupOpen("Skill effect###effect-modal"))
        return;
    float zDKUIDeltaSeconds = std::min(ImGui::GetIO().DeltaTime, .05f);
    zDKUIState.zDKUIProgress = zDKUIAnimationsEnabled
        ? std::clamp(
              zDKUIState.zDKUIProgress +
                  (zDKUIState.zDKUIClosing ? -zDKUIDeltaSeconds / .14f : zDKUIDeltaSeconds / .22f),
              0.f,
              1.f
          )
        : (zDKUIState.zDKUIClosing ? 0.f : 1.f);
    float zDKUIEased =
        zDKUIState.zDKUIProgress * zDKUIState.zDKUIProgress * (3 - 2 * zDKUIState.zDKUIProgress);
    ImVec2 zDKUIDisplay = ImGui::GetIO().DisplaySize;
    ImVec2 zDKUISize = zDKUIDisplay;
    ImGui::SetNextWindowSize(zDKUISize);
    ImGui::SetNextWindowPos({0, zDKUIScalePixels(18) * (1 - zDKUIEased)});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(28, 28));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::max(.001f, zDKUIEased));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, {.012f, .005f, .02f, .78f * zDKUIEased});
    if (ImGui::BeginPopupModal(
            "Skill effect###effect-modal",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        ))
    {
        zDKUIState.zDKUIWindowSize = ImGui::GetWindowSize();
        std::string zDKUITitle = zDKUIState.zDKUIChoosing
            ? (zDKUIState.zDKUIIsBoss ? "Add Enemy Effect" : "Add Unit Effect")
            : std::string(zDKUIState.zDKUICreating ? "Add " : "Edit ") +
                zDKEffectCategoryNames[int(zDKUIState.zDKUIDraft.zDKEffectCategory)];
        zDKUIDrawModalHeader(zDKUITitle.c_str());
        zDKUIAddSpacing(5);
        zDKUIDrawFontText(
            zDKUICaptionFont, zDKUIMuted, zDKUIState.zDKUIIsBoss ? "ENEMY SKILL EFFECT" : "UNIT SKILL EFFECT"
        );
        zDKUIAddSpacing(6);
        ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1);
        ImGui::BeginDisabled(zDKUIState.zDKUIClosing);
        float zDKUIBodyHeight =
            std::max(zDKUIScalePixels(100), ImGui::GetContentRegionAvail().y - zDKUIScalePixels(96));
        ImGui::PushID(zDKUIState.zDKUIRevision);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(12, 10));
        zDKUISmoothChild(
            zDKUIState.zDKUIChoosing ? "effect-picker" : "effect-form",
            {0, zDKUIBodyHeight},
            ImGuiChildFlags_AlwaysUseWindowPadding,
            ImGuiWindowFlags_NoBackground,
            zDKUIState.zDKUIScrollToError
        );
        zDKUIState.zDKUIScrollToError = false;
        if (zDKUIState.zDKUIChoosing)
        {
            zDKUIDrawLabel("Choose an effect type");
            zDKUIAddSpacing(8);
            for (auto zDKUICategory : zDKUIEffectCategories(zDKUIState.zDKUIIsBoss))
            {
                bool zDKUIChosen = zDKUIDrawButton(
                    zDKEffectCategoryNames[int(zDKUICategory)],
                    {-1, zDKUIScalePixels(52)},
                    false,
                    zDKUISkinKind::zDKUIEffectRow
                );
                zDKUIRecordControlProbe("add-category-" + std::to_string(int(zDKUICategory)));
                if (zDKUIChosen)
                {
                    zDKUIState.zDKUIDraft = zDKEffectCreate(zDKUICategory, zDKUIState.zDKUIIsBoss);
                    zDKUIState.zDKUIChoosing = false;
                    zDKUIState.zDKUIBodyProgress = 0;
                    ++zDKUIState.zDKUIRevision;
                    break;
                }
            }
        }
        else
        {
            zDKUIState.zDKUIBodyProgress = zDKUIAnimationsEnabled
                ? std::min(1.f, zDKUIState.zDKUIBodyProgress + zDKUIDeltaSeconds / .16f)
                : 1;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * zDKUIState.zDKUIBodyProgress);
            if (!zDKUIState.zDKUIError.empty())
            {
                zDKUIDrawWrappedText(zDKUIState.zDKUIError, zDKUIPink);
                zDKUIAddSpacing(6);
            }
            bool zDKUIConditioned =
                zDKUIState.zDKUIDraft.zDKEffectCategory == zDKEffectCategory::zDKEffectEntrance ||
                zDKUIState.zDKUIDraft.zDKEffectFirstTurn > 1 || zDKUIState.zDKUIDraft.zDKEffectLastTurn > 0 ||
                zDKUIState.zDKUIDraft.zDKEffectHealthRule != zDKEffectHealthRule::zDKEffectAlways ||
                zDKUIState.zDKUIDraft.zDKEffectTiming != zDKEffectAttackTiming::zDKEffectAlways;
            zDKUIDrawEffectFields(
                zDKUIState.zDKUIDraft, zDKUIState.zDKUIIsBoss, ImGui::IsWindowAppearing() && zDKUIConditioned
            );
            zDKUIAddSpacing(20);
            ImGui::PopStyleVar();
        }
        zDKUIEndSmoothChild();
        ImGui::PopStyleVar();
        ImGui::PopID();
        zDKUIAddSpacing(2);
        float zDKUIWidth = zDKUIState.zDKUIChoosing
            ? ImGui::GetContentRegionAvail().x
            : (ImGui::GetContentRegionAvail().x - zDKUIScalePixels(10)) / 2;
        if (zDKUIDrawButton("Cancel", {zDKUIWidth, zDKUIScalePixels(54)}))
            zDKUIState.zDKUIClosing = true;
        if (!zDKUIState.zDKUIChoosing)
        {
            ImGui::SameLine(0, zDKUIScalePixels(10));
            if (zDKUIDrawButton(
                    "Apply", {zDKUIWidth, zDKUIScalePixels(54)}, true, zDKUISkinKind::zDKUIAction
                ))
                zDKUIApplyEffect();
        }
        ImGui::EndDisabled();
        ImGui::PopStyleVar();
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            zDKUIState.zDKUIClosing = true;
        if (zDKUIState.zDKUIClosing && zDKUIState.zDKUIProgress <= 0)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void zDKUIDrawEffectsEditor(std::vector<zDKEffectDefinition>& zDKUIEffects, bool zDKUIIsBoss)
{
    int& zDKUIExpanded = zDKUIIsBoss ? zDKUIRequestedBossEffectIndex : zDKUIRequestedUnitEffectIndex;
    if (zDKUIExpanded >= 0)
    {
        zDKUIRequestEffect(zDKUIIsBoss, zDKUIExpanded);
        zDKUIExpanded = -1;
    }
    int zDKUIEnabled = 0;
    for (const auto& zDKUIEffect : zDKUIEffects)
        if (zDKEffectIsActive(
                zDKUIEffect,
                zDKUICurrentBattle,
                zDKUIIsBoss ? zDKUICurrentBoss.zDKEffectHealthPercent
                            : zDKEffectTeamHealthPercent(zDKUICurrentBattle),
                zDKUISelectedAttackKind == 1,
                zDKUIIsBoss
            ))
            ++zDKUIEnabled;
    ImGui::AlignTextToFramePadding();
    zDKUIDrawFontText(
        zDKUICaptionFont,
        zDKUIMuted,
        std::to_string(zDKUIEnabled) + " of " + std::to_string(zDKUIEffects.size()) + " effects active"
    );
    zDKUIShowTooltip("Skill effects active under the current turn, HP and attack timing.");
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - zDKUIScalePixels(110));
    ImGui::BeginDisabled(zDKUIEffects.size() >= 128);
    if (zDKUIDrawButton("+ Add", zDKUIScaleSize(110, 50), false, zDKUISkinKind::zDKUIAction))
        zDKUIRequestNewEffect(zDKUIIsBoss);
    ImGui::EndDisabled();
    int zDKUIRemove = -1;
    std::vector<int> zDKUIOrder;
    if (zDKUIIsBoss)
        for (int zDKUIIndex = 0; zDKUIIndex < int(zDKUIEffects.size()); ++zDKUIIndex)
            zDKUIOrder.push_back(zDKUIIndex);
    else
        zDKUIOrder = zDKEffectDisplayOrder(zDKUIEffects);
    int zDKUILastSection = -1, zDKUILastSuperKind = -1;
    for (int zDKUIIndex : zDKUIOrder)
    {
        auto& zDKUIEffectData = zDKUIEffects[zDKUIIndex];
        if (!zDKUIIsBoss)
        {
            const int zDKUISection = int(zDKEffectGetSection(zDKUIEffectData));
            if (zDKUISection != zDKUILastSection)
            {
                zDKUIDrawDivider();
                zDKUIDrawLabel(zDKEffectSectionNames[zDKUISection]);
                zDKUIRecordControlProbe("effect-section-" + std::to_string(zDKUISection));
                zDKUIAddSpacing(5);
                zDKUILastSection = zDKUISection;
            }
            if (zDKUISection == int(zDKEffectSection::zDKEffectSuperAttack))
            {
                const int zDKUISuperKind =
                    zDKUIEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack ? 1 : 0;
                if (zDKUISuperKind != zDKUILastSuperKind)
                {
                    zDKUIDrawFontText(
                        zDKUICaptionFont, zDKUIMuted, zDKUISuperKind ? "Ultra Super Attack" : "Super Attack"
                    );
                    zDKUILastSuperKind = zDKUISuperKind;
                }
            }
        }
        ImGui::PushID(zDKUIIndex);
        bool zDKUIIsActive = zDKEffectIsActive(
            zDKUIEffectData,
            zDKUICurrentBattle,
            zDKUIIsBoss ? zDKUICurrentBoss.zDKEffectHealthPercent
                        : zDKEffectTeamHealthPercent(zDKUICurrentBattle),
            zDKUISelectedAttackKind == 1,
            zDKUIIsBoss
        );
        if (ImGui::BeginTable("effect-header", 3, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("enabled", ImGuiTableColumnFlags_WidthFixed, zDKUIScalePixels(28));
            ImGui::TableSetupColumn("effect", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("remove", ImGuiTableColumnFlags_WidthFixed, zDKUIScalePixels(36));
            ImGui::TableNextColumn();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + zDKUIScalePixels(12));
            zDKUICheckbox("##enabled", &zDKUIEffectData.zDKEffectEnabled);
            zDKUIShowTooltip("Enable or disable this effect.");
            ImGui::TableNextColumn();
            std::string zDKUITitle = zDKUIBuildEffectSummary(zDKUIEffectData) + "###effect";
            ImGui::PushStyleColor(ImGuiCol_Text, zDKUIIsActive ? zDKUIWhite : zDKUIMuted);
            if (zDKUIDrawButton(
                    zDKUITitle.c_str(), {-1, zDKUIScalePixels(54)}, false, zDKUISkinKind::zDKUIEffectRow
                ))
                zDKUIRequestEffect(zDKUIIsBoss, zDKUIIndex);
            zDKUIRecordControlProbe("effect-row-" + std::to_string(zDKUIIndex));
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(zDKUIScalePixels(330));
                zDKUIDrawWrappedText(zDKUIBuildEffectDescription(zDKUIEffectData, zDKUIIsBoss), zDKUIWhite);
                zDKUIDrawWrappedText(
                    zDKUIIsActive ? "Active with the current turn, HP and attack timing."
                                  : "Inactive with the current turn, HP or attack timing."
                );
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            ImGui::TableNextColumn();
            if (zDKUIDrawButton("×", {zDKUIScalePixels(34), zDKUIScalePixels(54)}))
                zDKUIRemove = zDKUIIndex;
            zDKUIShowTooltip("Remove effect.");
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
    if (zDKUIRemove >= 0)
        zDKUIEffects.erase(zDKUIEffects.begin() + zDKUIRemove);
}

void zDKUIDrawUnitPanel(HWND zDKUIWindowHandle, ImVec2 zDKUISize)
{
    if (zDKUIDisplayedSection == 1)
    {
        zDKUIBeginPanel("unit-effects-panel", zDKUISize);
        zDKUIDrawEffectsEditor(zDKUICurrentUnit.zDKEffectEffects, false);
        if (zDKUICurrentUnit.zDKEffectEffects.empty())
        {
            zDKUIAddSpacing(14);
            zDKUIDrawWrappedText("No skill effects added.");
        }
    }
    else if (zDKUIDisplayedSection == 2)
    {
        zDKUIBeginPanel("unit-setup-panel", zDKUISize);
        zDKUIDrawPresetControls(zDKUIWindowHandle);
        zDKUIDrawDivider();
        zDKUIDrawNumberPair(
            "Max HP", zDKUICurrentBattle.zDKEffectMaxHP, "Remaining HP", zDKUICurrentBattle.zDKEffectCurrentHP
        );
        zDKUIDrawDivider();
        zDKUIDrawTypeDefenseBoost(zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel);
        zDKUIAddSpacing();
        zDKUICurrentUnit.zDKEffectRoundDownDefense = true;
        zDKUIDrawWrappedText("Stats round down after every step.");
    }
    else
    {
        zDKUIBeginPanel("unit-stats-panel", zDKUISize);
        zDKUIDrawNameInput("Character name", zDKUICurrentUnit.zDKEffectName);
        zDKUIDrawTypeAndClass(zDKUICurrentUnit.zDKEffectElementType, zDKUICurrentUnit.zDKEffectCombatClass);
        zDKUICheckbox("Manual DEF", &zDKUICurrentUnit.zDKEffectUseManualDefense);
        zDKUIShowTooltip(
            "Enter a known DEF stat instead of building it from Leader Skill, passives, links "
            "and SA effects. Guard and DR still apply."
        );
        if (zDKUICurrentUnit.zDKEffectUseManualDefense)
            zDKUIDrawNumberInput("DEF stat", zDKUICurrentUnit.zDKEffectManualDefense);
        else
            zDKUIDrawNumberPair(
                "Base DEF",
                zDKUICurrentUnit.zDKEffectBaseDefense,
                "Leader Skill DEF (%)",
                zDKUICurrentUnit.zDKEffectLeaderSkillPercent
            );
    }
    zDKUIEndPanel();
}

void zDKUIDrawBossPanel(ImVec2 zDKUISize)
{
    if (zDKUIDisplayedSection == 1)
    {
        zDKUIBeginPanel("boss-effects-panel", zDKUISize);
        zDKUIDrawEffectsEditor(zDKUICurrentBoss.zDKEffectEffects, true);
        if (zDKUICurrentBoss.zDKEffectEffects.empty())
        {
            zDKUIAddSpacing(14);
            zDKUIDrawWrappedText("No enemy skill effects added.");
        }
    }
    else if (zDKUIDisplayedSection == 2)
    {
        zDKUIBeginPanel("boss-skills-panel", zDKUISize);
        zDKUIDrawNumberInput("Enemy DEF", zDKUICurrentBoss.zDKEffectBaseDefense);
        zDKUIDrawNumberInput(
            "Manual ATK adjustment (%)",
            zDKUICurrentBoss.zDKEffectStats.zDKCalcAttackModifierPercent,
            "%.1f",
            "Advanced net multiplier. For in-game ATK Down use the source-specific fields below; do not "
            "enter the same effect twice."
        );
        zDKUIDrawNumberInput(
            "Passive ATK Down (%)",
            zDKUICurrentBoss.zDKEffectStats.zDKCalcPassiveAttackDownPercent,
            "%.1f",
            "Sum active passive ATK Down effects. Enter 0 if the boss is immune."
        );
        zDKUIDrawNumberInput(
            "Item ATK Down (%)",
            zDKUICurrentBoss.zDKEffectStats.zDKCalcItemAttackDownPercent,
            "%.1f",
            "An independent multiplier on enemy ATK. Enter only effects allowed by this event."
        );
        if (!zDKUICurrentBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats)
        {
            zDKUIDrawNumberInput(
                "SA-effect ATK Down (%)",
                zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttackDownPercent,
                "%.1f",
                "Subtracts percentage points from the enemy's normal/SA multiplier. A 300% SA with ATK Down "
                "20% becomes 280%, not 240%. Enter 0 if immune."
            );
            zDKUIDrawNumberInput(
                "Enemy SA ATK raise (%)",
                zDKUICurrentBoss.zDKEffectStats.zDKCalcEnemySuperRaisePercent,
                "%.1f",
                "Raises from the enemy's own Super Attack. Included in that SA; included in normals only "
                "after the checkbox below is enabled. Enter the active total; stacking is not simulated."
            );
            zDKUICheckbox(
                "Enemy has already used a Super Attack",
                &zDKUICurrentBoss.zDKEffectStats.zDKCalcEnemyAlreadySupered
            );
        }
        else
            zDKUIDrawWrappedText(
                "SA raises and SA-effect ATK Down: use Base ATK + SA multiplier in Boss Stats, or include "
                "them in your direct ATK values."
            );
        zDKUIDrawNumberInput(
            "DEF ignored (%)", zDKUICurrentBoss.zDKEffectStats.zDKCalcDefenseIgnoredPercent, "%.1f"
        );
        zDKUICheckbox("Critical hit", &zDKUICurrentBoss.zDKEffectStats.zDKCalcIsCritical);
        if (zDKUICurrentBoss.zDKEffectStats.zDKCalcIsCritical)
        {
            zDKUIDrawNumberPair(
                "Critical multiplier",
                zDKUICurrentBoss.zDKEffectStats.zDKCalcCriticalMultiplier,
                "Critical DEF ignored (%)",
                zDKUICurrentBoss.zDKEffectStats.zDKCalcCriticalDefenseIgnoredPercent,
                "%.2f"
            );
            zDKUICheckbox(
                "Override type and guard", &zDKUICurrentBoss.zDKEffectStats.zDKCalcUseCustomCriticalType
            );
            if (zDKUICurrentBoss.zDKEffectStats.zDKCalcUseCustomCriticalType)
                zDKUIDrawNumberPair(
                    "Type multiplier",
                    zDKUICurrentBoss.zDKEffectStats.zDKCalcCriticalTypeMultiplier,
                    "Guard multiplier",
                    zDKUICurrentBoss.zDKEffectStats.zDKCalcCriticalGuardMultiplier,
                    "%.3f"
                );
            zDKUIShowTooltip(
                "Manual critical scenario: community sources disagree on type/guard behavior. Verify this "
                "boss's DEF ignore and use overrides for a tested matchup. Overrides do not add Type DEF "
                "Boost."
            );
            zDKUIDrawWrappedText("Critical type / guard rules need boss-specific verification.", zDKUIPink);
        }
    }
    else
    {
        zDKUIBeginPanel("boss-stats-panel", zDKUISize);
        zDKUIDrawNameInput("Enemy name", zDKUICurrentBoss.zDKEffectName);
        static const char* zDKUIAttackModes[] = {"Direct normal / SA ATK", "Base ATK + SA multiplier"};
        int zDKUIAttackMode = zDKUICurrentBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats ? 0 : 1;
        if (zDKUIDrawComboInput("Enemy ATK input", zDKUIAttackMode, zDKUIAttackModes, 2))
        {
            zDKUICurrentBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats = zDKUIAttackMode == 0;
            if (zDKUIAttackMode == 0)
            {
                zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttackDownPercent = 0;
                zDKUICurrentBoss.zDKEffectStats.zDKCalcEnemySuperRaisePercent = 0;
                zDKUICurrentBoss.zDKEffectStats.zDKCalcEnemyAlreadySupered = false;
            }
        }
        zDKUIDrawNumberInput(
            "Normal ATK",
            zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack,
            "%.0f",
            "Base enemy normal ATK before added skill effects and ATK Down. In multiplier mode this is also "
            "the base for its Super Attack."
        );
        if (zDKUICurrentBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats)
            zDKUIDrawNumberInput(
                "Super Attack ATK",
                zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttack,
                "%.0f",
                "Enter the full enemy Super Attack ATK. No SA multiplier is needed. Skill effects still "
                "apply."
            );
        else
            zDKUIDrawNumberInput(
                "SA multiplier (x)",
                zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttackMultiplier,
                "%.2f",
                "Use 3.0 for 300%. Enemy SA raises are configured separately in Boss Skills; do not count "
                "them twice."
            );
        zDKUIDrawTypeAndClass(
            zDKUICurrentBoss.zDKEffectStats.zDKCalcElementType,
            zDKUICurrentBoss.zDKEffectStats.zDKCalcCombatClass
        );
        zDKUIDrawNumberInput("Enemy HP (%)", zDKUICurrentBoss.zDKEffectHealthPercent);
    }
    zDKUIEndPanel();
}

std::string zDKUIBuildCalculationReport(const zDKEffectEvaluation& zDKUIEvaluationData)
{
    std::ostringstream zDKUIOutputStream;
    const auto& zDKUIBattleResultData = zDKUIEvaluationData.zDKEffectResult;
    zDKUIOutputStream << "DOKKAN DEFENSE CALCULATOR\nUnit: " << zDKUICurrentUnit.zDKEffectName
                      << "\nEnemy: " << zDKUICurrentBoss.zDKEffectName
                      << "\nTurn: " << zDKUICurrentBattle.zDKEffectTurn << "\nAttack timing: "
                      << (zDKUICurrentBattle.zDKEffectAfterUnitAttack ? "After attacking"
                                                                      : "Before attacking")
                      << "\n";
    zDKUIOutputStream
        << "DEF stat: " << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcDefense.zDKCalcSelected)
        << "\nDEF used vs this attack: "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcDefense.zDKCalcEffective)
        << "\nNormals [min / mid / max roll]: "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage) << " / "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcNormalAttackDamage.zDKCalcAverageDamage) << " / "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcNormalAttackDamage.zDKCalcMaximumDamage)
        << "\nSuper Attack [min / mid / max roll]: "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage) << " / "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcSuperAttackDamage.zDKCalcAverageDamage) << " / "
        << zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcSuperAttackDamage.zDKCalcMaximumDamage) << "\n";
    zDKUIOutputStream << "HP: " << zDKUIFormatInteger(zDKUICurrentBattle.zDKEffectCurrentHP) << " / "
                      << zDKUIFormatInteger(zDKUICurrentBattle.zDKEffectMaxHP) << "\nDEF vs normals: "
                      << zDKUIFormatInteger(zDKUIEvaluationData.zDKEffectNormalDefense.zDKCalcSelected)
                      << "\nDEF vs Super Attacks: "
                      << zDKUIFormatInteger(zDKUIEvaluationData.zDKEffectSuperDefense.zDKCalcSelected)
                      << "\nDomain damage received increase: "
                      << zDKUIFormatDecimal(
                             zDKUIEvaluationData.zDKEffectResolvedUnit.zDKCalcDamageReceivedPercent
                         )
                      << "%"
                      << "\nType: " << zDKUIFormatMultiplier(zDKUIBattleResultData.zDKCalcTypeMultiplier)
                      << "\nGuard: " << zDKUIFormatMultiplier(zDKUIBattleResultData.zDKCalcGuardMultiplier)
                      << "\nRounding: down after every stat and damage step"
                      << "\n\nMathematical zero may display as minimum damage in game.\n\n"
                      << zDKStoreSaveSession(
                             zDKUICurrentUnit,
                             zDKUICurrentBoss,
                             zDKUICurrentBattle,
                             zDKUIAnimationsEnabled,
                             zDKUISelectedAttackKind
                         );
    return zDKUIOutputStream.str();
}

void zDKUIDrawMetricCard(
    const char* zDKUITitle, const std::string& zDKUIValue, ImVec4 zDKUIColor, float zDKUILogicalHeight = 100
)
{
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    float zDKUIWidth = ImGui::GetContentRegionAvail().x, zDKUIHeight = zDKUIScalePixels(zDKUILogicalHeight);
    zDKUIDrawSkin(
        ImGui::GetWindowDrawList(),
        zDKUISkinKind::zDKUICard,
        zDKUITopLeft,
        {zDKUITopLeft.x + zDKUIWidth, zDKUITopLeft.y + zDKUIHeight},
        2,
        .92f
    );
    ImVec2 zDKUILabelSize = zDKUILabelFont->CalcTextSizeA(
        (zDKUILabelFont->FontSize * ImGui::GetIO().FontGlobalScale), FLT_MAX, 0, zDKUITitle
    );
    zDKUIDrawOutlinedText(
        ImGui::GetWindowDrawList(),
        zDKUILabelFont,
        (zDKUILabelFont->FontSize * ImGui::GetIO().FontGlobalScale),
        {zDKUITopLeft.x + (zDKUIWidth - zDKUILabelSize.x) * .5f, zDKUITopLeft.y + zDKUIScalePixels(17)},
        zDKUIWhite,
        zDKUITitle
    );
    float zDKUIFontSize = (zDKUIMetricFont->FontSize * ImGui::GetIO().FontGlobalScale);
    ImVec2 zDKUIValueSize = zDKUIMetricFont->CalcTextSizeA(zDKUIFontSize, FLT_MAX, 0, zDKUIValue.c_str());
    if (zDKUIValueSize.x > zDKUIWidth - zDKUIScalePixels(52))
    {
        zDKUIFontSize *= (zDKUIWidth - zDKUIScalePixels(52)) / zDKUIValueSize.x;
        zDKUIValueSize = zDKUIMetricFont->CalcTextSizeA(zDKUIFontSize, FLT_MAX, 0, zDKUIValue.c_str());
    }
    zDKUIDrawOutlinedText(
        ImGui::GetWindowDrawList(),
        zDKUIMetricFont,
        zDKUIFontSize,
        {zDKUITopLeft.x + (zDKUIWidth - zDKUIValueSize.x) * .5f, zDKUITopLeft.y + zDKUIScalePixels(43)},
        zDKUIColor,
        zDKUIValue.c_str()
    );
    ImGui::Dummy({zDKUIWidth, zDKUIHeight});
}

void zDKUIDrawHealthTrack(float zDKUIFraction)
{
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    float zDKUIWidth = ImGui::GetContentRegionAvail().x, zDKUIHeight = zDKUIScalePixels(23);
    zDKUIDrawSkin(
        ImGui::GetWindowDrawList(),
        zDKUISkinKind::zDKUIProgress,
        zDKUITopLeft,
        {zDKUITopLeft.x + zDKUIWidth, zDKUITopLeft.y + zDKUIHeight}
    );
    float zDKUIFill = std::clamp(zDKUIFraction, 0.f, 1.f);
    if (zDKUIFill > 0)
        ImGui::GetWindowDrawList()->AddRectFilledMultiColor(
            {zDKUITopLeft.x + zDKUIScalePixels(9), zDKUITopLeft.y + zDKUIScalePixels(8)},
            {zDKUITopLeft.x + zDKUIScalePixels(9) + (zDKUIWidth - zDKUIScalePixels(35)) * zDKUIFill,
             zDKUITopLeft.y + zDKUIHeight - zDKUIScalePixels(6)},
            zDKUIPackColor({.12f, .43f, .32f, 1}),
            zDKUIPackColor(zDKUIGreen),
            zDKUIPackColor(zDKUIGreen),
            zDKUIPackColor({.12f, .43f, .32f, 1})
        );
    ImGui::Dummy({zDKUIWidth, zDKUIHeight});
}

bool zDKUIRequestTurnPlan = false;
zDKTurnAttackPlan zDKUITurnPlanDraft;
std::string zDKUITurnPlanError;

void zDKUIDrawTurnPlanModal()
{
    if (zDKUIRequestTurnPlan)
    {
        zDKUITurnPlanDraft = zDKUICurrentUnit.zDKEffectAttackPlan;
        zDKUITurnPlanError.clear();
        ImGui::OpenPopup("Attack plan");
        zDKUIRequestTurnPlan = false;
    }
    zDKUIConfigureFullScreenModal();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(28, 28));
    if (ImGui::BeginPopupModal(
            "Attack plan",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse
        ))
    {
        zDKUIDrawModalHeader("Attack Plan");
        zDKUISmoothChild(
            "turn-plan-body",
            {0, std::max(zDKUIScalePixels(100), ImGui::GetContentRegionAvail().y - zDKUIScalePixels(142))},
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoBackground
        );
        zDKUIDrawWrappedText(
            "Set the opening attack, then each additional in the order performed.", zDKUIWhite
        );
        zDKUIAddSpacing();
        int zDKUIRemove = -1, zDKUIMove = -1, zDKUIDirection = 0;
        auto& zDKUIAttacks = zDKUITurnPlanDraft.zDKTurnAttacks;
        for (int zDKUIIndex = 0; zDKUIIndex < int(zDKUIAttacks.size()); ++zDKUIIndex)
        {
            ImGui::PushID(zDKUIIndex);
            zDKUIDrawLabel(
                zDKUIIndex == 0 ? "Opening attack" : ("Additional " + std::to_string(zDKUIIndex)).c_str()
            );
            if (ImGui::BeginTable("attack-row", 4, ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("kind", ImGuiTableColumnFlags_WidthStretch);
                for (const char* zDKUIColumn : {"up", "down", "remove"})
                    ImGui::TableSetupColumn(
                        zDKUIColumn, ImGuiTableColumnFlags_WidthFixed, zDKUIScalePixels(38)
                    );
                ImGui::TableNextColumn();
                if (zDKUIBeginCombo("##kind", zDKTurnAttackNames[int(zDKUIAttacks[zDKUIIndex])]))
                {
                    for (int zDKUIKind = 0; zDKUIKind < 3; ++zDKUIKind)
                    {
                        if (ImGui::Selectable(
                                zDKTurnAttackNames[zDKUIKind], int(zDKUIAttacks[zDKUIIndex]) == zDKUIKind
                            ))
                            zDKUIAttacks[zDKUIIndex] = zDKTurnAttackKind(zDKUIKind);
                        zDKUIRecordControlProbe(
                            "turn-option-" + std::to_string(zDKUIIndex) + "-" + std::to_string(zDKUIKind)
                        );
                    }
                    ImGui::EndCombo();
                }
                zDKUIRecordControlProbe("turn-kind-" + std::to_string(zDKUIIndex));
                ImGui::TableNextColumn();
                ImGui::BeginDisabled(zDKUIIndex == 0);
                if (zDKUIDrawButton("Up", zDKUIScaleSize(38, 44)))
                {
                    zDKUIMove = zDKUIIndex;
                    zDKUIDirection = -1;
                }
                zDKUIRecordControlProbe("turn-up-" + std::to_string(zDKUIIndex));
                ImGui::EndDisabled();
                ImGui::TableNextColumn();
                ImGui::BeginDisabled(zDKUIIndex + 1 == int(zDKUIAttacks.size()));
                if (zDKUIDrawButton("Dn", zDKUIScaleSize(38, 44)))
                {
                    zDKUIMove = zDKUIIndex;
                    zDKUIDirection = 1;
                }
                zDKUIRecordControlProbe("turn-down-" + std::to_string(zDKUIIndex));
                ImGui::EndDisabled();
                ImGui::TableNextColumn();
                if (zDKUIDrawButton("×", zDKUIScaleSize(38, 44)))
                    zDKUIRemove = zDKUIIndex;
                zDKUIRecordControlProbe("turn-remove-" + std::to_string(zDKUIIndex));
                ImGui::EndTable();
            }
            zDKUIAddSpacing(6);
            ImGui::PopID();
        }
        if (zDKUIRemove >= 0)
            zDKUIAttacks.erase(zDKUIAttacks.begin() + zDKUIRemove);
        else if (zDKUIMove >= 0)
            std::swap(zDKUIAttacks[zDKUIMove], zDKUIAttacks[zDKUIMove + zDKUIDirection]);
        ImGui::BeginDisabled(zDKUIAttacks.size() >= zDKTurnMaximumAttacks);
        if (zDKUIDrawButton("+ Add attack", {-1, 0}))
            zDKUIAttacks.push_back(zDKTurnAttackKind::zDKTurnSuper);
        ImGui::EndDisabled();
        zDKUIAddSpacing();
        zDKUIDrawWrappedText(
            "Choose the actual SA or Ultra SA performed. Additional-attack chances are not rolled."
        );
        if (!zDKUITurnPlanError.empty())
            zDKUIDrawWrappedText(zDKUITurnPlanError, zDKUIPink);
        zDKUIEndSmoothChild();
        zDKUIAddSpacing(2);
        if (zDKUIDrawButton("Apply attack plan", {-1, zDKUIScalePixels(54)}))
        {
            try
            {
                zDKTurnValidate(zDKUITurnPlanDraft);
                auto zDKUITestUnit = zDKUICurrentUnit;
                zDKUITestUnit.zDKEffectAttackPlan = zDKUITurnPlanDraft;
                zDKTurnCalculateTimeline(zDKUITestUnit, zDKUICurrentBoss, zDKUICurrentBattle);
                zDKUICurrentUnit.zDKEffectAttackPlan = zDKUITurnPlanDraft;
                ImGui::CloseCurrentPopup();
            }
            catch (const std::exception& zDKUIError)
            {
                zDKUITurnPlanError = zDKUIError.what();
            }
        }
        zDKUIAddSpacing(4);
        if (zDKUIDrawButton("Cancel attack plan", {-1, zDKUIScalePixels(54)}) ||
            ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

void zDKUIDrawTurnDefense()
{
    if (zDKUIDrawButton("Edit attack plan", {-1, 0}))
        zDKUIRequestTurnPlan = true;
    zDKUIAddSpacing();
    const auto zDKUIPoints = zDKTurnCalculateTimeline(zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle);
    const auto& zDKUIFinal = zDKUIPoints.back();
    zDKUIDrawMetricCard(
        zDKUIPoints.size() > 1 ? "END-OF-TURN DEF" : "STARTING DEF",
        zDKUIFormatInteger(
            zDKUISelectedAttackKind ? zDKUIFinal.zDKTurnSuperDefense : zDKUIFinal.zDKTurnNormalDefense
        ),
        zDKUIWhite,
        96
    );
    zDKUIDrawWrappedText(
        "SA and Ultra SA counts in Unit Effects are your starting raises. The preview adds each planned "
        "attack."
    );
    if (zDKUICurrentUnit.zDKEffectUseManualDefense)
        zDKUIDrawWrappedText("Manual DEF stays fixed. Enter base DEF to calculate growth.", zDKUIPink);
    std::ostringstream zDKUIReport;
    zDKUIReport << "Dokkan Defense Calculator\nTurn " << zDKUICurrentBattle.zDKEffectTurn << " attack plan\n";
    for (const auto& zDKUIPoint : zDKUIPoints)
    {
        ImGui::PushID(zDKUIPoint.zDKTurnAttackNumber);
        zDKUIAddSpacing(10);
        zDKUIDrawDivider();
        const std::string zDKUITitle = zDKUIPoint.zDKTurnAttackNumber == 0
            ? "Before attacking"
            : "After attack " + std::to_string(zDKUIPoint.zDKTurnAttackNumber) + ": " +
                zDKTurnAttackNames[int(zDKUIPoint.zDKTurnKind)];
        zDKUIDrawWrappedText(zDKUITitle, zDKUIWhite);
        zDKUIDrawStatisticRow("DEF vs normals", zDKUIFormatInteger(zDKUIPoint.zDKTurnNormalDefense));
        zDKUIDrawStatisticRow("DEF vs Super Attacks", zDKUIFormatInteger(zDKUIPoint.zDKTurnSuperDefense));
        if (zDKUIPoint.zDKTurnAttackNumber > 0)
        {
            const auto& zDKUIPrevious = zDKUIPoints[zDKUIPoint.zDKTurnAttackNumber - 1];
            const double zDKUIGain = zDKUISelectedAttackKind
                ? zDKUIPoint.zDKTurnSuperDefense - zDKUIPrevious.zDKTurnSuperDefense
                : zDKUIPoint.zDKTurnNormalDefense - zDKUIPrevious.zDKTurnNormalDefense;
            zDKUIDrawStatisticRow(
                "DEF change",
                (zDKUIGain >= 0 ? "+" : "") + zDKUIFormatInteger(zDKUIGain),
                zDKUIGain >= 0 ? zDKUIGreen : zDKUIPink
            );
        }
        if (zDKUIBeginReveal("Incoming damage"))
        {
            zDKUIDrawStatisticRow(
                "Effective DEF vs normals", zDKUIFormatInteger(zDKUIPoint.zDKTurnNormalEffectiveDefense)
            );
            zDKUIDrawStatisticRow(
                "Effective DEF vs SA", zDKUIFormatInteger(zDKUIPoint.zDKTurnSuperEffectiveDefense)
            );
            zDKUIDrawStatisticRow("Max normal damage", zDKUIFormatInteger(zDKUIPoint.zDKTurnNormalMaxDamage));
            zDKUIDrawStatisticRow("Max SA damage", zDKUIFormatInteger(zDKUIPoint.zDKTurnSuperMaxDamage));
            zDKUIEndReveal();
        }
        zDKUIReport << zDKUITitle
                    << "\nDEF vs normals: " << zDKUIFormatInteger(zDKUIPoint.zDKTurnNormalDefense)
                    << "\nDEF vs Super Attacks: " << zDKUIFormatInteger(zDKUIPoint.zDKTurnSuperDefense)
                    << "\nMax normal damage: " << zDKUIFormatInteger(zDKUIPoint.zDKTurnNormalMaxDamage)
                    << "\nMax SA damage: " << zDKUIFormatInteger(zDKUIPoint.zDKTurnSuperMaxDamage) << "\n\n";
        ImGui::PopID();
    }
    zDKUIAddSpacing();
    if (zDKUIDrawButton("Copy turn DEF", {-1, 0}))
    {
        ImGui::SetClipboardText(zDKUIReport.str().c_str());
        zDKUIShowNotice("Turn DEF copied.");
    }
}

void zDKUIDrawAnalysisPanel(ImVec2 zDKUISize)
{
    static const char* zDKUIIds[] = {"damage-panel", "tanking-panel", "turn-panel", "calcs-panel"};
    zDKUIBeginPanel(zDKUIIds[zDKUIDisplayedSection], zDKUISize);
    zDKEffectEvaluation zDKUIEvaluation;
    try
    {
        zDKUICurrentBattle.zDKEffectMaxHP =
            static_cast<double>(zDKCalcFloorStatistic(zDKUICurrentBattle.zDKEffectMaxHP));
        zDKUICurrentBattle.zDKEffectCurrentHP =
            static_cast<double>(zDKCalcFloorStatistic(zDKUICurrentBattle.zDKEffectCurrentHP));
        zDKUIEvaluation = zDKTurnEvaluate(
            zDKUICurrentUnit, zDKUICurrentBoss, zDKUICurrentBattle, zDKUISelectedAttackKind == 1
        );
    }
    catch (const std::exception& zDKUIException)
    {
        zDKUIDrawWrappedText(zDKUIException.what(), zDKUIPink);
        zDKUIEndPanel();
        return;
    }
    auto& zDKUIBattleResultData = zDKUIEvaluation.zDKEffectResult;
    auto& zDKUIUnitStatsData = zDKUIEvaluation.zDKEffectResolvedUnit;
    if (zDKUIDisplayedSection == 0)
    {
        zDKUIDrawMetricCard(
            "DEF STAT",
            zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcDefense.zDKCalcSelected),
            zDKUIWhite,
            96
        );
        zDKUIDrawStatisticRow(
            "DEF used vs this attack",
            zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcDefense.zDKCalcEffective)
        );
        zDKUIShowTooltip("DEF after DEF Down and any DEF ignored by this enemy.");
        zDKUIDrawStatisticRow(
            "DEF vs normals", zDKUIFormatInteger(zDKUIEvaluation.zDKEffectNormalDefense.zDKCalcSelected)
        );
        zDKUIDrawStatisticRow(
            "DEF vs Super Attacks", zDKUIFormatInteger(zDKUIEvaluation.zDKEffectSuperDefense.zDKCalcSelected)
        );
        zDKUIDrawDivider();
    }
    float zDKUIWidth = (ImGui::GetContentRegionAvail().x - zDKUIScalePixels(8)) / 2;
    if (zDKUIDrawButton("Normals", {zDKUIWidth, 0}, zDKUISelectedAttackKind == 0))
        zDKUISelectedAttackKind = 0;
    ImGui::SameLine(0, zDKUIScalePixels(8));
    if (zDKUIDrawButton(
            zDKUIWidth < zDKUIScalePixels(170) ? "SA" : "Super Attack",
            {zDKUIWidth, 0},
            zDKUISelectedAttackKind == 1
        ))
        zDKUISelectedAttackKind = 1;
    zDKUIAddSpacing();
    const zDKCalcDamageRange& zDKUIDamageRangeData = zDKUISelectedAttackKind
        ? zDKUIBattleResultData.zDKCalcSuperAttackDamage
        : zDKUIBattleResultData.zDKCalcNormalAttackDamage;
    if (zDKUIDisplayedSection == 0)
    {
        zDKUIDrawMetricCard(
            "DAMAGE RECEIVED",
            zDKUIFormatInteger(zDKUIDamageRangeData.zDKCalcAverageDamage),
            zDKUICurrentBattle.zDKEffectMaxHP > 0 &&
                    zDKUIDamageRangeData.zDKCalcAverageDamage >= zDKUICurrentBattle.zDKEffectCurrentHP
                ? zDKUIPink
                : zDKUIWhite,
            100
        );
        zDKUIDrawStatisticRow("Min roll", zDKUIFormatInteger(zDKUIDamageRangeData.zDKCalcMinimumDamage));
        zDKUIDrawStatisticRow("Max roll", zDKUIFormatInteger(zDKUIDamageRangeData.zDKCalcMaximumDamage));
        zDKUIShowTooltip("Damage rolls use variance 1.00–1.03. The main value is the mid roll at 1.015.");
        if (zDKUIDamageRangeData.zDKCalcMaximumDamage == 0)
            zDKUIDrawWrappedText("At the damage floor; in-game minimum damage is not modeled.");
    }
    else if (zDKUIDisplayedSection == 1)
    {
        zDKUIAddSpacing();
        std::string zDKUISurvival = zDKUICurrentBattle.zDKEffectMaxHP == 0 ? "Enter team HP to check survival"
            : zDKUIDamageRangeData.zDKCalcMaximumDamage == 0
            ? "Damage floor: small in-game damage is not modeled"
            : zDKUIDamageRangeData.zDKCalcMinimumDamage >= zDKUICurrentBattle.zDKEffectCurrentHP
            ? "K.O. even on min roll"
            : zDKUIDamageRangeData.zDKCalcMaximumDamage >= zDKUICurrentBattle.zDKEffectCurrentHP
            ? "K.O. depends on the roll"
            : "Survives max roll";
        zDKUIDrawWrappedText(
            zDKUISurvival,
            zDKUICurrentBattle.zDKEffectMaxHP == 0 ? zDKUIMuted
                : zDKUIDamageRangeData.zDKCalcMaximumDamage < zDKUICurrentBattle.zDKEffectCurrentHP
                ? zDKUIGreen
                : zDKUIPink
        );
        double zDKUIHpLeft =
            std::max(0.0, zDKUICurrentBattle.zDKEffectCurrentHP - zDKUIDamageRangeData.zDKCalcMaximumDamage);
        float zDKUIHpFill = zDKUICurrentBattle.zDKEffectMaxHP > 0
            ? float(zDKUIHpLeft / zDKUICurrentBattle.zDKEffectMaxHP)
            : 0;
        zDKUIDrawHealthTrack(zDKUIHpFill);
        zDKUIDrawStatisticRow(
            "HP after max damage", zDKUIFormatInteger(zDKUIHpLeft), zDKUIHpLeft > 0 ? zDKUIGreen : zDKUIPink
        );
        zDKUIDrawStatisticRow(
            "Remaining HP",
            zDKUIFormatInteger(zDKUICurrentBattle.zDKEffectCurrentHP) + " / " +
                zDKUIFormatInteger(zDKUICurrentBattle.zDKEffectMaxHP)
        );
        zDKUIDrawStatisticRow(
            "HP lost on max damage",
            zDKUIFormatDecimal(
                std::min(
                    100.0,
                    100 * zDKUIDamageRangeData.zDKCalcMaximumDamage /
                        std::max(1.0, zDKUICurrentBattle.zDKEffectCurrentHP)
                )
            ) + "%"
        );
        zDKUIDrawStatisticRow(
            "Hits until K.O.",
            zDKUICurrentBattle.zDKEffectMaxHP == 0 ? "—"
                : zDKUIDamageRangeData.zDKCalcMaximumDamage > 0
                ? zDKUIFormatInteger(
                      std::ceil(
                          zDKUICurrentBattle.zDKEffectCurrentHP / zDKUIDamageRangeData.zDKCalcMaximumDamage
                      )
                  )
                : "Minimum damage"
        );
        zDKUIShowTooltip(
            "Uses the max roll and the same DEF, guard and DR on every hit. HP conditions may "
            "change after taking damage."
        );
    }
    else if (zDKUIDisplayedSection == 2)
    {
        try
        {
            zDKUIDrawTurnDefense();
        }
        catch (const std::exception& zDKUIError)
        {
            zDKUIDrawWrappedText(zDKUIError.what(), zDKUIPink);
        }
    }
    else
    {
        if (zDKUIBeginReveal("Guard & DR"))
        {
            zDKUIDrawStatisticRow(
                "Type matchup",
                zDKUIBattleResultData.zDKCalcMatchupRelation == zDKCalcTypeRelation::zDKCalcAdvantage
                    ? "Type advantage"
                    : zDKUIBattleResultData.zDKCalcMatchupRelation == zDKCalcTypeRelation::zDKCalcDisadvantage
                    ? "Type disadvantage"
                    : "Type neutral"
            );
            zDKUIDrawStatisticRow(
                "Type / guard",
                zDKUIFormatMultiplier(zDKUIBattleResultData.zDKCalcTypeMultiplier) + " / " +
                    zDKUIFormatMultiplier(zDKUIBattleResultData.zDKCalcGuardMultiplier)
            );
            zDKUIDrawStatisticRow(
                "Damage Reduction (DR)",
                zDKUIFormatDecimal(zDKUIUnitStatsData.zDKCalcDamageReductionPercent) + "%"
            );
            zDKUIDrawStatisticRow(
                "Normal / SA DR",
                zDKUIFormatDecimal(zDKUIUnitStatsData.zDKCalcNormalDamageReductionPercent) + "% / " +
                    zDKUIFormatDecimal(zDKUIUnitStatsData.zDKCalcSuperDamageReductionPercent) + "%"
            );
            zDKUIDrawStatisticRow(
                "DEF Down", zDKUIFormatDecimal(zDKUIUnitStatsData.zDKCalcDefenseDebuffPercent) + "%"
            );
            zDKUIDrawStatisticRow(
                "Domain damage received",
                "+" + zDKUIFormatDecimal(zDKUIUnitStatsData.zDKCalcDamageReceivedPercent) + "% after DEF"
            );
            zDKUIDrawStatisticRow(
                "Type DEF Boost", zDKUIBattleResultData.zDKCalcTypeDefenseBoostActive ? "Applied" : "Inactive"
            );
            zDKUIDrawStatisticRow(
                "DEF to reach damage floor", zDKUIFormatInteger(zDKUIDamageRangeData.zDKCalcRequiredDefense)
            );
            zDKUIShowTooltip(
                "DEF needed after DEF Down and DEF ignored to reach mathematical zero on the max roll."
            );
            zDKUIEndReveal();
        }
        if (zDKUIBeginReveal("Enemy ATK calc"))
        {
            zDKUIDrawStatisticRow(
                "Base ATK after passive effects",
                zDKUIFormatInteger(zDKUIEvaluation.zDKEffectResolvedBoss.zDKCalcNormalAttack)
            );
            zDKUIDrawStatisticRow("Enemy DEF", zDKUIFormatInteger(zDKUIEvaluation.zDKEffectBossDefense));
            zDKUIShowTooltip("Enemy DEF is shown for reference and does not affect damage received.");
            const double zDKUICritical = zDKUIEvaluation.zDKEffectResolvedBoss.zDKCalcIsCritical
                ? zDKUIEvaluation.zDKEffectResolvedBoss.zDKCalcCriticalMultiplier
                : 1;
            zDKUIDrawStatisticRow(
                "Normal ATK",
                zDKUIFormatInteger(
                    static_cast<double>(zDKCalcApplyMultiplier(
                        zDKCalcFloorStatistic(zDKCalcNormalAttackStat(zDKUIEvaluation.zDKEffectResolvedBoss)),
                        zDKUICritical
                    ))
                )
            );
            zDKUIDrawStatisticRow(
                "SA ATK",
                zDKUIFormatInteger(
                    static_cast<double>(zDKCalcApplyMultiplier(
                        zDKCalcFloorStatistic(zDKCalcSuperAttackStat(zDKUIEvaluation.zDKEffectResolvedBoss)),
                        zDKUICritical
                    ))
                )
            );
            if (zDKUIEvaluation.zDKEffectResolvedBoss.zDKCalcIsCritical)
                zDKUIDrawStatisticRow("Critical multiplier", zDKUIFormatMultiplier(zDKUICritical));
            zDKUIDrawStatisticRow(
                "ATK after DR & type", zDKUIFormatInteger(zDKUIDamageRangeData.zDKCalcAdjustedAttack)
            );
            zDKUIDrawStatisticRow(
                "DEF ignored",
                zDKUIFormatDecimal(zDKUIEvaluation.zDKEffectResolvedBoss.zDKCalcDefenseIgnoredPercent) + "%"
            );
            zDKUIEndReveal();
        }
        if (zDKUIBeginReveal("DEF calc"))
        {
            zDKUIDrawStatisticRow(
                "Pre-attack DEF", zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcDefense.zDKCalcBeforeAttack)
            );
            zDKUIDrawStatisticRow(
                "Post-attack DEF", zDKUIFormatInteger(zDKUIBattleResultData.zDKCalcDefense.zDKCalcAfterAttack)
            );
            if (!zDKUICurrentUnit.zDKEffectUseManualDefense)
                for (const auto& zDKUIStep : zDKUIBattleResultData.zDKCalcDefense.zDKCalcSteps)
                    zDKUIDrawStatisticRow(zDKUIStep.zDKCalcName, zDKUIFormatInteger(zDKUIStep.zDKCalcValue));
            zDKUIEndReveal();
        }
        zDKUIAddSpacing();
        if (zDKUIDrawButton("Copy calc", {-1, 0}))
        {
            ImGui::SetClipboardText(zDKUIBuildCalculationReport(zDKUIEvaluation).c_str());
            zDKUIShowNotice("Calc copied.");
        }
    }
    zDKUIEndPanel();
}

void zDKUIDrawReferenceModal()
{
    zDKUIConfigureFullScreenModal();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(28, 28));
    if (ImGui::BeginPopupModal(
            "Calculation reference",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse
        ))
    {
        zDKUIDrawModalHeader("Calculation Reference");
        zDKUISmoothChild(
            "reference-body",
            {0, std::max(zDKUIScalePixels(100), ImGui::GetContentRegionAvail().y - zDKUIScalePixels(72))},
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoBackground
        );
        zDKUIDrawWrappedText(
            "DEF calc: Base DEF × Leader Skill × start-of-turn passive (SoT) × Domain × Support Items × Link "
            "Skills × Active Skill × when-attacking passive × Super Attack effects.",
            zDKUIWhite
        );
        zDKUIAddSpacing();
        zDKUIDrawWrappedText(
            "Boosts in the same phase add together. Multiplicative applies a separate multiplier. SA stacks "
            "use the boost per stack × stacks applied, including within a separate multiplier."
        );
        zDKUIAddSpacing();
        zDKUIDrawWrappedText(
            "SA = Super Attack. SoT = start of turn. DR = damage reduction. Before/after "
            "attacking refers to the unit's attack, including normal attacks. Enter only "
            "the SA stacks that apply at that point."
        );
        zDKUIAddSpacing();
        zDKUIDrawWrappedText(
            "Enter Normal ATK and full Super Attack ATK separately. Damage = max(0, (selected ATK × skill "
            "boosts × critical × general DR × specific DR × type × variance − effective DEF) × guard)",
            zDKUIWhite
        );
        zDKUIAddSpacing();
        zDKUIDrawWrappedText(
            "Guard applies after DEF. Type DEF Boost (Hidden Potential) requires natural "
            "type advantage. Critical hit rules must match the enemy. Damage rolls use "
            "variance 1.00–1.03; the mid roll uses 1.015."
        );
        zDKUIAddSpacing();
        zDKUIDrawWrappedText(
            "Every ATK, DEF and damage step rounds down before the next step. Boosts in the same "
            "phase add before applying the multiplier. Damage received uses this integer-step policy; "
            "not every internal rounding boundary has been verified in game. Minimum damage and "
            "event-specific rules can differ."
        );
        zDKUIAddSpacing();
        zDKUIEndSmoothChild();
        zDKUIAddSpacing(2);
        if (zDKUIDrawButton("Close", {-1, zDKUIScalePixels(54)}) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

enum class zDKUIUpdatePhase
{
    zDKUIIdle,
    zDKUIChecking,
    zDKUIAvailable,
    zDKUICurrent,
    zDKUIDownloading,
    zDKUIReady,
    zDKUIError
};
zDKUIUpdatePhase zDKUIUpdateState = zDKUIUpdatePhase::zDKUIIdle;
std::future<zDK::zDKUpdateRelease> zDKUIUpdateCheckTask;
std::future<std::filesystem::path> zDKUIUpdateDownloadTask;
zDK::zDKUpdateRelease zDKUIUpdateReleaseData;
std::filesystem::path zDKUIUpdateDownloaded;
std::string zDKUIUpdateErrorText;

void zDKUIPollUpdates()
{
    try
    {
        if (zDKUIUpdateState == zDKUIUpdatePhase::zDKUIChecking &&
            zDKUIUpdateCheckTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            zDKUIUpdateReleaseData = zDKUIUpdateCheckTask.get();
            zDKUIUpdateState = zDK::zDKUpdateParseVersion(zDKUIUpdateReleaseData.zDKUpdateVersion) >
                    zDK::zDKUpdateParseVersion(zDK::zDKAppVersion)
                ? zDKUIUpdatePhase::zDKUIAvailable
                : zDKUIUpdatePhase::zDKUICurrent;
        }
        if (zDKUIUpdateState == zDKUIUpdatePhase::zDKUIDownloading &&
            zDKUIUpdateDownloadTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            zDKUIUpdateDownloaded = zDKUIUpdateDownloadTask.get();
            zDKUIUpdateState = zDKUIUpdatePhase::zDKUIReady;
        }
    }
    catch (const std::exception& zDKUIError)
    {
        zDKUIUpdateErrorText = zDKUIError.what();
        zDKUIUpdateState = zDKUIUpdatePhase::zDKUIError;
    }
}

void zDKUIDrawUpdatesModal(HWND zDKUIWindowHandle)
{
    zDKUIPollUpdates();
    zDKUIConfigureFullScreenModal();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(28, 28));
    if (ImGui::BeginPopupModal(
            "App updates",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse
        ))
    {
        zDKUIDrawModalHeader("App Updates");
        zDKUISmoothChild(
            "updates-body",
            {0, std::max(zDKUIScalePixels(100), ImGui::GetContentRegionAvail().y - zDKUIScalePixels(144))},
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoBackground
        );
        zDKUIDrawFontText(zDKUIHeadingFont, zDKUIWhite, "Dokkan Defense Calculator");
        zDKUIDrawWrappedText(std::string("Installed version: ") + zDK::zDKAppVersion);
        zDKUIAddSpacing(14);
        switch (zDKUIUpdateState)
        {
            case zDKUIUpdatePhase::zDKUIIdle:
                zDKUIDrawWrappedText(
                    "Check for the latest Windows release on GitHub. No account is required."
                );
                break;
            case zDKUIUpdatePhase::zDKUIChecking:
                zDKUIDrawWrappedText("Checking GitHub for updates...", zDKUIWhite);
                break;
            case zDKUIUpdatePhase::zDKUIAvailable:
            case zDKUIUpdatePhase::zDKUIDownloading:
            case zDKUIUpdatePhase::zDKUIReady:
                zDKUIDrawWrappedText(
                    "Version " + zDKUIUpdateReleaseData.zDKUpdateVersion + " is available.", zDKUIGreen
                );
                zDKUIAddSpacing();
                zDKUIDrawWrappedText(
                    zDKUIUpdateState == zDKUIUpdatePhase::zDKUIDownloading
                        ? "Downloading and verifying the update..."
                        : zDKUIUpdateState == zDKUIUpdatePhase::zDKUIReady
                        ? "Download verified. Install the update to restart the calculator."
                        : "Download the update, then install it when you are ready."
                );
                zDKUIAddSpacing();
                zDKUIDrawWrappedText(zDKUIUpdateReleaseData.zDKUpdateNotes, zDKUIWhite);
                break;
            case zDKUIUpdatePhase::zDKUICurrent:
                zDKUIDrawWrappedText("You're up to date.", zDKUIGreen);
                break;
            case zDKUIUpdatePhase::zDKUIError:
                zDKUIDrawWrappedText(zDKUIUpdateErrorText, zDKUIPink);
                break;
        }
        zDKUIAddSpacing(14);
        zDKUIDrawWrappedText("Your presets and saved session are kept when updating.");
        zDKUIEndSmoothChild();
        zDKUIAddSpacing(2);
        const bool zDKUIBusy = zDKUIUpdateState == zDKUIUpdatePhase::zDKUIChecking ||
            zDKUIUpdateState == zDKUIUpdatePhase::zDKUIDownloading;
        ImGui::BeginDisabled(zDKUIBusy);
        const char* zDKUIAction = zDKUIUpdateState == zDKUIUpdatePhase::zDKUIAvailable ? "Download update"
            : zDKUIUpdateState == zDKUIUpdatePhase::zDKUIReady                         ? "Install & restart"
                                                                                       : "Check for updates";
        if (zDKUIDrawButton(zDKUIAction, {-1, zDKUIScalePixels(54)}))
        {
            try
            {
                if (zDKUIUpdateState == zDKUIUpdatePhase::zDKUIAvailable)
                {
                    const auto zDKUIRelease = zDKUIUpdateReleaseData;
                    zDKUIUpdateDownloadTask = std::async(
                        std::launch::async, [zDKUIRelease] { return zDK::zDKUpdateDownload(zDKUIRelease); }
                    );
                    zDKUIUpdateState = zDKUIUpdatePhase::zDKUIDownloading;
                }
                else if (zDKUIUpdateState == zDKUIUpdatePhase::zDKUIReady)
                {
                    zDK::zDKUpdateStartInstall(zDKUIUpdateDownloaded, zDKUIUpdateReleaseData);
                    PostMessageW(zDKUIWindowHandle, WM_CLOSE, 0, 0);
                }
                else
                {
                    zDKUIUpdateCheckTask = std::async(std::launch::async, zDK::zDKUpdateCheck);
                    zDKUIUpdateState = zDKUIUpdatePhase::zDKUIChecking;
                }
            }
            catch (const std::exception& zDKUIError)
            {
                zDKUIUpdateErrorText = zDKUIError.what();
                zDKUIUpdateState = zDKUIUpdatePhase::zDKUIError;
            }
        }
        ImGui::EndDisabled();
        zDKUIAddSpacing(4);
        if (zDKUIDrawButton("Back to calculator", {-1, zDKUIScalePixels(54)}) ||
            ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        zDKUIRecordControlProbe("Back to calculator");
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

void zDKUIDrawNavigationBar(
    const char* zDKUIId,
    const char* const* zDKUILabels,
    int& zDKUISelected,
    ImVec2 zDKUISize,
    bool zDKUIPrimary = false,
    int zDKUICount = 3
)
{
    ImGui::PushID(zDKUIId);
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos(),
           zDKUIBottomRight = {zDKUITopLeft.x + zDKUISize.x, zDKUITopLeft.y + zDKUISize.y};
    auto* zDKUIDrawList = ImGui::GetWindowDrawList();
    zDKUIDrawSkin(zDKUIDrawList, zDKUISkinKind::zDKUISection, zDKUITopLeft, zDKUIBottomRight, 0);
    float zDKUIInset = zDKUIScalePixels(10), zDKUISegment = (zDKUISize.x - zDKUIInset * 2) / zDKUICount;
    ImGui::PushFont(zDKUIPrimary ? zDKUIHeadingFont : zDKUIBodyFont);
    for (int zDKUIIndex = 0; zDKUIIndex < zDKUICount; ++zDKUIIndex)
    {
        ImVec2 zDKUIMinimum =
                   {zDKUITopLeft.x + zDKUIInset + zDKUIIndex * zDKUISegment,
                    zDKUITopLeft.y + zDKUIScalePixels(8)},
               zDKUIMaximum = {zDKUIMinimum.x + zDKUISegment, zDKUIBottomRight.y - zDKUIScalePixels(8)};
        ImGui::SetCursorScreenPos(zDKUIMinimum);
        ImGui::PushID(zDKUIIndex);
        bool zDKUIHit = ImGui::InvisibleButton(
            zDKUILabels[zDKUIIndex],
            {zDKUISegment, zDKUISize.y - zDKUIScalePixels(16)},
            ImGuiButtonFlags_EnableNav
        );
        zDKUIRecordControlProbe(zDKUILabels[zDKUIIndex]);
        if (zDKUIHit && !zDKUISuppressGestureClick())
            zDKUISelected = zDKUIIndex;
        float zDKUIAmount =
            zDKUIAnimateValue(ImGui::GetID("nav-selected"), zDKUISelected == zDKUIIndex ? 1.f : 0.f, .085f);
        float zDKUIHover =
            zDKUIAnimateValue(ImGui::GetID("nav-hover"), ImGui::IsItemHovered() ? 1.f : 0.f, .08f);
        zDKUIDrawList->AddRectFilled(
            {zDKUIMinimum.x + zDKUIScalePixels(3), zDKUIMinimum.y + zDKUIScalePixels(2)},
            {zDKUIMaximum.x - zDKUIScalePixels(3), zDKUIMaximum.y - zDKUIScalePixels(2)},
            zDKUIPackColor({.75f, .035f, .33f, .25f * zDKUIAmount + .10f * zDKUIHover}),
            zDKUIScalePixels(4)
        );
        if (zDKUIIndex)
            zDKUIDrawList->AddLine(
                {zDKUIMinimum.x, zDKUIMinimum.y + zDKUIScalePixels(6)},
                {zDKUIMinimum.x, zDKUIMaximum.y - zDKUIScalePixels(6)},
                zDKUIPackColor({.65f, .14f, .38f, .65f}),
                zDKUIScalePixels(1)
            );
        ImGui::PushStyleColor(ImGuiCol_Text, zDKUISelected == zDKUIIndex ? zDKUIWhite : zDKUIMuted);
        zDKUIDrawControlText(zDKUILabels[zDKUIIndex], zDKUIMinimum, zDKUIMaximum, zDKUIScalePixels(8));
        ImGui::PopStyleColor();
        zDKUIDrawList->AddLine(
            {zDKUIMinimum.x + zDKUIScalePixels(16), zDKUIMaximum.y - zDKUIScalePixels(2)},
            {zDKUIMaximum.x - zDKUIScalePixels(16), zDKUIMaximum.y - zDKUIScalePixels(2)},
            zDKUIPackColor({1, .24f, .61f, zDKUIAmount}),
            zDKUIScalePixels(2)
        );
        if (ImGui::IsItemFocused())
            zDKUIDrawList->AddRect(
                {zDKUIMinimum.x + zDKUIScalePixels(4), zDKUIMinimum.y + zDKUIScalePixels(3)},
                {zDKUIMaximum.x - zDKUIScalePixels(4), zDKUIMaximum.y - zDKUIScalePixels(3)},
                zDKUIPackColor(zDKUIPink),
                zDKUIScalePixels(4)
            );
        ImGui::PopID();
    }
    ImGui::PopFont();
    ImGui::PopID();
    ImGui::SetCursorScreenPos({zDKUITopLeft.x, zDKUIBottomRight.y});
    ImGui::Dummy({0, 0});
}

void zDKUIRender(HWND zDKUIWindowHandle)
{
    zDKUIGesturePressBlocked = false;
    if (ImGui::IsMouseClicked(0) || (!ImGui::GetIO().MouseDown[0] && !ImGui::IsMouseReleased(0)))
        zDKUIDragGestureState = {};
    auto* zDKUIViewport = ImGui::GetMainViewport();
    auto* zDKUIBackgroundDrawList = ImGui::GetBackgroundDrawList();
    zDKUIBackgroundDrawList->AddRectFilled({0, 0}, zDKUIViewport->Size, IM_COL32(8, 3, 10, 255));
    if (zDKUIBackgroundTexture)
    {
        const float zDKUIScale = std::max(zDKUIViewport->Size.x / 941.f, zDKUIViewport->Size.y / 1672.f);
        ImVec2 zDKUISize = {941 * zDKUIScale, 1672 * zDKUIScale},
               zDKUITopLeft = {
                   (zDKUIViewport->Size.x - zDKUISize.x) * .5f, (zDKUIViewport->Size.y - zDKUISize.y) * .5f
               };
        zDKUIBackgroundDrawList->AddImage(
            (ImTextureID)(intptr_t)zDKUIBackgroundTexture.Get(),
            zDKUITopLeft,
            {zDKUITopLeft.x + zDKUISize.x, zDKUITopLeft.y + zDKUISize.y}
        );
    }
    zDKUIBackgroundDrawList->AddRectFilledMultiColor(
        {0, 0},
        zDKUIViewport->Size,
        IM_COL32(4, 2, 7, 165),
        IM_COL32(4, 2, 7, 165),
        IM_COL32(4, 2, 7, 195),
        IM_COL32(4, 2, 7, 195)
    );
    ImGui::SetNextWindowPos(zDKUIViewport->Pos);
    ImGui::SetNextWindowSize(zDKUIViewport->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, zDKUIScaleSize(12, 10));
    ImGui::Begin(
        "Dokkan Defense Calculator",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
    );
    float zDKUIAvailable = ImGui::GetContentRegionAvail().x,
          zDKUIWidth = std::min(zDKUIAvailable, zDKUIScalePixels(590));
    float zDKUILeft = ImGui::GetCursorPosX() + (zDKUIAvailable - zDKUIWidth) * .5f;
    ImGui::SetCursorPosX(zDKUILeft);
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos();
    auto* zDKUIDrawList = ImGui::GetWindowDrawList();
    zDKUIDrawSkin(
        zDKUIDrawList,
        zDKUISkinKind::zDKUIHeader,
        zDKUITopLeft,
        {zDKUITopLeft.x + zDKUIWidth, zDKUITopLeft.y + zDKUIScalePixels(68)},
        2,
        .94f
    );
    const bool zDKUITopmost = zDKUIIsAlwaysOnTop(zDKUIWindowHandle);
    const float zDKUITitleLeft = zDKUITopLeft.x + zDKUIScalePixels(48);
    zDKUIDrawList->AddText(
        zDKUITitleFont,
        (zDKUITitleFont->FontSize * ImGui::GetIO().FontGlobalScale),
        {zDKUITitleLeft, zDKUITopLeft.y + zDKUIScalePixels(7)},
        zDKUIPackColor(zDKUIWhite),
        "DOKKAN"
    );
    zDKUIDrawList->AddText(
        zDKUICaptionFont,
        (zDKUICaptionFont->FontSize * ImGui::GetIO().FontGlobalScale),
        {zDKUITitleLeft, zDKUITopLeft.y + zDKUIScalePixels(43)},
        zDKUIPackColor({1, .60f, .80f, 1}),
        "DEFENSE CALCULATOR"
    );
    if (zDKUITopmost)
    {
        ImGui::SetCursorScreenPos(
            {zDKUITopLeft.x + zDKUIWidth - zDKUIScalePixels(121), zDKUITopLeft.y + zDKUIScalePixels(14)}
        );
        ImVec2 zDKUIPinStart = ImGui::GetCursorScreenPos();
        ImGui::Dummy(zDKUIScaleSize(36, 38));
        zDKUIRecordControlProbe("topmost-indicator");
        const ImVec2 zDKUIPinCenter{
            zDKUIPinStart.x + zDKUIScalePixels(18), zDKUIPinStart.y + zDKUIScalePixels(19)
        };
        zDKUIDrawList->AddCircleFilled(
            zDKUIPinCenter, zDKUIScalePixels(17), zDKUIPackColor({1, .1f, .45f, .14f})
        );
        zDKUIDrawSettingsGlyph(
            zDKUIDrawList, zDKUIPinCenter, zDKUIScalePixels(13), zDKUIPackColor(zDKUIPink), true
        );
        zDKUIShowTooltip("Always on Top is enabled. Change it in Settings.");
    }
    ImGui::SetCursorPos(
        {zDKUILeft + zDKUIWidth - zDKUIScalePixels(78),
         zDKUITopLeft.y - ImGui::GetWindowPos().y + zDKUIScalePixels(12)}
    );
    if (zDKUIDrawButton("##settings", zDKUIScaleSize(43, 42)))
        ImGui::OpenPopup("settings");
    zDKUIRecordControlProbe("Settings");
    const auto zDKUISettingsMin = ImGui::GetItemRectMin(), zDKUISettingsMax = ImGui::GetItemRectMax();
    zDKUIDrawSettingsGlyph(
        zDKUIDrawList,
        {(zDKUISettingsMin.x + zDKUISettingsMax.x) * .5f, (zDKUISettingsMin.y + zDKUISettingsMax.y) * .5f},
        zDKUIScalePixels(12),
        zDKUIPackColor(ImGui::IsItemHovered() ? zDKUIPink : zDKUIWhite),
        false
    );
    zDKUIShowTooltip("Settings");
    bool zDKUIOpenReference = false, zDKUIOpenUpdates = false;
    if (ImGui::BeginPopup("settings"))
    {
        zDKUIDrawFontText(zDKUIHeadingFont, zDKUIWhite, "Settings");
        bool zDKUITopmostChoice = zDKUITopmost;
        const bool zDKUITopmostChanged = zDKUICheckbox("Always on Top", &zDKUITopmostChoice);
        if (zDKUITopmostChanged)
            if (!zDKUISetAlwaysOnTop(zDKUIWindowHandle, zDKUITopmostChoice))
                zDKUIShowNotice("Unable to change Always on Top.");
        zDKUIRecordControlProbe("Always on Top");
        zDKUIShowTooltip(
            "Keep this calculator above other windows. The pin beside Settings shows when it is enabled."
        );
        zDKUICheckbox("Animations & glow", &zDKUIAnimationsEnabled);
        if (ImGui::MenuItem("App updates"))
            zDKUIOpenUpdates = true;
        zDKUIRecordControlProbe("App updates");
        if (ImGui::MenuItem("Calculation reference"))
            zDKUIOpenReference = true;
        if (ImGui::MenuItem("New calculation"))
        {
            zDKUICurrentUnit = zDKEffectCreateEmptyUnitProfile();
            zDKUICurrentBoss = zDKEffectBossProfile{};
            zDKUICurrentBattle = zDKEffectBattleState{};
            zDKUISelectedPreset = -1;
            zDKUIActiveTab = 0;
            for (auto& zDKUISection : zDKUISelectedSubviews)
                zDKUISection = 0;
        }
        if (ImGui::MenuItem("Resume last session"))
            zDKUIShowNotice(
                zDKUIRestoreSavedSession() ? "Session restored." : "No valid saved session found."
            );
        ImGui::Separator();
        ImGui::TextDisabled("Interface scale: %.0f%%", zDKUIDisplayScale * 100);
        ImGui::EndPopup();
    }
    if (zDKUIOpenReference)
        ImGui::OpenPopup("Calculation reference");
    zDKUIDrawReferenceModal();
    if (zDKUIOpenUpdates)
        ImGui::OpenPopup("App updates");
    zDKUIDrawUpdatesModal(zDKUIWindowHandle);
    ImGui::SetCursorScreenPos({zDKUITopLeft.x, zDKUITopLeft.y + zDKUIScalePixels(76)});
    static const char* zDKUIPages[] = {"Unit", "Boss", "Info"};
    zDKUIDrawNavigationBar(
        "primary-nav", zDKUIPages, zDKUIActiveTab, {zDKUIWidth, zDKUIScalePixels(62)}, true
    );
    ImGui::SetCursorScreenPos({zDKUITopLeft.x, zDKUITopLeft.y + zDKUIScalePixels(146)});
    ImGui::BeginChild(
        "battle-context",
        {zDKUIWidth, zDKUIScalePixels(88)},
        ImGuiChildFlags_None,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar
    );
    static const char* zDKUIMoments[] = {"Before attack", "After attack"};
    int zDKUIMoment = zDKUICurrentBattle.zDKEffectAfterUnitAttack ? 1 : 0;
    zDKUIDrawFieldPair(
        "Turn",
        "Attack timing",
        [&] { zDKUIDrawIntegerField("turn", zDKUICurrentBattle.zDKEffectTurn); },
        [&] { zDKUIDrawComboField("moment", zDKUIMoment, zDKUIMoments, 2); },
        true,
        2.4f
    );
    zDKUICurrentBattle.zDKEffectAfterUnitAttack = zDKUIMoment == 1;
    ImGui::EndChild();
    float zDKUINavHeight = zDKUIScalePixels(58),
          zDKUIBottom = ImGui::GetWindowHeight() - zDKUIScalePixels(12);
    float zDKUIContentY = zDKUITopLeft.y + zDKUIScalePixels(242);
    if (ImGui::GetTime() < zDKUIStatusUntil)
    {
        ImGui::SetCursorScreenPos({zDKUITopLeft.x + zDKUIScalePixels(12), zDKUIContentY});
        ImGui::PushFont(zDKUICaptionFont);
        zDKUIDrawWrappedText(zDKUIStatus, zDKUIPink);
        ImGui::PopFont();
        zDKUIContentY = ImGui::GetCursorScreenPos().y + zDKUIScalePixels(3);
    }
    float zDKUINavY = ImGui::GetWindowPos().y + zDKUIBottom - zDKUINavHeight;
    ImGui::SetCursorScreenPos({zDKUITopLeft.x, zDKUINavY});
    static const char* zDKUIUnitSections[] = {"Stats", "Effects", "Setup"};
    static const char* zDKUIBossSections[] = {"Stats", "Effects", "Skills"};
    static const char* zDKUIInfoSections[] = {"Damage", "Tanking", "Turn", "Calcs"};
    zDKUIDrawNavigationBar(
        "secondary-nav",
        zDKUIActiveTab == 0       ? zDKUIUnitSections
            : zDKUIActiveTab == 1 ? zDKUIBossSections
                                  : zDKUIInfoSections,
        zDKUISelectedSubviews[zDKUIActiveTab],
        {zDKUIWidth, zDKUINavHeight},
        false,
        zDKUIActiveTab == 2 ? 4 : 3
    );
    int zDKUIDesired = zDKUIActiveTab * 4 + zDKUISelectedSubviews[zDKUIActiveTab];
    if (zDKUIDisplayedTab < 0)
    {
        zDKUIDisplayedTab = zDKUIActiveTab;
        zDKUIDisplayedSection = zDKUISelectedSubviews[zDKUIActiveTab];
    }
    int zDKUIDisplayed = zDKUIDisplayedTab * 4 + zDKUIDisplayedSection;
    if (!zDKUIAnimationsEnabled)
    {
        zDKUIDisplayedTab = zDKUIActiveTab;
        zDKUIDisplayedSection = zDKUISelectedSubviews[zDKUIActiveTab];
        zDKUIPageProgress = 1;
    }
    else if (zDKUIDisplayed != zDKUIDesired)
    {
        zDKUIPageDirection = zDKUIDesired > zDKUIDisplayed ? 1 : -1;
        zDKUIPageProgress =
            std::max(0.f, zDKUIPageProgress - std::min(ImGui::GetIO().DeltaTime, .05f) / .13f);
        if (zDKUIPageProgress == 0)
        {
            zDKUIDisplayedTab = zDKUIActiveTab;
            zDKUIDisplayedSection = zDKUISelectedSubviews[zDKUIActiveTab];
        }
    }
    else
        zDKUIPageProgress =
            std::min(1.f, zDKUIPageProgress + std::min(ImGui::GetIO().DeltaTime, .05f) / .20f);
    float zDKUIEased = zDKUIPageProgress * zDKUIPageProgress * (3 - 2 * zDKUIPageProgress);
    float zDKUIOffset = zDKUIScalePixels(14) * (1 - zDKUIEased) *
        (zDKUIDisplayedTab * 4 + zDKUIDisplayedSection == zDKUIDesired ? zDKUIPageDirection
                                                                       : -zDKUIPageDirection);
    ImGui::SetCursorScreenPos({zDKUITopLeft.x + zDKUIOffset, zDKUIContentY});
    ImVec2 zDKUIContentSize = {
        zDKUIWidth, std::max(zDKUIScalePixels(180), zDKUINavY - zDKUIContentY - zDKUIScalePixels(9))
    };
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * zDKUIEased);
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1);
    ImGui::BeginDisabled(zDKUIPageProgress < .99f);
    if (zDKUIDisplayedTab == 0)
        zDKUIDrawUnitPanel(zDKUIWindowHandle, zDKUIContentSize);
    else if (zDKUIDisplayedTab == 1)
        zDKUIDrawBossPanel(zDKUIContentSize);
    else
        zDKUIDrawAnalysisPanel(zDKUIContentSize);
    ImGui::EndDisabled();
    ImGui::PopStyleVar(2);
    zDKUIDrawEffectModal();
    zDKUIDrawTurnPlanModal();
    ImGui::End();
    ImGui::PopStyleVar();
}
} // namespace zDKUI
