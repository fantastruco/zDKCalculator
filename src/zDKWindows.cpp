#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <wrl/client.h>
#include <chrono>
#include <vector>
#include <filesystem>
#include <array>
#include <functional>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "zDKUIAssets.h"
#include "zDKUpdater.h"
using Microsoft::WRL::ComPtr;

namespace zDKUI
{
ComPtr<ID3D11Device> zDKWinDevice;
ComPtr<ID3D11DeviceContext> zDKWinDeviceContext;
ComPtr<IDXGISwapChain> zDKWinSwapChain;
ComPtr<ID3D11RenderTargetView> zDKWinRenderTarget;
ComPtr<ID3D11ShaderResourceView> zDKUIBackgroundTexture;
SIZE zDKWinInitialWindowSize{540, 980};
bool zDKWinInSizeMove = false, zDKWinRendering = false;
RECT zDKWinResizeStart{};
std::function<void()> zDKWinRenderDuringResize;
float zDKWinFontScale = 1;
UINT zDKWinWindowDpi = 96;
SIZE zDKWinFrameSize(HWND zDKWinWindow, UINT zDKWinDpi = 0);
SIZE zDKWinPortraitLimit(HWND zDKWinWindow, bool zDKWinMinimum);
void zDKWinConstrainResize(HWND zDKWinWindow, WPARAM zDKWinEdge, RECT& zDKWinBounds);
HWND zDKWinApplicationWindow = nullptr;
bool zDKWinDiagnosticWindow = false;
HICON zDKWinLargeIcon = nullptr, zDKWinSmallIcon = nullptr;

bool zDKWinUpdateWindowIcons(HWND zDKWinWindow, UINT zDKWinDpi)
{
    auto zDKWinInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(zDKWinWindow, GWLP_HINSTANCE));
    auto zDKWinLoadIcon = [&](int zDKWinWidthMetric, int zDKWinHeightMetric)
    {
        return static_cast<HICON>(LoadImageW(
            zDKWinInstance,
            MAKEINTRESOURCEW(1),
            IMAGE_ICON,
            GetSystemMetricsForDpi(zDKWinWidthMetric, zDKWinDpi),
            GetSystemMetricsForDpi(zDKWinHeightMetric, zDKWinDpi),
            0
        ));
    };
    HICON zDKWinNewLarge = zDKWinLoadIcon(SM_CXICON, SM_CYICON);
    HICON zDKWinNewSmall = zDKWinLoadIcon(SM_CXSMICON, SM_CYSMICON);
    if (!zDKWinNewLarge || !zDKWinNewSmall)
    {
        if (zDKWinNewLarge)
            DestroyIcon(zDKWinNewLarge);
        if (zDKWinNewSmall)
            DestroyIcon(zDKWinNewSmall);
        return false;
    }
    SendMessageW(zDKWinWindow, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(zDKWinNewLarge));
    SendMessageW(zDKWinWindow, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(zDKWinNewSmall));
    if (zDKWinLargeIcon)
        DestroyIcon(zDKWinLargeIcon);
    if (zDKWinSmallIcon)
        DestroyIcon(zDKWinSmallIcon);
    zDKWinLargeIcon = zDKWinNewLarge;
    zDKWinSmallIcon = zDKWinNewSmall;
    return true;
}

std::array<std::array<ComPtr<ID3D11ShaderResourceView>, 3>, int(zDKUISkinKind::zDKUICount)> zDKUISkinTextures;
UINT zDKWinPendingWidth = 0, zDKWinPendingHeight = 0;
float zDKUIDisplayScale = 1, zDKUIPendingDisplayScale = 0, zDKUIScaleOverride = 0;
ImFont *zDKUIBodyFont = nullptr, *zDKUIHeadingFont = nullptr, *zDKUITitleFont = nullptr,
       *zDKUIMetricFont = nullptr, *zDKUICaptionFont = nullptr, *zDKUILabelFont = nullptr;
} // namespace zDKUI

#include "zDKUI.h"
#include "../tests/zDKUIInteractionTests.h"

namespace zDKUI
{
void zDKWinCreateRenderTarget()
{
    ComPtr<ID3D11Texture2D> zDKWinBuffer;
    if (SUCCEEDED(zDKWinSwapChain->GetBuffer(0, IID_PPV_ARGS(&zDKWinBuffer))))
        zDKWinDevice->CreateRenderTargetView(zDKWinBuffer.Get(), nullptr, &zDKWinRenderTarget);
}

bool zDKWinInitializeDirect3D(HWND zDKWinWindowHandle)
{
    DXGI_SWAP_CHAIN_DESC zDKWinSwapChainDescription{};
    zDKWinSwapChainDescription.BufferCount = 2;
    zDKWinSwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    zDKWinSwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    zDKWinSwapChainDescription.OutputWindow = zDKWinWindowHandle;
    zDKWinSwapChainDescription.SampleDesc.Count = 1;
    zDKWinSwapChainDescription.Windowed = TRUE;
    zDKWinSwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    const D3D_FEATURE_LEVEL zDKWinSupportedFeatureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL zDKWinSelectedFeatureLevel;
    HRESULT zDKWinStatusCode = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        zDKWinSupportedFeatureLevels,
        2,
        D3D11_SDK_VERSION,
        &zDKWinSwapChainDescription,
        &zDKWinSwapChain,
        &zDKWinDevice,
        &zDKWinSelectedFeatureLevel,
        &zDKWinDeviceContext
    );
    if (FAILED(zDKWinStatusCode))
        zDKWinStatusCode = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            0,
            zDKWinSupportedFeatureLevels,
            2,
            D3D11_SDK_VERSION,
            &zDKWinSwapChainDescription,
            &zDKWinSwapChain,
            &zDKWinDevice,
            &zDKWinSelectedFeatureLevel,
            &zDKWinDeviceContext
        );
    if (FAILED(zDKWinStatusCode))
        return false;
    zDKWinCreateRenderTarget();
    return bool(zDKWinRenderTarget);
}

bool zDKWinLoadTexture(int zDKWinResourceId, ComPtr<ID3D11ShaderResourceView>& zDKWinOutputTexture)
{
    HRSRC zDKWinResource = FindResourceW(nullptr, MAKEINTRESOURCEW(zDKWinResourceId), RT_RCDATA);
    if (!zDKWinResource)
        return false;
    auto zDKWinData = static_cast<BYTE*>(LockResource(LoadResource(nullptr, zDKWinResource)));
    DWORD zDKWinSize = SizeofResource(nullptr, zDKWinResource);
    ComPtr<IWICImagingFactory> zDKWinFactory;
    ComPtr<IWICStream> zDKWinStream;
    ComPtr<IWICBitmapDecoder> zDKWinDecoder;
    ComPtr<IWICBitmapFrameDecode> zDKWinFrame;
    ComPtr<IWICFormatConverter> zDKWinConverter;
    if (FAILED(CoCreateInstance(
            CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&zDKWinFactory)
        )) ||
        FAILED(zDKWinFactory->CreateStream(&zDKWinStream)) ||
        FAILED(zDKWinStream->InitializeFromMemory(zDKWinData, zDKWinSize)) ||
        FAILED(zDKWinFactory->CreateDecoderFromStream(
            zDKWinStream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &zDKWinDecoder
        )) ||
        FAILED(zDKWinDecoder->GetFrame(0, &zDKWinFrame)) ||
        FAILED(zDKWinFactory->CreateFormatConverter(&zDKWinConverter)))
        return false;
    if (FAILED(zDKWinConverter->Initialize(
            zDKWinFrame.Get(),
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr,
            0,
            WICBitmapPaletteTypeCustom
        )))
        return false;
    UINT zDKWinWidth, zDKWinHeight;
    zDKWinConverter->GetSize(&zDKWinWidth, &zDKWinHeight);
    std::vector<BYTE> zDKWinPixels(size_t(zDKWinWidth) * zDKWinHeight * 4);
    if (FAILED(zDKWinConverter->CopyPixels(
            nullptr, zDKWinWidth * 4, UINT(zDKWinPixels.size()), zDKWinPixels.data()
        )))
        return false;
    D3D11_TEXTURE2D_DESC zDKWinTextureDescription{};
    zDKWinTextureDescription.Width = zDKWinWidth;
    zDKWinTextureDescription.Height = zDKWinHeight;
    zDKWinTextureDescription.MipLevels = 1;
    zDKWinTextureDescription.ArraySize = 1;
    zDKWinTextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    zDKWinTextureDescription.SampleDesc.Count = 1;
    zDKWinTextureDescription.Usage = D3D11_USAGE_IMMUTABLE;
    zDKWinTextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA zDKWinTextureData{zDKWinPixels.data(), zDKWinWidth * 4, 0};
    ComPtr<ID3D11Texture2D> zDKWinTexture;
    if (FAILED(zDKWinDevice->CreateTexture2D(&zDKWinTextureDescription, &zDKWinTextureData, &zDKWinTexture)))
        return false;
    return SUCCEEDED(
        zDKWinDevice->CreateShaderResourceView(zDKWinTexture.Get(), nullptr, &zDKWinOutputTexture)
    );
}

bool zDKWinCaptureFrame(const std::filesystem::path& zDKWinPath)
{
    ComPtr<ID3D11Texture2D> zDKWinBuffer, zDKWinStaging;
    if (FAILED(zDKWinSwapChain->GetBuffer(0, IID_PPV_ARGS(&zDKWinBuffer))))
        return false;
    D3D11_TEXTURE2D_DESC zDKWinTextureDescription{};
    zDKWinBuffer->GetDesc(&zDKWinTextureDescription);
    zDKWinTextureDescription.Usage = D3D11_USAGE_STAGING;
    zDKWinTextureDescription.BindFlags = 0;
    zDKWinTextureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    zDKWinTextureDescription.MiscFlags = 0;
    if (FAILED(zDKWinDevice->CreateTexture2D(&zDKWinTextureDescription, nullptr, &zDKWinStaging)))
        return false;
    zDKWinDeviceContext->CopyResource(zDKWinStaging.Get(), zDKWinBuffer.Get());
    D3D11_MAPPED_SUBRESOURCE zDKWinMap{};
    if (FAILED(zDKWinDeviceContext->Map(zDKWinStaging.Get(), 0, D3D11_MAP_READ, 0, &zDKWinMap)))
        return false;
    ComPtr<IWICImagingFactory> zDKWinFactory;
    ComPtr<IWICStream> zDKWinStream;
    ComPtr<IWICBitmapEncoder> zDKWinEncoder;
    ComPtr<IWICBitmapFrameEncode> zDKWinFrame;
    bool zDKWinSucceeded = false;
    if (SUCCEEDED(CoCreateInstance(
            CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&zDKWinFactory)
        )) &&
        SUCCEEDED(zDKWinFactory->CreateStream(&zDKWinStream)) &&
        SUCCEEDED(zDKWinStream->InitializeFromFilename(zDKWinPath.c_str(), GENERIC_WRITE)) &&
        SUCCEEDED(zDKWinFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &zDKWinEncoder)) &&
        SUCCEEDED(zDKWinEncoder->Initialize(zDKWinStream.Get(), WICBitmapEncoderNoCache)) &&
        SUCCEEDED(zDKWinEncoder->CreateNewFrame(&zDKWinFrame, nullptr)) &&
        SUCCEEDED(zDKWinFrame->Initialize(nullptr)) &&
        SUCCEEDED(zDKWinFrame->SetSize(zDKWinTextureDescription.Width, zDKWinTextureDescription.Height)))
    {
        WICPixelFormatGUID zDKWinFormat = GUID_WICPixelFormat32bppBGRA;
        std::vector<BYTE> zDKWinPixels(
            size_t(zDKWinTextureDescription.Width) * zDKWinTextureDescription.Height * 4
        );
        for (UINT zDKWinVerticalPosition = 0; zDKWinVerticalPosition < zDKWinTextureDescription.Height;
             ++zDKWinVerticalPosition)
        {
            auto* zDKWinSource =
                static_cast<BYTE*>(zDKWinMap.pData) + size_t(zDKWinVerticalPosition) * zDKWinMap.RowPitch;
            auto* zDKWinDestinationPixels =
                zDKWinPixels.data() + size_t(zDKWinVerticalPosition) * zDKWinTextureDescription.Width * 4;
            for (UINT zDKWinHorizontalPosition = 0; zDKWinHorizontalPosition < zDKWinTextureDescription.Width;
                 ++zDKWinHorizontalPosition)
            {
                zDKWinDestinationPixels[4 * zDKWinHorizontalPosition] =
                    zDKWinSource[4 * zDKWinHorizontalPosition + 2];
                zDKWinDestinationPixels[4 * zDKWinHorizontalPosition + 1] =
                    zDKWinSource[4 * zDKWinHorizontalPosition + 1];
                zDKWinDestinationPixels[4 * zDKWinHorizontalPosition + 2] =
                    zDKWinSource[4 * zDKWinHorizontalPosition];
                zDKWinDestinationPixels[4 * zDKWinHorizontalPosition + 3] = 255;
            }
        }
        zDKWinSucceeded = SUCCEEDED(zDKWinFrame->SetPixelFormat(&zDKWinFormat)) &&
            IsEqualGUID(zDKWinFormat, GUID_WICPixelFormat32bppBGRA) &&
            SUCCEEDED(zDKWinFrame->WritePixels(
                zDKWinTextureDescription.Height,
                zDKWinTextureDescription.Width * 4,
                UINT(zDKWinPixels.size()),
                zDKWinPixels.data()
            )) &&
            SUCCEEDED(zDKWinFrame->Commit()) && SUCCEEDED(zDKWinEncoder->Commit());
    }
    zDKWinDeviceContext->Unmap(zDKWinStaging.Get(), 0);
    return zDKWinSucceeded;
}

void zDKUIConfigureStyle(float zDKWinScale)
{
    ImGui::GetStyle() = ImGuiStyle();
    ImGui::StyleColorsDark();
    auto& zDKWinStyleData = ImGui::GetStyle();
    zDKWinStyleData.WindowRounding = 0;
    zDKWinStyleData.ChildRounding = 12;
    zDKWinStyleData.FrameRounding = 7;
    zDKWinStyleData.PopupRounding = 10;
    zDKWinStyleData.GrabRounding = 8;
    zDKWinStyleData.ScrollbarRounding = 8;
    zDKWinStyleData.WindowPadding = {20, 20};
    zDKWinStyleData.FramePadding = {22, 16};
    zDKWinStyleData.ItemSpacing = {10, 10};
    zDKWinStyleData.ItemInnerSpacing = {8, 7};
    zDKWinStyleData.ChildBorderSize = 0;
    zDKWinStyleData.FrameBorderSize = 0;
    zDKWinStyleData.ScrollbarSize = 20;
    zDKWinStyleData.GrabMinSize = 34;
    zDKWinStyleData.Colors[ImGuiCol_Text] = zDKUIWhite;
    zDKWinStyleData.Colors[ImGuiCol_TextDisabled] = zDKUIMuted;
    zDKWinStyleData.Colors[ImGuiCol_Border] = {.30f, .12f, .24f, .7f};
    zDKWinStyleData.Colors[ImGuiCol_FrameBg] = {.085f, .054f, .09f, 1};
    zDKWinStyleData.Colors[ImGuiCol_FrameBgHovered] = {.17f, .065f, .13f, 1};
    zDKWinStyleData.Colors[ImGuiCol_FrameBgActive] = {.23f, .07f, .16f, 1};
    zDKWinStyleData.Colors[ImGuiCol_CheckMark] = zDKUIPink;
    zDKWinStyleData.Colors[ImGuiCol_SliderGrab] = zDKUIPink;
    zDKWinStyleData.Colors[ImGuiCol_ButtonHovered] = {.28f, .065f, .17f, 1};
    zDKWinStyleData.Colors[ImGuiCol_ButtonActive] = {.4f, .09f, .24f, 1};
    zDKWinStyleData.Colors[ImGuiCol_Button] = {.16f, .055f, .12f, 1};
    zDKWinStyleData.Colors[ImGuiCol_ScrollbarBg] = {0, 0, 0, 0};
    zDKWinStyleData.Colors[ImGuiCol_ScrollbarGrab] = {0, 0, 0, 0};
    zDKWinStyleData.Colors[ImGuiCol_ScrollbarGrabHovered] = {0, 0, 0, 0};
    zDKWinStyleData.Colors[ImGuiCol_ScrollbarGrabActive] = {0, 0, 0, 0};
    zDKWinStyleData.Colors[ImGuiCol_Header] = {.16f, .055f, .12f, 1};
    zDKWinStyleData.Colors[ImGuiCol_HeaderHovered] = {.26f, .07f, .17f, 1};
    zDKWinStyleData.Colors[ImGuiCol_HeaderActive] = {.34f, .08f, .2f, 1};
    zDKWinStyleData.Colors[ImGuiCol_Tab] = {.08f, .04f, .075f, 1};
    zDKWinStyleData.Colors[ImGuiCol_TabSelected] = {.24f, .065f, .16f, 1};
    zDKWinStyleData.Colors[ImGuiCol_TabHovered] = {.3f, .075f, .19f, 1};
    zDKWinStyleData.Colors[ImGuiCol_TabSelectedOverline] = zDKUIPink;
    zDKWinStyleData.Colors[ImGuiCol_Separator] = {.31f, .12f, .24f, .6f};
    zDKWinStyleData.Colors[ImGuiCol_ResizeGrip] = zDKUIPink;
    zDKWinStyleData.Colors[ImGuiCol_NavCursor] = zDKUIPink;
    zDKWinStyleData.Colors[ImGuiCol_PopupBg] = {.045f, .025f, .05f, .99f};
    zDKWinStyleData.ScaleAllSizes(zDKWinScale);
}

void zDKUIApplyDisplayScale(float zDKWinScale, bool zDKWinBackendReady)
{
    zDKUIDisplayScale = std::clamp(zDKWinScale, .25f, 8.f);
    zDKWinFontScale = zDKUIDisplayScale;
    if (zDKWinBackendReady)
        ImGui_ImplDX11_InvalidateDeviceObjects();
    auto& zDKWinInputOutput = ImGui::GetIO();
    zDKWinInputOutput.FontGlobalScale = 1;
    zDKWinInputOutput.Fonts->Clear();
    static const ImWchar zDKWinGlyphs[] = {0x20, 0xFF, 0x2000, 0x206F, 0x2190, 0x2194, 0};
    zDKUIBodyFont = zDKWinInputOutput.Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/segoeui.ttf", 20 * zDKUIDisplayScale, nullptr, zDKWinGlyphs
    );
    const char* zDKWinDisplayFont = std::filesystem::exists("C:/Windows/Fonts/bahnschrift.ttf")
        ? "C:/Windows/Fonts/bahnschrift.ttf"
        : "C:/Windows/Fonts/seguisb.ttf";
    zDKUIHeadingFont = zDKWinInputOutput.Fonts->AddFontFromFileTTF(
        zDKWinDisplayFont, 23 * zDKUIDisplayScale, nullptr, zDKWinGlyphs
    );
    zDKUITitleFont = zDKWinInputOutput.Fonts->AddFontFromFileTTF(
        zDKWinDisplayFont, 32 * zDKUIDisplayScale, nullptr, zDKWinGlyphs
    );
    zDKUIMetricFont = zDKWinInputOutput.Fonts->AddFontFromFileTTF(
        zDKWinDisplayFont, 38 * zDKUIDisplayScale, nullptr, zDKWinGlyphs
    );
    zDKUICaptionFont = zDKWinInputOutput.Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/segoeui.ttf", 15 * zDKUIDisplayScale, nullptr, zDKWinGlyphs
    );
    zDKUILabelFont = zDKWinInputOutput.Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/seguisb.ttf", 20 * zDKUIDisplayScale, nullptr, zDKWinGlyphs
    );
    if (!zDKUIBodyFont)
        zDKUIBodyFont = zDKWinInputOutput.Fonts->AddFontDefault();
    if (!zDKUIHeadingFont)
        zDKUIHeadingFont = zDKUIBodyFont;
    if (!zDKUITitleFont)
        zDKUITitleFont = zDKUIHeadingFont;
    if (!zDKUIMetricFont)
        zDKUIMetricFont = zDKUIBodyFont;
    if (!zDKUICaptionFont)
        zDKUICaptionFont = zDKUIBodyFont;
    if (!zDKUILabelFont)
        zDKUILabelFont = zDKUIBodyFont;
    zDKWinInputOutput.FontDefault = zDKUIBodyFont;
    zDKWinInputOutput.Fonts->Build();
    zDKUIConfigureStyle(zDKUIDisplayScale);
    if (zDKWinBackendReady)
        ImGui_ImplDX11_CreateDeviceObjects();
}

void zDKWinLoadLegacySession()
{
    std::ifstream zDKWinInputStream(zDKUIDataDirectory() / L"session.dokkan");
    std::string zDKWinMagic, zDKWinKey;
    int zDKWinVersion;
    double zDKWinValue;
    if (!(zDKWinInputStream >> zDKWinMagic >> zDKWinVersion) || zDKWinMagic != "DOKKAN_DEFENSE_LAB" ||
        zDKWinVersion != 1)
        return;
    zDK::zDKCalcUnitStats zDKWinLegacyUnit;
    zDK::zDKCalcEnemyStats zDKWinLegacyBoss;
    bool zDKWinAfter = true;
    while (zDKWinInputStream >> zDKWinKey >> zDKWinValue)
    {
        if (!std::isfinite(zDKWinValue))
            return;
        if ((zDKWinKey == "unit.type" || zDKWinKey == "enemy.type") &&
            (zDKWinValue < 0 || zDKWinValue > 4 || zDKWinValue != std::floor(zDKWinValue)))
            return;
        if ((zDKWinKey == "unit.cls" || zDKWinKey == "enemy.cls") && (zDKWinValue != 0 && zDKWinValue != 1))
            return;
        if (zDKWinKey == "unit.base")
        {
            zDKWinLegacyUnit.zDKCalcBaseDefense =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcBaseDefense)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.leader")
        {
            zDKWinLegacyUnit.zDKCalcLeaderSkillPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcLeaderSkillPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.phase1")
        {
            zDKWinLegacyUnit.zDKCalcStartOfTurnPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcStartOfTurnPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.domain")
        {
            zDKWinLegacyUnit.zDKCalcDomainPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcDomainPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.item")
        {
            zDKWinLegacyUnit.zDKCalcItemPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcItemPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.links")
        {
            zDKWinLegacyUnit.zDKCalcLinkSkillPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcLinkSkillPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.active")
        {
            zDKWinLegacyUnit.zDKCalcActiveSkillPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcActiveSkillPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.phase2")
        {
            zDKWinLegacyUnit.zDKCalcOnAttackPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcOnAttackPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.stacks")
        {
            zDKWinLegacyUnit.zDKCalcSuperAttackStackCount =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcSuperAttackStackCount)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.stackBonus")
        {
            zDKWinLegacyUnit.zDKCalcStackDefensePercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcStackDefensePercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.supers")
        {
            zDKWinLegacyUnit.zDKCalcCurrentSuperAttackCount =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcCurrentSuperAttackCount)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.superBonus")
        {
            zDKWinLegacyUnit.zDKCalcSuperAttackDefensePercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcSuperAttackDefensePercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.otherSA")
        {
            zDKWinLegacyUnit.zDKCalcOtherSuperAttackPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcOtherSuperAttackPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.direct")
        {
            zDKWinLegacyUnit.zDKCalcUseManualDefense =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcUseManualDefense)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.directDefense")
        {
            zDKWinLegacyUnit.zDKCalcManualDefense =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcManualDefense)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.truncate")
        {
            zDKWinLegacyUnit.zDKCalcRoundDownDefense =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcRoundDownDefense)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.guard")
        {
            zDKWinLegacyUnit.zDKCalcGuardsAllAttacks =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcGuardsAllAttacks)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.dr")
        {
            zDKWinLegacyUnit.zDKCalcDamageReductionPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcDamageReductionPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.drNormal")
        {
            zDKWinLegacyUnit.zDKCalcNormalDamageReductionPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcNormalDamageReductionPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.drSuper")
        {
            zDKWinLegacyUnit.zDKCalcSuperDamageReductionPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcSuperDamageReductionPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.tdb")
        {
            zDKWinLegacyUnit.zDKCalcTypeDefenseBoostLevel =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcTypeDefenseBoostLevel)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.defenseDown")
        {
            zDKWinLegacyUnit.zDKCalcDefenseDebuffPercent =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcDefenseDebuffPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.hp")
        {
            zDKWinLegacyUnit.zDKCalcTeamHealth =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcTeamHealth)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.type")
        {
            zDKWinLegacyUnit.zDKCalcElementType =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcElementType)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.cls")
        {
            zDKWinLegacyUnit.zDKCalcCombatClass =
                static_cast<decltype(zDKWinLegacyUnit.zDKCalcCombatClass)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "unit.afterAttack")
        {
            zDKWinAfter = zDKWinValue != 0;
            continue;
        }
        if (zDKWinKey == "enemy.attack")
        {
            zDKWinLegacyBoss.zDKCalcNormalAttack =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcNormalAttack)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.superMultiplier")
        {
            zDKWinLegacyBoss.zDKCalcSuperAttackMultiplier =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcSuperAttackMultiplier)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.modifier")
        {
            zDKWinLegacyBoss.zDKCalcAttackModifierPercent =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcAttackModifierPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.type")
        {
            zDKWinLegacyBoss.zDKCalcElementType =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcElementType)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.cls")
        {
            zDKWinLegacyBoss.zDKCalcCombatClass =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcCombatClass)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.critical")
        {
            zDKWinLegacyBoss.zDKCalcIsCritical =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcIsCritical)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.customCriticalType")
        {
            zDKWinLegacyBoss.zDKCalcUseCustomCriticalType =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcUseCustomCriticalType)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.criticalMultiplier")
        {
            zDKWinLegacyBoss.zDKCalcCriticalMultiplier =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcCriticalMultiplier)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.ignoredDefense")
        {
            zDKWinLegacyBoss.zDKCalcCriticalDefenseIgnoredPercent =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcCriticalDefenseIgnoredPercent)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.criticalType")
        {
            zDKWinLegacyBoss.zDKCalcCriticalTypeMultiplier =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcCriticalTypeMultiplier)>(zDKWinValue);
            continue;
        };
        if (zDKWinKey == "enemy.criticalGuard")
        {
            zDKWinLegacyBoss.zDKCalcCriticalGuardMultiplier =
                static_cast<decltype(zDKWinLegacyBoss.zDKCalcCriticalGuardMultiplier)>(zDKWinValue);
            continue;
        };
    }
    try
    {
        zDK::zDKCalcCalculateBattle(zDKWinLegacyUnit, zDKWinLegacyBoss);
        zDK::zDKEffectUnitProfile zDKWinUnitProfileData;
        zDKWinUnitProfileData.zDKEffectBaseDefense = zDKWinLegacyUnit.zDKCalcBaseDefense;
        zDKWinUnitProfileData.zDKEffectLeaderSkillPercent = zDKWinLegacyUnit.zDKCalcLeaderSkillPercent;
        zDKWinUnitProfileData.zDKEffectTypeDefenseBoostLevel = zDKWinLegacyUnit.zDKCalcTypeDefenseBoostLevel;
        zDKWinUnitProfileData.zDKEffectUseManualDefense = zDKWinLegacyUnit.zDKCalcUseManualDefense;
        zDKWinUnitProfileData.zDKEffectManualDefense = zDKWinLegacyUnit.zDKCalcManualDefense;
        zDKWinUnitProfileData.zDKEffectRoundDownDefense = true;
        zDKWinUnitProfileData.zDKEffectElementType = zDKWinLegacyUnit.zDKCalcElementType;
        zDKWinUnitProfileData.zDKEffectCombatClass = zDKWinLegacyUnit.zDKCalcCombatClass;
        auto zDKWinAdd =
            [&](double zDKWinAmount,
                zDK::zDKEffectCategory zDKWinCategory,
                zDK::zDKEffectStat zDKWinStatisticIndex,
                zDK::zDKEffectDefensePhase zDKWinDefensePhaseIndex,
                int zDKWinCount = 1,
                zDK::zDKEffectAttackTiming zDKWinTiming = zDK::zDKEffectAttackTiming::zDKEffectAlways)
        {
            if (zDKWinAmount == 0 && zDKWinCategory != zDK::zDKEffectCategory::zDKEffectGuard)
                return;
            auto zDKWinEffectData = zDK::zDKEffectCreate(zDKWinCategory);
            zDKWinEffectData.zDKEffectValue = zDKWinAmount;
            if (zDKWinCategory != zDK::zDKEffectCategory::zDKEffectGuard)
                zDKWinEffectData.zDKEffectStat = zDKWinStatisticIndex;
            zDKWinEffectData.zDKEffectGroup = zDKWinDefensePhaseIndex;
            zDKWinEffectData.zDKEffectCount = zDKWinCount;
            zDKWinEffectData.zDKEffectTiming = zDKWinTiming;
            zDKWinUnitProfileData.zDKEffectEffects.push_back(zDKWinEffectData);
        };
        using namespace zDK;
        const double zDKWinValues[] = {
            zDKWinLegacyUnit.zDKCalcStartOfTurnPercent,
            zDKWinLegacyUnit.zDKCalcDomainPercent,
            zDKWinLegacyUnit.zDKCalcItemPercent,
            zDKWinLegacyUnit.zDKCalcLinkSkillPercent,
            zDKWinLegacyUnit.zDKCalcActiveSkillPercent,
            zDKWinLegacyUnit.zDKCalcOnAttackPercent,
            zDKWinLegacyUnit.zDKCalcOtherSuperAttackPercent
        };
        for (int zDKWinIndex = 0; zDKWinIndex < 7; ++zDKWinIndex)
            zDKWinAdd(
                zDKWinValues[zDKWinIndex],
                zDKWinIndex == 4 ? zDKEffectCategory::zDKEffectActive : zDKEffectCategory::zDKEffectBasic,
                zDKEffectStat::zDKEffectDefense,
                zDKEffectDefensePhase(zDKWinIndex)
            );
        if (zDKWinLegacyUnit.zDKCalcSuperAttackStackCount > 0)
            zDKWinAdd(
                zDKWinLegacyUnit.zDKCalcStackDefensePercent,
                zDKEffectCategory::zDKEffectSuperAttackStack,
                zDKEffectStat::zDKEffectDefense,
                zDKEffectDefensePhase::zDKEffectSuperAttack,
                int(zDKWinLegacyUnit.zDKCalcSuperAttackStackCount)
            );
        if (zDKWinLegacyUnit.zDKCalcCurrentSuperAttackCount > 0)
            zDKWinAdd(
                zDKWinLegacyUnit.zDKCalcSuperAttackDefensePercent,
                zDKEffectCategory::zDKEffectSuperAttackStack,
                zDKEffectStat::zDKEffectDefense,
                zDKEffectDefensePhase::zDKEffectSuperAttack,
                int(zDKWinLegacyUnit.zDKCalcCurrentSuperAttackCount),
                zDKEffectAttackTiming::zDKEffectAfterAttack
            );
        zDKWinAdd(
            zDKWinLegacyUnit.zDKCalcDamageReductionPercent,
            zDKEffectCategory::zDKEffectBasic,
            zDKEffectStat::zDKEffectDamageReduction,
            zDKEffectDefensePhase::zDKEffectStartOfTurn
        );
        zDKWinAdd(
            zDKWinLegacyUnit.zDKCalcNormalDamageReductionPercent,
            zDKEffectCategory::zDKEffectBasic,
            zDKEffectStat::zDKEffectNormalDamageReduction,
            zDKEffectDefensePhase::zDKEffectStartOfTurn
        );
        zDKWinAdd(
            zDKWinLegacyUnit.zDKCalcSuperDamageReductionPercent,
            zDKEffectCategory::zDKEffectBasic,
            zDKEffectStat::zDKEffectSuperDamageReduction,
            zDKEffectDefensePhase::zDKEffectStartOfTurn
        );
        zDKWinAdd(
            zDKWinLegacyUnit.zDKCalcDefenseDebuffPercent,
            zDKEffectCategory::zDKEffectBasic,
            zDKEffectStat::zDKEffectDefenseDebuff,
            zDKEffectDefensePhase::zDKEffectStartOfTurn
        );
        if (zDKWinLegacyUnit.zDKCalcGuardsAllAttacks)
            zDKWinAdd(
                0,
                zDKEffectCategory::zDKEffectGuard,
                zDKEffectStat::zDKEffectDefense,
                zDKEffectDefensePhase::zDKEffectStartOfTurn
            );
        zDKEffectBossProfile zDKWinBossProfileData;
        zDKWinBossProfileData.zDKEffectStats = zDKWinLegacyBoss;
        zDKEffectBattleState zDKWinState;
        zDKWinState.zDKEffectCurrentHP = zDKWinLegacyUnit.zDKCalcTeamHealth;
        zDKWinState.zDKEffectMaxHP = std::max(1.0, zDKWinLegacyUnit.zDKCalcTeamHealth);
        zDKWinState.zDKEffectAfterUnitAttack = zDKWinAfter;
        zDKEffectEvaluate(zDKWinUnitProfileData, zDKWinBossProfileData, zDKWinState);
        zDKUICurrentUnit = std::move(zDKWinUnitProfileData);
        zDKUICurrentBoss = zDKWinBossProfileData;
        zDKUICurrentBattle = zDKWinState;
    }
    catch (...)
    {
    }
}

bool zDKUIRestoreSavedSession()
{
    for (const auto& zDKWinPath :
         {zDKUISessionPath(),
          zDKUIDataDirectory() / L"session-v5.dokkan",
          zDKUIDataDirectory() / L"session-v4.dokkan",
          zDKUIDataDirectory() / L"session-v3.dokkan",
          zDKUIDataDirectory() / L"session-v2.dokkan"})
    {
        std::ifstream zDKWinInputStream(zDKWinPath);
        if (zDKWinInputStream &&
            zDKStoreLoadSession(
                zDKWinInputStream,
                zDKUICurrentUnit,
                zDKUICurrentBoss,
                zDKUICurrentBattle,
                zDKUIAnimationsEnabled,
                zDKUISelectedAttackKind
            ))
        {
            zDKUISelectedPreset = -1;
            return true;
        }
    }
    return false;
}

SIZE zDKWinCalculatePortraitSize(float zDKWinScale, const RECT& zDKWinWork)
{
    RECT zDKWinFrame{};
    AdjustWindowRectExForDpi(
        &zDKWinFrame,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
        FALSE,
        0,
        UINT(std::lround(zDKWinScale * 96))
    );
    const LONG zDKWinFrameWidth = zDKWinFrame.right - zDKWinFrame.left;
    const LONG zDKWinFrameHeight = zDKWinFrame.bottom - zDKWinFrame.top;
    float zDKWinFit = std::min(
        {1.f,
         float(zDKWinWork.right - zDKWinWork.left - 24 - zDKWinFrameWidth) / (540 * zDKWinScale),
         float(zDKWinWork.bottom - zDKWinWork.top - 24 - zDKWinFrameHeight) / (980 * zDKWinScale)}
    );
    const LONG zDKWinClientWidth = LONG(std::lround(540 * zDKWinScale * zDKWinFit));
    return {
        zDKWinClientWidth + zDKWinFrameWidth,
        LONG(std::lround(zDKWinClientWidth * 980.0 / 540)) + zDKWinFrameHeight
    };
}

SIZE zDKWinFrameSize(HWND zDKWinWindow, UINT zDKWinDpi)
{
    RECT zDKWinFrame{};
    AdjustWindowRectExForDpi(
        &zDKWinFrame,
        DWORD(GetWindowLongPtrW(zDKWinWindow, GWL_STYLE)),
        FALSE,
        DWORD(GetWindowLongPtrW(zDKWinWindow, GWL_EXSTYLE)),
        zDKWinDpi ? zDKWinDpi : GetDpiForWindow(zDKWinWindow)
    );
    return {zDKWinFrame.right - zDKWinFrame.left, zDKWinFrame.bottom - zDKWinFrame.top};
}

SIZE zDKWinPortraitLimit(HWND zDKWinWindow, bool zDKWinMinimum)
{
    MONITORINFO zDKWinMonitor{sizeof(zDKWinMonitor)};
    GetMonitorInfoW(MonitorFromWindow(zDKWinWindow, MONITOR_DEFAULTTONEAREST), &zDKWinMonitor);
    const auto zDKWinFrame = zDKWinFrameSize(zDKWinWindow);
    const LONG zDKWinMaximumWidth = std::max(
        1L,
        std::min(
            {4320L,
             zDKWinMonitor.rcWork.right - zDKWinMonitor.rcWork.left - zDKWinFrame.cx - 24,
             LONG(
                 std::floor(
                     (zDKWinMonitor.rcWork.bottom - zDKWinMonitor.rcWork.top - zDKWinFrame.cy - 24) * 540.0 /
                     980
                 )
             )}
        )
    );
    const LONG zDKWinWidth = zDKWinMinimum ? std::min(360L, zDKWinMaximumWidth) : zDKWinMaximumWidth;
    return {zDKWinWidth + zDKWinFrame.cx, LONG(std::lround(zDKWinWidth * 980.0 / 540)) + zDKWinFrame.cy};
}

void zDKWinConstrainResize(HWND zDKWinWindow, WPARAM zDKWinEdge, RECT& zDKWinBounds)
{
    const auto zDKWinFrame = zDKWinFrameSize(zDKWinWindow);
    const auto zDKWinMinimum = zDKWinPortraitLimit(zDKWinWindow, true);
    const auto zDKWinMaximum = zDKWinPortraitLimit(zDKWinWindow, false);
    double zDKWinClientWidth = zDKWinBounds.right - zDKWinBounds.left - zDKWinFrame.cx;
    const double zDKWinClientHeight = zDKWinBounds.bottom - zDKWinBounds.top - zDKWinFrame.cy;
    bool zDKWinUseHeight = zDKWinEdge == WMSZ_TOP || zDKWinEdge == WMSZ_BOTTOM;
    if (zDKWinEdge != WMSZ_LEFT && zDKWinEdge != WMSZ_RIGHT && !zDKWinUseHeight)
    {
        RECT zDKWinStart = zDKWinResizeStart;
        if (zDKWinStart.right <= zDKWinStart.left)
            GetWindowRect(zDKWinWindow, &zDKWinStart);
        zDKWinUseHeight =
            std::abs((zDKWinBounds.bottom - zDKWinBounds.top) - (zDKWinStart.bottom - zDKWinStart.top)) /
                980.0 >
            std::abs((zDKWinBounds.right - zDKWinBounds.left) - (zDKWinStart.right - zDKWinStart.left)) /
                540.0;
    }
    if (zDKWinUseHeight)
        zDKWinClientWidth = zDKWinClientHeight * 540 / 980;
    const LONG zDKWinWidth = std::clamp(
                                 LONG(std::lround(zDKWinClientWidth)),
                                 zDKWinMinimum.cx - zDKWinFrame.cx,
                                 zDKWinMaximum.cx - zDKWinFrame.cx
                             ) +
        zDKWinFrame.cx;
    const LONG zDKWinHeight =
        LONG(std::lround((zDKWinWidth - zDKWinFrame.cx) * 980.0 / 540)) + zDKWinFrame.cy;
    if (zDKWinEdge == WMSZ_TOP || zDKWinEdge == WMSZ_BOTTOM)
        zDKWinBounds.left += ((zDKWinBounds.right - zDKWinBounds.left) - zDKWinWidth) / 2;
    else if (zDKWinEdge == WMSZ_LEFT || zDKWinEdge == WMSZ_TOPLEFT || zDKWinEdge == WMSZ_BOTTOMLEFT)
        zDKWinBounds.left = zDKWinBounds.right - zDKWinWidth;
    zDKWinBounds.right = zDKWinBounds.left + zDKWinWidth;
    if (zDKWinEdge == WMSZ_LEFT || zDKWinEdge == WMSZ_RIGHT)
        zDKWinBounds.top += ((zDKWinBounds.bottom - zDKWinBounds.top) - zDKWinHeight) / 2;
    else if (zDKWinEdge == WMSZ_TOP || zDKWinEdge == WMSZ_TOPLEFT || zDKWinEdge == WMSZ_TOPRIGHT)
        zDKWinBounds.top = zDKWinBounds.bottom - zDKWinHeight;
    zDKWinBounds.bottom = zDKWinBounds.top + zDKWinHeight;
}
} // namespace zDKUI

using namespace zDKUI;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WINAPI zDKWinWindowProcedure(
    HWND zDKWinWindowHandle, UINT zDKWinMessage, WPARAM zDKWinWordParameter, LPARAM zDKWinLongParameter
)
{
    if (ImGui_ImplWin32_WndProcHandler(
            zDKWinWindowHandle, zDKWinMessage, zDKWinWordParameter, zDKWinLongParameter
        ))
        return true;
    switch (zDKWinMessage)
    {
        case WM_SIZE:
            if (zDKWinWordParameter != SIZE_MINIMIZED)
            {
                zDKWinPendingWidth = LOWORD(zDKWinLongParameter);
                zDKWinPendingHeight = HIWORD(zDKWinLongParameter);
                if (!zDKWinDiagnosticWindow)
                    zDKUIPendingDisplayScale =
                        std::min(zDKWinPendingWidth / 540.f, zDKWinPendingHeight / 980.f);
            }
            return 0;
        case WM_GETMINMAXINFO:
        {
            auto* zDKWinLimits = reinterpret_cast<MINMAXINFO*>(zDKWinLongParameter);
            if (zDKWinDiagnosticWindow)
            {
                zDKWinLimits->ptMinTrackSize = {1, 1};
                zDKWinLimits->ptMaxTrackSize = {32767, 32767};
                return 0;
            }
            const auto zDKWinMinimum = zDKWinPortraitLimit(zDKWinWindowHandle, true);
            const auto zDKWinMaximum = zDKWinPortraitLimit(zDKWinWindowHandle, false);
            zDKWinLimits->ptMinTrackSize = {zDKWinMinimum.cx, zDKWinMinimum.cy};
            zDKWinLimits->ptMaxTrackSize = {zDKWinMaximum.cx, zDKWinMaximum.cy};
            return 0;
        }
        case WM_WINDOWPOSCHANGING:
        {
            auto* zDKWinPos = reinterpret_cast<WINDOWPOS*>(zDKWinLongParameter);
            if (!zDKWinDiagnosticWindow && !zDKWinInSizeMove &&
                zDKWinApplicationWindow == zDKWinWindowHandle && !(zDKWinPos->flags & SWP_NOSIZE) &&
                !IsIconic(zDKWinWindowHandle) && zDKWinPos->cx > 0 && zDKWinPos->cy > 0)
            {
                RECT zDKWinCurrent{};
                GetWindowRect(zDKWinWindowHandle, &zDKWinCurrent);
                RECT zDKWinProposed{0, 0, zDKWinPos->cx, zDKWinPos->cy};
                zDKWinConstrainResize(
                    zDKWinWindowHandle,
                    zDKWinPos->cx != zDKWinCurrent.right - zDKWinCurrent.left ? WMSZ_RIGHT : WMSZ_BOTTOM,
                    zDKWinProposed
                );
                zDKWinPos->cx = zDKWinProposed.right - zDKWinProposed.left;
                zDKWinPos->cy = zDKWinProposed.bottom - zDKWinProposed.top;
            }
            break;
        }
        case WM_SYSCOMMAND:
            if ((zDKWinWordParameter & 0xfff0) == SC_KEYMENU || (zDKWinWordParameter & 0xfff0) == SC_MAXIMIZE)
                return 0;
            break;
        case WM_SIZING:
            zDKWinConstrainResize(
                zDKWinWindowHandle, zDKWinWordParameter, *reinterpret_cast<RECT*>(zDKWinLongParameter)
            );
            return TRUE;
        case WM_ENTERSIZEMOVE:
            zDKWinInSizeMove = true;
            GetWindowRect(zDKWinWindowHandle, &zDKWinResizeStart);
            SetTimer(zDKWinWindowHandle, 1, 16, nullptr);
            return 0;
        case WM_EXITSIZEMOVE:
        {
            KillTimer(zDKWinWindowHandle, 1);
            zDKWinInSizeMove = false;
            RECT zDKWinClient{};
            GetClientRect(zDKWinWindowHandle, &zDKWinClient);
            if (!zDKWinDiagnosticWindow)
                zDKUIPendingDisplayScale = std::min(zDKWinClient.right / 540.f, zDKWinClient.bottom / 980.f);
            return 0;
        }
        case WM_TIMER:
            if (zDKWinWordParameter == 1 && zDKWinInSizeMove && zDKWinRenderDuringResize && !zDKWinRendering)
                zDKWinRenderDuringResize();
            return 0;
        case WM_DPICHANGED:
        {
            zDKWinUpdateWindowIcons(zDKWinWindowHandle, HIWORD(zDKWinWordParameter));
            const float zDKWinDpiScale = float(HIWORD(zDKWinWordParameter)) / zDKWinWindowDpi;
            zDKWinWindowDpi = HIWORD(zDKWinWordParameter);
            auto* zDKWinBoundsData = reinterpret_cast<RECT*>(zDKWinLongParameter);
            MONITORINFO zDKWinMonitor{sizeof(zDKWinMonitor)};
            GetMonitorInfoW(MonitorFromRect(zDKWinBoundsData, MONITOR_DEFAULTTONEAREST), &zDKWinMonitor);
            if (!zDKWinDiagnosticWindow)
            {
                RECT zDKWinClient{};
                GetClientRect(zDKWinWindowHandle, &zDKWinClient);
                const auto zDKWinFrame = zDKWinFrameSize(zDKWinWindowHandle, HIWORD(zDKWinWordParameter));
                const LONG zDKWinWidth = std::min(
                    LONG(std::lround(zDKWinClient.right * zDKWinDpiScale)),
                    LONG(
                        std::floor(
                            (zDKWinMonitor.rcWork.bottom - zDKWinMonitor.rcWork.top - zDKWinFrame.cy - 24) *
                            540.0 / 980
                        )
                    )
                );
                zDKWinInitialWindowSize = {
                    zDKWinWidth + zDKWinFrame.cx,
                    LONG(std::lround(zDKWinWidth * 980.0 / 540)) + zDKWinFrame.cy
                };
            }
            SetWindowPos(
                zDKWinWindowHandle,
                nullptr,
                zDKWinBoundsData->left,
                zDKWinBoundsData->top,
                zDKWinInitialWindowSize.cx,
                zDKWinInitialWindowSize.cy,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
            return 0;
        }
        case WM_DESTROY:
            if (zDKWinLargeIcon)
                DestroyIcon(zDKWinLargeIcon);
            if (zDKWinSmallIcon)
                DestroyIcon(zDKWinSmallIcon);
            zDKWinLargeIcon = zDKWinSmallIcon = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(zDKWinWindowHandle, zDKWinMessage, zDKWinWordParameter, zDKWinLongParameter);
}

int zDKWinRunApplication(HINSTANCE zDKWinInstance)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    ImGui_ImplWin32_EnableDpiAwareness();
    std::filesystem::path zDKWinCapturePath, zDKWinUiCheckPath;
    bool zDKWinCaptureExample = false, zDKWinSupportExample = false, zDKWinPreviewTopmost = false,
         zDKWinPreviewSettings = false, zDKWinPreviewUpdates = false, zDKWinTurnExample = false,
         zDKWinPreviewTurnPlan = false;
    int zDKWinForcedWidth = 0, zDKWinForcedHeight = 0, zDKWinPreviewEffect = -1, zDKWinCaptureAt = 40,
        zDKWinInitialSection = 0;
    bool zDKWinShowcase = false, zDKWinPreviewSwitch = false, zDKWinPreviewPicker = false;
    float zDKWinTransitionScale = 0;
    int zDKWinArgumentCount = 0;
    LPWSTR* zDKWinArguments = CommandLineToArgvW(GetCommandLineW(), &zDKWinArgumentCount);
    for (int zDKWinIndex = 1; zDKWinIndex < zDKWinArgumentCount; ++zDKWinIndex)
    {
        std::wstring zDKWinArgument = zDKWinArguments[zDKWinIndex];
        if (zDKWinArgument == L"--capture" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKWinCapturePath = zDKWinArguments[++zDKWinIndex];
        else if (zDKWinArgument == L"--ui-smoke" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKWinUiCheckPath = zDKWinArguments[++zDKWinIndex];
        else if (zDKWinArgument == L"--preview-effect" && zDKWinIndex + 1 < zDKWinArgumentCount)
        {
            std::wstring zDKWinEffect = zDKWinArguments[++zDKWinIndex];
            zDKWinPreviewEffect = zDKWinEffect == L"active" ? 1
                : zDKWinEffect == L"entrance"               ? 2
                : zDKWinEffect == L"sa"                     ? 3
                : zDKWinEffect == L"guard"                  ? 4
                : zDKWinEffect == L"support"                ? 5
                : zDKWinEffect == L"attacking"              ? 6
                : zDKWinEffect == L"receiving"              ? 7
                : zDKWinEffect == L"ultra"                  ? 8
                                                            : 0;
        }
        else if (zDKWinArgument == L"--preview-switch")
            zDKWinPreviewSwitch = true;
        else if (zDKWinArgument == L"--preview-picker")
            zDKWinPreviewPicker = true;
        else if (zDKWinArgument == L"--preview-topmost")
            zDKWinPreviewTopmost = true;
        else if (zDKWinArgument == L"--preview-settings")
            zDKWinPreviewSettings = true;
        else if (zDKWinArgument == L"--preview-updates")
            zDKWinPreviewUpdates = true;
        else if (zDKWinArgument == L"--turn-example")
            zDKWinTurnExample = true;
        else if (zDKWinArgument == L"--preview-plan")
            zDKWinPreviewTurnPlan = true;
        else if (zDKWinArgument == L"--capture-frame" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKWinCaptureAt = std::clamp(_wtoi(zDKWinArguments[++zDKWinIndex]), 4, 240);
        else if (zDKWinArgument == L"--example")
            zDKWinCaptureExample = true;
        else if (zDKWinArgument == L"--support-example")
            zDKWinSupportExample = true;
        else if (zDKWinArgument == L"--showcase")
            zDKWinShowcase = true;
        else if (zDKWinArgument == L"--tab" && zDKWinIndex + 1 < zDKWinArgumentCount)
        {
            std::wstring zDKWinPage = zDKWinArguments[++zDKWinIndex];
            zDKUIActiveTab = zDKWinPage == L"boss" ? 1 : zDKWinPage == L"info" ? 2 : 0;
        }
        else if (zDKWinArgument == L"--section" && zDKWinIndex + 1 < zDKWinArgumentCount)
        {
            std::wstring zDKWinSection = zDKWinArguments[++zDKWinIndex];
            zDKWinInitialSection = (zDKWinSection == L"effects" || zDKWinSection == L"tanking")         ? 1
                : (zDKWinSection == L"setup" || zDKWinSection == L"skills" || zDKWinSection == L"turn") ? 2
                : zDKWinSection == L"calcs"                                                             ? 3
                                                                                                        : 0;
        }
        else if (zDKWinArgument == L"--width" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKWinForcedWidth = _wtoi(zDKWinArguments[++zDKWinIndex]);
        else if (zDKWinArgument == L"--height" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKWinForcedHeight = _wtoi(zDKWinArguments[++zDKWinIndex]);
        else if (zDKWinArgument == L"--scale" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKUIScaleOverride = float(_wtof(zDKWinArguments[++zDKWinIndex]));
        else if (zDKWinArgument == L"--transition-scale" && zDKWinIndex + 1 < zDKWinArgumentCount)
            zDKWinTransitionScale = std::clamp(float(_wtof(zDKWinArguments[++zDKWinIndex])), .75f, 4.f);
    }
    LocalFree(zDKWinArguments);
    zDKUISelectedSubviews[zDKUIActiveTab] = std::clamp(zDKWinInitialSection, 0, zDKUIActiveTab == 2 ? 3 : 2);
    bool zDKWinDiagnostic = !zDKWinCapturePath.empty() || !zDKWinUiCheckPath.empty();
    zDKWinDiagnosticWindow = zDKWinDiagnostic;
    zDKTestInteractionChecks zDKWinSmoke;
    zDKUICollectInteractionProbes = !zDKWinUiCheckPath.empty();
    WNDCLASSEXW zDKWinWindowClass{
        sizeof(zDKWinWindowClass),
        CS_CLASSDC,
        zDKWinWindowProcedure,
        0,
        0,
        zDKWinInstance,
        LoadIconW(zDKWinInstance, MAKEINTRESOURCEW(1)),
        LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        L"DokkanDefenseLab",
        LoadIconW(zDKWinInstance, MAKEINTRESOURCEW(1))
    };
    RegisterClassExW(&zDKWinWindowClass);
    RECT zDKWinWork{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &zDKWinWork, 0);
    zDKUIDisplayScale = zDKUIScaleOverride > 0
        ? std::clamp(zDKUIScaleOverride, .75f, 4.f)
        : ImGui_ImplWin32_GetDpiScaleForMonitor(
              MonitorFromPoint({zDKWinWork.left, zDKWinWork.top}, MONITOR_DEFAULTTOPRIMARY)
          );
    zDKWinInitialWindowSize = zDKWinCalculatePortraitSize(zDKUIDisplayScale, zDKWinWork);
    if (zDKWinDiagnostic)
    {
        if (zDKWinForcedWidth >= 360 && zDKWinForcedWidth <= 5120)
            zDKWinInitialWindowSize.cx = zDKWinForcedWidth;
        if (zDKWinForcedHeight >= 620 && zDKWinForcedHeight <= 3000)
            zDKWinInitialWindowSize.cy = zDKWinForcedHeight;
        if (zDKWinForcedWidth || zDKWinForcedHeight)
        {
            RECT zDKWinFrame{};
            AdjustWindowRectExForDpi(
                &zDKWinFrame,
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
                FALSE,
                0,
                UINT(
                    std::lround(
                        ImGui_ImplWin32_GetDpiScaleForMonitor(
                            MonitorFromPoint({zDKWinWork.left, zDKWinWork.top}, MONITOR_DEFAULTTOPRIMARY)
                        ) *
                        96
                    )
                )
            );
            if (zDKWinForcedWidth)
                zDKWinInitialWindowSize.cx += zDKWinFrame.right - zDKWinFrame.left;
            if (zDKWinForcedHeight)
                zDKWinInitialWindowSize.cy += zDKWinFrame.bottom - zDKWinFrame.top;
        }
    }
    int zDKWinWidth = zDKWinInitialWindowSize.cx, zDKWinHeight = zDKWinInitialWindowSize.cy;
    HWND zDKWinWindowHandle = CreateWindowW(
        zDKWinWindowClass.lpszClassName,
        L"Dokkan Defense Calculator",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
        zDKWinWork.left + (zDKWinWork.right - zDKWinWork.left - zDKWinWidth) / 2,
        zDKWinWork.top + (zDKWinWork.bottom - zDKWinWork.top - zDKWinHeight) / 2,
        zDKWinWidth,
        zDKWinHeight,
        nullptr,
        nullptr,
        zDKWinInstance,
        nullptr
    );
    zDKWinApplicationWindow = zDKWinWindowHandle;
    if (!zDKWinDiagnostic)
        zDKUIRestoreWindowSettings(zDKWinWindowHandle);
    else if (zDKWinPreviewTopmost)
        zDKUISetAlwaysOnTop(zDKWinWindowHandle, true);
    zDKWinWindowDpi = GetDpiForWindow(zDKWinWindowHandle);
    zDKWinUpdateWindowIcons(zDKWinWindowHandle, GetDpiForWindow(zDKWinWindowHandle));
    if (zDKUIScaleOverride <= 0)
    {
        RECT zDKWinClient{};
        GetClientRect(zDKWinWindowHandle, &zDKWinClient);
        zDKUIDisplayScale = std::min(zDKWinClient.right / 540.f, zDKWinClient.bottom / 980.f);
    }
    BOOL zDKWinDark = TRUE;
    DwmSetWindowAttribute(zDKWinWindowHandle, 20, &zDKWinDark, sizeof(zDKWinDark));
    COLORREF zDKWinBorder = RGB(109, 27, 68);
    DwmSetWindowAttribute(zDKWinWindowHandle, 34, &zDKWinBorder, sizeof(zDKWinBorder));
    if (!zDKWinInitializeDirect3D(zDKWinWindowHandle))
    {
        MessageBoxW(
            zDKWinWindowHandle,
            L"Unable to initialize Direct3D 11.",
            L"Dokkan Defense Calculator",
            MB_ICONERROR
        );
        DestroyWindow(zDKWinWindowHandle);
        CoUninitialize();
        return 1;
    }
    if (!zDKWinLoadTexture(101, zDKUIBackgroundTexture))
    {
        MessageBoxW(
            zDKWinWindowHandle, L"Unable to load embedded assets.", L"Dokkan Defense Calculator", MB_ICONERROR
        );
        DestroyWindow(zDKWinWindowHandle);
        CoUninitialize();
        return 1;
    }
    for (int zDKWinKind = 0; zDKWinKind < int(zDKUISkinKind::zDKUICount); ++zDKWinKind)
        for (int zDKWinState = 0; zDKWinState < 3; ++zDKWinState)
            if (!zDKWinLoadTexture(
                    zDKUISkinDefinitions[zDKWinKind].zDKUIResource[zDKWinState],
                    zDKUISkinTextures[zDKWinKind][zDKWinState]
                ))
            {
                MessageBoxW(
                    zDKWinWindowHandle,
                    L"Unable to load the mobile UI skin.",
                    L"Dokkan Defense Calculator",
                    MB_ICONERROR
                );
                DestroyWindow(zDKWinWindowHandle);
                CoUninitialize();
                return 1;
            }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& zDKWinInputOutput = ImGui::GetIO();
    zDKWinInputOutput.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    zDKWinInputOutput.IniFilename = nullptr;
    zDKWinInputOutput.LogFilename = nullptr;
    if (zDKUICollectInteractionProbes)
        zDKWinInputOutput.ConfigInputTrickleEventQueue = false;
    zDKUIApplyDisplayScale(zDKUIDisplayScale, false);
    ImGui_ImplWin32_Init(zDKWinWindowHandle);
    ImGui_ImplDX11_Init(zDKWinDevice.Get(), zDKWinDeviceContext.Get());
    zDKUIRefreshPresets();
    if (zDKUICollectInteractionProbes)
    {
        zDKUICurrentUnit = zDKEffectCreateGuideUnitProfile();
        zDKUICurrentBattle.zDKEffectCurrentHP = zDKUICurrentBattle.zDKEffectMaxHP = 700000;
    }
    if (zDKWinCaptureExample)
    {
        zDKUICurrentUnit = zDKEffectUnitProfile{};
        zDKUICurrentUnit.zDKEffectUseManualDefense = true;
        zDKUICurrentUnit.zDKEffectManualDefense = 400000;
        zDKUICurrentUnit.zDKEffectTypeDefenseBoostLevel = 10;
        zDKUICurrentUnit.zDKEffectEffects.push_back(zDKEffectCreate(zDKEffectCategory::zDKEffectGuard));
        auto zDKWinEffectData = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
        zDKWinEffectData.zDKEffectStat = zDKEffectStat::zDKEffectDamageReduction;
        zDKWinEffectData.zDKEffectValue = 30;
        zDKUICurrentUnit.zDKEffectEffects.push_back(zDKWinEffectData);
        zDKUICurrentBoss = zDKEffectBossProfile{};
        zDKUICurrentBoss.zDKEffectStats.zDKCalcNormalAttack = 1000000;
        zDKUICurrentBoss.zDKEffectStats.zDKCalcSuperAttack = 3000000;
        zDKUICurrentBoss.zDKEffectStats.zDKCalcElementType = zDKCalcUnitType::zDKCalcSTR;
        zDKUICurrentBattle.zDKEffectCurrentHP = zDKUICurrentBattle.zDKEffectMaxHP = 700000;
        zDKUISelectedAttackKind = 0;
    }
    if (zDKWinSupportExample)
    {
        zDKUICurrentUnit = zDKEffectUnitProfile{};
        zDKUICurrentUnit.zDKEffectBaseDefense = 10000;
        zDKUICurrentUnit.zDKEffectLeaderSkillPercent = 440;
        const zDKEffectCategory zDKWinCategories[] = {
            zDKEffectCategory::zDKEffectUltraSuperAttack,
            zDKEffectCategory::zDKEffectWhenReceiving,
            zDKEffectCategory::zDKEffectBasic,
            zDKEffectCategory::zDKEffectWhenAttacking,
            zDKEffectCategory::zDKEffectEntrance,
            zDKEffectCategory::zDKEffectSupport,
            zDKEffectCategory::zDKEffectSuperAttackStack
        };
        const double zDKWinBoosts[] = {50, 50, 200, 100, 30, 40, 30};
        for (int zDKWinIndex = 0; zDKWinIndex < 7; ++zDKWinIndex)
        {
            auto zDKWinEntry = zDKEffectCreate(zDKWinCategories[zDKWinIndex]);
            zDKWinEntry.zDKEffectValue = zDKWinBoosts[zDKWinIndex];
            if (zDKWinEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectSupport)
                zDKWinEntry.zDKEffectName = "INT Broly";
            zDKUICurrentUnit.zDKEffectEffects.push_back(zDKWinEntry);
        }
    }
    if (zDKWinDiagnostic && zDKWinTurnExample)
    {
        zDKUICurrentUnit = zDKEffectCreateEmptyUnitProfile();
        zDKUICurrentUnit.zDKEffectName = "Mixed attack example";
        zDKUICurrentUnit.zDKEffectBaseDefense = 13169;
        zDKUICurrentUnit.zDKEffectLeaderSkillPercent = 440;
        for (const auto& zDKWinPhase : std::vector<std::pair<zDKEffectDefensePhase, double>>{
                 {zDKEffectDefensePhase::zDKEffectStartOfTurn, 300},
                 {zDKEffectDefensePhase::zDKEffectDomain, 15},
                 {zDKEffectDefensePhase::zDKEffectItem, 20},
                 {zDKEffectDefensePhase::zDKEffectLinks, 10}
             })
        {
            auto zDKWinEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
            zDKWinEffect.zDKEffectGroup = zDKWinPhase.first;
            zDKWinEffect.zDKEffectValue = zDKWinPhase.second;
            zDKUICurrentUnit.zDKEffectEffects.push_back(zDKWinEffect);
        }
        auto zDKWinBuild = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenAttacking);
        zDKWinBuild.zDKEffectValue = 50;
        zDKWinBuild.zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger::zDKTurnAnyAttack;
        zDKWinBuild.zDKEffectBuildUpLimit = 8;
        auto zDKWinSA = zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack);
        zDKWinSA.zDKEffectValue = 20;
        auto zDKWinUltra = zDKEffectCreate(zDKEffectCategory::zDKEffectUltraSuperAttack);
        zDKWinUltra.zDKEffectValue = 50;
        zDKUICurrentUnit.zDKEffectEffects.push_back(zDKWinBuild);
        zDKUICurrentUnit.zDKEffectEffects.push_back(zDKWinSA);
        zDKUICurrentUnit.zDKEffectEffects.push_back(zDKWinUltra);
        zDKUICurrentUnit.zDKEffectAttackPlan.zDKTurnAttacks = {
            zDKTurnAttackKind::zDKTurnUltra,
            zDKTurnAttackKind::zDKTurnNormal,
            zDKTurnAttackKind::zDKTurnSuper,
            zDKTurnAttackKind::zDKTurnSuper
        };
        zDKUIActiveTab = 2;
        zDKUISelectedSubviews[2] = 2;
    }
    if (zDKWinShowcase)
    {
        zDKUICurrentUnit.zDKEffectName = "Goku Black";
        zDKUICurrentBoss.zDKEffectName = "Enemy";
        auto zDKWinEffectData = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
        zDKWinEffectData.zDKEffectValue = 100;
        zDKWinEffectData.zDKEffectFirstTurn = 3;
        zDKWinEffectData.zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectBelow;
        zDKWinEffectData.zDKEffectHealthThresholdPercent = 50;
        zDKUICurrentBoss.zDKEffectEffects.push_back(zDKWinEffectData);
        zDKUIRequestedBossEffectIndex = 0;
        if (zDKUIActiveTab == 1)
            zDKUISelectedSubviews[1] = 1;
    }
    if (zDKWinPreviewEffect >= 0)
        zDKUIRequestEffect(
            zDKUIActiveTab == 1,
            -1,
            zDKEffectCategory(zDKUIActiveTab == 1 ? std::min(3, zDKWinPreviewEffect) : zDKWinPreviewEffect)
        );
    if (zDKWinPreviewPicker)
        zDKUIRequestNewEffect(zDKUIActiveTab == 1);
    std::string zDKWinPersisted;
    try
    {
        zDKWinPersisted = zDKStoreSaveSession(
            zDKUICurrentUnit,
            zDKUICurrentBoss,
            zDKUICurrentBattle,
            zDKUIAnimationsEnabled,
            zDKUISelectedAttackKind
        );
    }
    catch (...)
    {
    }
    std::string zDKWinObserved = zDKWinPersisted;
    double zDKWinDirtyAt = 0;
    if (!zDKWinDiagnostic)
        ShowWindow(zDKWinWindowHandle, SW_SHOWDEFAULT);
    UpdateWindow(zDKWinWindowHandle);
    bool zDKWinDone = false, zDKWinCaptureOK = zDKWinCapturePath.empty();
    int zDKWinFrames = 0;
    zDKWinRenderDuringResize = [&]
    {
        if (zDKWinRendering)
            return;
        zDKWinRendering = true;
        if (zDKUIPendingDisplayScale > 0)
        {
            float zDKWinScale = zDKUIScaleOverride > 0 ? zDKUIScaleOverride : zDKUIPendingDisplayScale;
            zDKUIPendingDisplayScale = 0;
            if (zDKWinInSizeMove)
            {
                zDKUIDisplayScale = std::clamp(zDKWinScale, .25f, 8.f);
                ImGui::GetIO().FontGlobalScale = zDKUIDisplayScale / zDKWinFontScale;
                zDKUIConfigureStyle(zDKUIDisplayScale);
            }
            else if (
                std::abs(zDKWinScale - zDKUIDisplayScale) > .001f ||
                std::abs(zDKWinScale - zDKWinFontScale) > .001f
            )
                zDKUIApplyDisplayScale(zDKWinScale, true);
        }
        if (zDKWinPendingWidth && zDKWinPendingHeight)
        {
            zDKWinRenderTarget.Reset();
            if (FAILED(zDKWinSwapChain->ResizeBuffers(
                    0, zDKWinPendingWidth, zDKWinPendingHeight, DXGI_FORMAT_UNKNOWN, 0
                )))
            {
                zDKWinDone = true;
                zDKWinRendering = false;
                return;
            }
            zDKWinPendingWidth = zDKWinPendingHeight = 0;
            zDKWinCreateRenderTarget();
            if (!zDKWinRenderTarget)
            {
                zDKWinDone = true;
                zDKWinRendering = false;
                return;
            }
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        if (zDKWinDiagnostic)
            zDKWinInputOutput.DeltaTime = 1.f / 60;
        if (zDKUICollectInteractionProbes)
        {
            // Hidden interaction checks use deterministic synthetic input, independent of desktop focus.
            zDKWinInputOutput.ClearEventsQueue();
            zDKWinSmoke.zDKTestBeforeFrame();
        }
        if (zDKWinPreviewSwitch && zDKWinFrames == 10)
            zDKUIActiveTab = 1;
        ImGui::NewFrame();
        if (zDKWinDiagnostic && zDKWinPreviewTurnPlan && zDKWinFrames == 10)
            zDKUIRequestTurnPlan = true;
        if (zDKWinDiagnostic && (zDKWinPreviewSettings || zDKWinPreviewUpdates) && zDKWinFrames == 10)
        {
            ImGui::Begin("Dokkan Defense Calculator");
            ImGui::OpenPopup(zDKWinPreviewUpdates ? "App updates" : "settings");
            ImGui::End();
        }
        zDKUIRender(zDKWinWindowHandle);
        if (!zDKWinDiagnostic)
            try
            {
                std::string zDKWinState = zDKStoreSaveSession(
                    zDKUICurrentUnit,
                    zDKUICurrentBoss,
                    zDKUICurrentBattle,
                    zDKUIAnimationsEnabled,
                    zDKUISelectedAttackKind
                );
                if (zDKWinState != zDKWinObserved)
                {
                    zDKWinObserved = zDKWinState;
                    zDKWinDirtyAt = ImGui::GetTime();
                }
                if (zDKWinState != zDKWinPersisted && ImGui::GetTime() - zDKWinDirtyAt > .6)
                {
                    if (zDKUIWriteFile(zDKUISessionPath(), zDKWinState))
                        zDKWinPersisted = zDKWinState;
                    else
                        zDKUIShowNotice("Unable to save the session.");
                }
            }
            catch (...)
            {
            }
        ImGui::Render();
        const float zDKWinClear[] = {.025f, .015f, .035f, 1};
        zDKWinDeviceContext->OMSetRenderTargets(1, zDKWinRenderTarget.GetAddressOf(), nullptr);
        zDKWinDeviceContext->ClearRenderTargetView(zDKWinRenderTarget.Get(), zDKWinClear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if (zDKWinDiagnostic)
        {
            ++zDKWinFrames;
            if (zDKWinFrames == 1 && zDKWinTransitionScale > 0)
            {
                zDKUIScaleOverride = zDKWinTransitionScale;
                zDKUIPendingDisplayScale = zDKWinTransitionScale;
            }
            if (zDKUICollectInteractionProbes)
            {
                if (zDKWinFrames >= 2600 && !zDKWinSmoke.zDKTestDone)
                    zDKWinSmoke.zDKTestExpect(false, "UI checks timed out");
                if (zDKWinSmoke.zDKTestDone)
                {
                    std::ofstream zDKWinOutputStream(zDKWinUiCheckPath);
                    zDKWinOutputStream << zDKWinSmoke.zDKTestLog.str();
                    if (zDKWinSmoke.zDKTestFailed)
                    {
                        zDKWinCaptureFrame(zDKWinUiCheckPath.wstring() + L".png");
                    }
                    zDKWinCaptureOK = !zDKWinSmoke.zDKTestFailed && bool(zDKWinOutputStream);
                    zDKWinDone = true;
                }
            }
            else if (zDKWinFrames >= zDKWinCaptureAt)
            {
                zDKWinCaptureOK = zDKWinCaptureFrame(zDKWinCapturePath);
                zDKWinDone = true;
            }
        }
        HRESULT zDKWinStatusCode = zDKWinSwapChain->Present(1, 0);
        if (zDKWinStatusCode == DXGI_STATUS_OCCLUDED)
            Sleep(zDKWinDiagnostic ? 1 : 40);
        else if (FAILED(zDKWinStatusCode))
        {
            zDKWinDone = true;
            zDKWinRendering = false;
            return;
        }
        zDKWinRendering = false;
    };
    while (!zDKWinDone)
    {
        MSG zDKWinMessage;
        while (PeekMessageW(&zDKWinMessage, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&zDKWinMessage);
            DispatchMessageW(&zDKWinMessage);
            if (zDKWinMessage.message == WM_QUIT)
                zDKWinDone = true;
        }
        if (zDKWinDone)
            break;
        if (IsIconic(zDKWinWindowHandle))
        {
            WaitMessage();
            continue;
        }
        zDKWinRenderDuringResize();
    }
    KillTimer(zDKWinWindowHandle, 1);
    zDKWinRenderDuringResize = {};
    if (!zDKWinDiagnostic)
        try
        {
            std::string zDKWinCurrent = zDKStoreSaveSession(
                zDKUICurrentUnit,
                zDKUICurrentBoss,
                zDKUICurrentBattle,
                zDKUIAnimationsEnabled,
                zDKUISelectedAttackKind
            );
            if (zDKWinCurrent != zDKWinPersisted)
                zDKUIWriteFile(zDKUISessionPath(), zDKWinCurrent);
        }
        catch (...)
        {
        }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    for (auto& zDKWinSkin : zDKUISkinTextures)
        for (auto& zDKWinState : zDKWinSkin)
            zDKWinState.Reset();
    zDKUIBackgroundTexture.Reset();
    zDKWinRenderTarget.Reset();
    zDKWinSwapChain.Reset();
    zDKWinDeviceContext.Reset();
    zDKWinDevice.Reset();
    DestroyWindow(zDKWinWindowHandle);
    UnregisterClassW(zDKWinWindowClass.lpszClassName, zDKWinInstance);
    CoUninitialize();
    return zDKWinCaptureOK ? 0 : 2;
}

// Windows requires this exact external entry point. Application logic uses the project naming convention.
int WINAPI wWinMain(HINSTANCE zDKWinInstance, HINSTANCE, LPWSTR, int)
{
    int zDKWinArgumentCount = 0;
    LPWSTR* zDKWinArguments = CommandLineToArgvW(GetCommandLineW(), &zDKWinArgumentCount);
    if (zDKWinArguments && zDKWinArgumentCount == 3 && std::wstring(zDKWinArguments[1]) == L"--apply-update")
    {
        const std::filesystem::path zDKWinConfiguration(zDKWinArguments[2]);
        LocalFree(zDKWinArguments);
        return zDK::zDKUpdateApply(zDKWinConfiguration);
    }
    if (zDKWinArguments && zDKWinArgumentCount == 3 && std::wstring(zDKWinArguments[1]) == L"--check-updates")
    {
        std::ofstream zDKWinReport(zDKWinArguments[2]);
        LocalFree(zDKWinArguments);
        try
        {
            const auto zDKWinRelease = zDK::zDKUpdateCheck();
            zDKWinReport << zDKWinRelease.zDKUpdateVersion << '\n' << zDKWinRelease.zDKUpdateSha256 << '\n';
            return zDKWinReport ? 0 : 1;
        }
        catch (const std::exception& zDKWinError)
        {
            zDKWinReport << zDKWinError.what();
            return 1;
        }
    }
    if (zDKWinArguments)
        LocalFree(zDKWinArguments);
    return zDKWinRunApplication(zDKWinInstance);
}
