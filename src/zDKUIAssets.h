#pragma once

// Generated mobile skin resource definitions.
namespace zDKUI
{
enum class zDKUISkinKind
{
    zDKUIHeader,
    zDKUIPanel,
    zDKUIButton,
    zDKUIAction,
    zDKUITab,
    zDKUIField,
    zDKUIEffectRow,
    zDKUISection,
    zDKUIReticle,
    zDKUISparkle,
    zDKUIProgress,
    zDKUIDivider,
    zDKUICard,
    zDKUIToggle,
    zDKUICount
};

struct zDKUISkinDefinition
{
    int zDKUIWidth, zDKUIHeight, zDKUILeft, zDKUITop, zDKUIRight, zDKUIBottom;
    int zDKUIResource[3];
};

inline constexpr zDKUISkinDefinition zDKUISkinDefinitions[] = {
    {1286, 133, 74, 24, 74, 24, {200, 201, 202}}, // Header
    {754, 483, 74, 54, 74, 54, {203, 204, 205}},  // Panel
    {204, 58, 28, 16, 28, 16, {206, 207, 208}},   // Button
    {498, 86, 56, 24, 56, 24, {209, 210, 211}},   // Action
    {268, 102, 45, 28, 42, 28, {212, 213, 214}},  // Tab
    {210, 54, 25, 16, 25, 16, {215, 216, 217}},   // Field
    {496, 89, 58, 24, 42, 24, {218, 219, 220}},   // Effect
    {755, 117, 64, 28, 64, 28, {221, 222, 223}},  // Section
    {89, 89, 0, 0, 0, 0, {224, 225, 226}},        // Reticle
    {84, 92, 0, 0, 0, 0, {227, 228, 229}},        // Sparkle
    {208, 38, 20, 10, 36, 10, {230, 231, 232}},   // Progress
    {366, 22, 0, 0, 0, 0, {233, 234, 235}},       // Divider
    {214, 114, 30, 28, 30, 28, {236, 237, 238}},  // Card
    {140, 139, 24, 24, 48, 48, {239, 240, 241}},  // Toggle
};
} // namespace zDKUI
