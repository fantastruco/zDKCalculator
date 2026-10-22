#pragma once
#include "zDKUIAssets.h"

namespace zDKUI
{
extern std::array<std::array<ComPtr<ID3D11ShaderResourceView>, 3>, int(zDKUISkinKind::zDKUICount)>
    zDKUISkinTextures;
extern float zDKUIDisplayScale;
extern bool zDKUIAnimationsEnabled;
void zDKUIBlockGestureInput();
bool zDKUISuppressGestureClick();

float zDKUIAnimateValue(ImGuiID zDKUIKey, float zDKUIDestination, float zDKUISeconds = .10f)
{
    auto* zDKUIStorage = ImGui::GetStateStorage();
    float zDKUIValue = zDKUIStorage->GetFloat(zDKUIKey, 0);
    zDKUIValue = zDKUIAnimationsEnabled ? zDKUIValue +
            (zDKUIDestination - zDKUIValue) *
                (1 - std::exp(-ImGui::GetIO().DeltaTime / std::max(.01f, zDKUISeconds)))
                                        : zDKUIDestination;
    if (std::abs(zDKUIValue - zDKUIDestination) < .001f)
        zDKUIValue = zDKUIDestination;
    zDKUIStorage->SetFloat(zDKUIKey, zDKUIValue);
    return zDKUIValue;
}

void zDKUIDrawOutlinedText(
    ImDrawList* zDKUIDrawList,
    ImFont* zDKUIFont,
    float zDKUISize,
    ImVec2 zDKUIPos,
    ImVec4 zDKUIColor,
    const char* zDKUIText,
    float zDKUIWrap = 0
)
{
    const float zDKUIOffset = zDKUIDisplayScale;
    const ImU32 zDKUIOutline = ImGui::GetColorU32(ImVec4(0, 0, 0, .92f));
    for (int zDKUIVerticalPosition = -1; zDKUIVerticalPosition <= 1; ++zDKUIVerticalPosition)
        for (int zDKUIHorizontalPosition = -1; zDKUIHorizontalPosition <= 1; ++zDKUIHorizontalPosition)
            if (zDKUIHorizontalPosition || zDKUIVerticalPosition)
                zDKUIDrawList->AddText(
                    zDKUIFont,
                    zDKUISize,
                    {zDKUIPos.x + zDKUIHorizontalPosition * zDKUIOffset,
                     zDKUIPos.y + zDKUIVerticalPosition * zDKUIOffset},
                    zDKUIOutline,
                    zDKUIText,
                    nullptr,
                    zDKUIWrap
                );
    zDKUIDrawList->AddText(
        zDKUIFont, zDKUISize, zDKUIPos, ImGui::GetColorU32(zDKUIColor), zDKUIText, nullptr, zDKUIWrap
    );
}

void zDKUIDrawControlText(
    const char* zDKUILabel,
    ImVec2 zDKUITopLeft,
    ImVec2 zDKUIBottomRight,
    float zDKUIInset,
    bool zDKUICentered = true
)
{
    std::string zDKUIText = zDKUILabel;
    auto zDKUIHidden = zDKUIText.find("##");
    if (zDKUIHidden != std::string::npos)
        zDKUIText.resize(zDKUIHidden);
    auto* zDKUIFont = ImGui::GetFont();
    float zDKUISize = (zDKUIFont->FontSize * ImGui::GetIO().FontGlobalScale),
          zDKUIAvailable = std::max(1.f, zDKUIBottomRight.x - zDKUITopLeft.x - zDKUIInset * 2);
    ImVec2 zDKUIMeasured = zDKUIFont->CalcTextSizeA(zDKUISize, FLT_MAX, 0, zDKUIText.c_str());
    if (zDKUIMeasured.x > zDKUIAvailable)
        zDKUISize *= std::max(.87f, zDKUIAvailable / zDKUIMeasured.x);
    if (zDKUIFont->CalcTextSizeA(zDKUISize, FLT_MAX, 0, zDKUIText.c_str()).x > zDKUIAvailable)
    {
        while (!zDKUIText.empty() &&
               zDKUIFont->CalcTextSizeA(zDKUISize, FLT_MAX, 0, (zDKUIText + "…").c_str()).x > zDKUIAvailable)
        {
            size_t zDKUILast = zDKUIText.size() - 1;
            while (zDKUILast > 0 && (static_cast<unsigned char>(zDKUIText[zDKUILast]) & 0xc0) == 0x80)
                --zDKUILast;
            zDKUIText.resize(zDKUILast);
        }
        zDKUIText += "…";
    }
    zDKUIMeasured = zDKUIFont->CalcTextSizeA(zDKUISize, FLT_MAX, 0, zDKUIText.c_str());
    ImVec2 zDKUIPos = {
        zDKUICentered ? zDKUITopLeft.x + (zDKUIBottomRight.x - zDKUITopLeft.x - zDKUIMeasured.x) * .5f
                      : zDKUITopLeft.x + zDKUIInset,
        zDKUITopLeft.y + (zDKUIBottomRight.y - zDKUITopLeft.y - zDKUIMeasured.y) * .5f
    };
    ImGui::GetWindowDrawList()->PushClipRect(
        {zDKUITopLeft.x + zDKUIInset, zDKUITopLeft.y},
        {zDKUIBottomRight.x - zDKUIInset, zDKUIBottomRight.y},
        true
    );
    zDKUIDrawOutlinedText(
        ImGui::GetWindowDrawList(),
        zDKUIFont,
        zDKUISize,
        zDKUIPos,
        ImGui::GetStyleColorVec4(ImGuiCol_Text),
        zDKUIText.c_str()
    );
    ImGui::GetWindowDrawList()->PopClipRect();
}

void zDKUIDrawSkin(
    ImDrawList* zDKUIDrawList,
    zDKUISkinKind zDKUIKind,
    ImVec2 zDKUITopLeft,
    ImVec2 zDKUIBottomRight,
    int zDKUIState = 0,
    float zDKUIOpacity = 1
)
{
    if (zDKUIBottomRight.x <= zDKUITopLeft.x || zDKUIBottomRight.y <= zDKUITopLeft.y || zDKUIOpacity <= 0)
        return;
    const auto& zDKUIEffectRow = zDKUISkinDefinitions[int(zDKUIKind)];
    auto* zDKUITexture = zDKUISkinTextures[int(zDKUIKind)][zDKUIState].Get();
    if (!zDKUITexture)
        return;
    ImTextureID zDKUIId = (ImTextureID)(intptr_t)zDKUITexture;
    ImU32 zDKUITint =
        IM_COL32(255, 255, 255, int(std::clamp(zDKUIOpacity * ImGui::GetStyle().Alpha, 0.f, 1.f) * 255));
    if (!zDKUIEffectRow.zDKUILeft && !zDKUIEffectRow.zDKUITop && !zDKUIEffectRow.zDKUIRight &&
        !zDKUIEffectRow.zDKUIBottom)
    {
        zDKUIDrawList->AddImage(zDKUIId, zDKUITopLeft, zDKUIBottomRight, {0, 0}, {1, 1}, zDKUITint);
        return;
    }
    // Keep corners in logical pixels while only the middle strips stretch.
    float zDKUIScale = (zDKUIBottomRight.y - zDKUITopLeft.y) / float(zDKUIEffectRow.zDKUIHeight);
    if (zDKUIKind == zDKUISkinKind::zDKUIPanel)
        zDKUIScale = .42f * zDKUIDisplayScale;
    float zDKUILabel =
              std::min(zDKUIEffectRow.zDKUILeft * zDKUIScale, (zDKUIBottomRight.x - zDKUITopLeft.x) * .44f),
          zDKUIResult =
              std::min(zDKUIEffectRow.zDKUIRight * zDKUIScale, (zDKUIBottomRight.x - zDKUITopLeft.x) * .44f);
    float zDKUITypeIndex =
              std::min(zDKUIEffectRow.zDKUITop * zDKUIScale, (zDKUIBottomRight.y - zDKUITopLeft.y) * .44f),
          zDKUIIsBossTab =
              std::min(zDKUIEffectRow.zDKUIBottom * zDKUIScale, (zDKUIBottomRight.y - zDKUITopLeft.y) * .44f);
    const float zDKUIHorizontalPosition[] = {
        zDKUITopLeft.x, zDKUITopLeft.x + zDKUILabel, zDKUIBottomRight.x - zDKUIResult, zDKUIBottomRight.x
    };
    const float zDKUIVerticalPosition[] = {
        zDKUITopLeft.y,
        zDKUITopLeft.y + zDKUITypeIndex,
        zDKUIBottomRight.y - zDKUIIsBossTab,
        zDKUIBottomRight.y
    };
    const float zDKUIUnit[] = {
        0,
        float(zDKUIEffectRow.zDKUILeft) / zDKUIEffectRow.zDKUIWidth,
        1 - float(zDKUIEffectRow.zDKUIRight) / zDKUIEffectRow.zDKUIWidth,
        1
    };
    const float zDKUIValue[] = {
        0,
        float(zDKUIEffectRow.zDKUITop) / zDKUIEffectRow.zDKUIHeight,
        1 - float(zDKUIEffectRow.zDKUIBottom) / zDKUIEffectRow.zDKUIHeight,
        1
    };
    for (int zDKUIRowIndex = 0; zDKUIRowIndex < 3; ++zDKUIRowIndex)
        for (int zDKUIColumnIndex = 0; zDKUIColumnIndex < 3; ++zDKUIColumnIndex)
        {
            if (zDKUIHorizontalPosition[zDKUIColumnIndex + 1] > zDKUIHorizontalPosition[zDKUIColumnIndex] &&
                zDKUIVerticalPosition[zDKUIRowIndex + 1] > zDKUIVerticalPosition[zDKUIRowIndex])
                zDKUIDrawList->AddImage(
                    zDKUIId,
                    {zDKUIHorizontalPosition[zDKUIColumnIndex], zDKUIVerticalPosition[zDKUIRowIndex]},
                    {zDKUIHorizontalPosition[zDKUIColumnIndex + 1], zDKUIVerticalPosition[zDKUIRowIndex + 1]},
                    {zDKUIUnit[zDKUIColumnIndex], zDKUIValue[zDKUIRowIndex]},
                    {zDKUIUnit[zDKUIColumnIndex + 1], zDKUIValue[zDKUIRowIndex + 1]},
                    zDKUITint
                );
        }
}

void zDKUIPaintControl(
    ImDrawList* zDKUIDrawList,
    zDKUISkinKind zDKUIKind,
    ImVec2 zDKUITopLeft,
    ImVec2 zDKUIBottomRight,
    bool zDKUIHovered,
    bool zDKUIIsActive,
    ImGuiID zDKUIId
)
{
    zDKUIDrawSkin(zDKUIDrawList, zDKUIKind, zDKUITopLeft, zDKUIBottomRight, 0);
    float zDKUIHover = zDKUIAnimateValue(zDKUIId ^ 0x719ab3u, zDKUIHovered ? 1.f : 0.f),
          zDKUISelected = zDKUIAnimateValue(zDKUIId ^ 0x41fc87u, zDKUIIsActive ? 1.f : 0.f);
    float zDKUIPulse = zDKUIAnimationsEnabled ? (.96f + .04f * std::sin(float(ImGui::GetTime()) * 2.8f)) : 1;
    zDKUIDrawSkin(
        zDKUIDrawList, zDKUIKind, zDKUITopLeft, zDKUIBottomRight, 1, zDKUIHover * (1 - zDKUISelected)
    );
    zDKUIDrawSkin(zDKUIDrawList, zDKUIKind, zDKUITopLeft, zDKUIBottomRight, 2, zDKUISelected * zDKUIPulse);
}

// Own splitters allow skinned backgrounds below native widgets, including inside tables.
struct zDKUISkinLayer
{
    ImDrawList* zDKUIDrawList;
    ImDrawListSplitter zDKUIDrawChannels;

    zDKUISkinLayer() : zDKUIDrawList(ImGui::GetWindowDrawList())
    {
        zDKUIDrawChannels.Split(zDKUIDrawList, 2);
        zDKUIDrawChannels.SetCurrentChannel(zDKUIDrawList, 1);
    }

    void zDKUIPaint(
        zDKUISkinKind zDKUIKind,
        ImVec2 zDKUITopLeft,
        ImVec2 zDKUIBottomRight,
        bool zDKUIHovered,
        bool zDKUIIsActive
    )
    {
        ImGuiID zDKUIId = ImGui::GetItemID();
        zDKUIDrawChannels.SetCurrentChannel(zDKUIDrawList, 0);
        zDKUIPaintControl(
            zDKUIDrawList, zDKUIKind, zDKUITopLeft, zDKUIBottomRight, zDKUIHovered, zDKUIIsActive, zDKUIId
        );
    }

    ~zDKUISkinLayer()
    {
        zDKUIDrawChannels.Merge(zDKUIDrawList);
    }
};

void zDKUIBeginTransparentControl()
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0, 0, 0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
}

void zDKUIEndTransparentControl()
{
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(6);
}

void zDKUISkinLastItem(zDKUISkinLayer& zDKUILayer, zDKUISkinKind zDKUISkin)
{
    zDKUILayer.zDKUIPaint(
        zDKUISkin,
        ImGui::GetItemRectMin(),
        ImGui::GetItemRectMax(),
        ImGui::IsItemHovered(),
        ImGui::IsItemActive() || ImGui::IsItemFocused()
    );
}

bool zDKUIBeginCombo(const char* zDKUIId, const char* zDKUIPreview)
{
    ImGui::SetNextItemWidth(-1);
    ImGui::SetNextWindowSizeConstraints(
        {0, 0},
        {std::max(100.f, ImGui::GetIO().DisplaySize.x - 24 * zDKUIDisplayScale),
         std::max(100.f, ImGui::GetIO().DisplaySize.y - 32 * zDKUIDisplayScale)}
    );
    ImVec2 zDKUITopLeft = ImGui::GetCursorScreenPos(),
           zDKUIBottomRight = {
               zDKUITopLeft.x + ImGui::GetContentRegionAvail().x, zDKUITopLeft.y + ImGui::GetFrameHeight()
           };
    if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(zDKUITopLeft, zDKUIBottomRight))
        zDKUIBlockGestureInput();
    zDKUIPaintControl(
        ImGui::GetWindowDrawList(),
        zDKUISkinKind::zDKUIField,
        zDKUITopLeft,
        zDKUIBottomRight,
        ImGui::IsMouseHoveringRect(zDKUITopLeft, zDKUIBottomRight),
        ImGui::IsPopupOpen(zDKUIId),
        ImGui::GetID(zDKUIId)
    );
    std::string zDKUIFitted = zDKUIPreview;
    float zDKUIAvailable = std::max(
        1.f,
        zDKUIBottomRight.x - zDKUITopLeft.x - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x * 2
    );
    if (ImGui::CalcTextSize(zDKUIPreview).x > zDKUIAvailable)
    {
        while (!zDKUIFitted.empty() && ImGui::CalcTextSize((zDKUIFitted + "…").c_str()).x > zDKUIAvailable)
        {
            size_t zDKUILast = zDKUIFitted.size() - 1;
            while (zDKUILast > 0 && (static_cast<unsigned char>(zDKUIFitted[zDKUILast]) & 0xc0) == 0x80)
                --zDKUILast;
            zDKUIFitted.resize(zDKUILast);
        }
        zDKUIFitted += "…";
    }
    zDKUIBeginTransparentControl();
    bool zDKUIOpen = ImGui::BeginCombo(zDKUIId, zDKUIFitted.c_str());
    zDKUIEndTransparentControl();
    if (!zDKUIOpen && zDKUIFitted != zDKUIPreview && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", zDKUIPreview);
    return zDKUIOpen;
}

bool zDKUICheckbox(const char* zDKUILabel, bool* zDKUIValue)
{
    zDKUISkinLayer zDKUILayer;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4 * zDKUIDisplayScale, 4 * zDKUIDisplayScale});
    zDKUIBeginTransparentControl();
    bool zDKUIChanged = ImGui::Checkbox(zDKUILabel, zDKUIValue);
    zDKUIEndTransparentControl();
    if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered())
        zDKUIBlockGestureInput();
    ImVec2 zDKUITopLeft = ImGui::GetItemRectMin();
    float zDKUISize = ImGui::GetFrameHeight();
    zDKUILayer.zDKUIPaint(
        zDKUISkinKind::zDKUIToggle,
        zDKUITopLeft,
        {zDKUITopLeft.x + zDKUISize, zDKUITopLeft.y + zDKUISize},
        ImGui::IsItemHovered(),
        *zDKUIValue
    );
    ImGui::PopStyleVar();
    return zDKUIChanged;
}

bool zDKUIDisclosure(const char* zDKUILabel, bool zDKUIForceOpen = false)
{
    ImGuiID zDKUIId = ImGui::GetID(zDKUILabel);
    bool zDKUIOpen = ImGui::GetStateStorage()->GetBool(zDKUIId, false);
    if (zDKUIForceOpen)
        zDKUIOpen = true;
    zDKUISkinLayer zDKUILayer;
    zDKUIBeginTransparentControl();
    float zDKUIWidth = ImGui::GetContentRegionAvail().x;
    ImGui::PushID(zDKUILabel);
    bool zDKUIPressed = ImGui::Button("##disclosure", {zDKUIWidth, 50 * zDKUIDisplayScale});
    ImGui::PopID();
    zDKUIEndTransparentControl();
    if (zDKUIPressed && !zDKUISuppressGestureClick())
        zDKUIOpen = !zDKUIOpen;
    ImGui::GetStateStorage()->SetBool(zDKUIId, zDKUIOpen);
    ImVec2 zDKUITopLeft = ImGui::GetItemRectMin(), zDKUIBottomRight = ImGui::GetItemRectMax();
    zDKUILayer.zDKUIPaint(
        zDKUISkinKind::zDKUIEffectRow, zDKUITopLeft, zDKUIBottomRight, ImGui::IsItemHovered(), zDKUIOpen
    );
    zDKUILayer.zDKUIDrawChannels.SetCurrentChannel(zDKUILayer.zDKUIDrawList, 1);
    zDKUIDrawControlText(
        zDKUILabel,
        {zDKUITopLeft.x + 17 * zDKUIDisplayScale, zDKUITopLeft.y},
        zDKUIBottomRight,
        22 * zDKUIDisplayScale,
        false
    );
    const float zDKUIHorizontalPosition = zDKUITopLeft.x + 24 * zDKUIDisplayScale,
                zDKUIVerticalPosition = (zDKUITopLeft.y + zDKUIBottomRight.y) * .5f,
                zDKUIText = 3.5f * zDKUIDisplayScale;
    float zDKUIAngle = zDKUIAnimateValue(zDKUIId ^ 0x71832u, zDKUIOpen ? 1.f : 0.f) * 1.5707963f;
    auto zDKUIPoint = [&](float zDKUIHorizontalDelta, float zDKUIVerticalDelta)
    {
        return ImVec2{
            zDKUIHorizontalPosition + std::cos(zDKUIAngle) * zDKUIHorizontalDelta -
                std::sin(zDKUIAngle) * zDKUIVerticalDelta,
            zDKUIVerticalPosition + std::sin(zDKUIAngle) * zDKUIHorizontalDelta +
                std::cos(zDKUIAngle) * zDKUIVerticalDelta
        };
    };
    zDKUILayer.zDKUIDrawList->AddTriangleFilled(
        zDKUIPoint(-zDKUIText * .5f, -zDKUIText),
        zDKUIPoint(-zDKUIText * .5f, zDKUIText),
        zDKUIPoint(zDKUIText, 0),
        ImGui::GetColorU32(ImVec4(1, .64f, .82f, 1))
    );
    return zDKUIOpen;
}
} // namespace zDKUI
