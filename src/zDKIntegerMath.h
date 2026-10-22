#pragma once
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace zDK
{
using zDKCalcInteger = std::uint64_t;

// Public/persisted values use doubles. Keep every integer exactly representable there.
constexpr zDKCalcInteger zDKCalcMaximumInteger = 9007199254740991ULL;
constexpr zDKCalcInteger zDKCalcRatioScale = 1000000000ULL;

inline zDKCalcInteger zDKCalcFloorStatistic(double zDKCalcValue)
{
    if (!std::isfinite(zDKCalcValue) || zDKCalcValue < 0 ||
        zDKCalcValue > static_cast<double>(zDKCalcMaximumInteger))
        throw std::invalid_argument("Statistic exceeds the exact integer range (9,007,199,254,740,991).");
    return static_cast<zDKCalcInteger>(zDKCalcValue);
}

inline zDKCalcInteger zDKCalcAddIntegers(zDKCalcInteger zDKCalcLeft, zDKCalcInteger zDKCalcRight)
{
    if (zDKCalcRight > zDKCalcMaximumInteger || zDKCalcLeft > zDKCalcMaximumInteger - zDKCalcRight)
        throw std::invalid_argument("Combined statistic exceeds the exact integer range.");
    return zDKCalcLeft + zDKCalcRight;
}

// Split the dividend before multiplication: no floating point statistics or overflowing product.
inline zDKCalcInteger zDKCalcMultiplyRatio(
    zDKCalcInteger zDKCalcValue, zDKCalcInteger zDKCalcNumerator, zDKCalcInteger zDKCalcDenominator
)
{
    if (zDKCalcDenominator == 0 || zDKCalcDenominator > zDKCalcRatioScale ||
        (zDKCalcDenominator != 1 && zDKCalcNumerator > zDKCalcRatioScale))
        throw std::invalid_argument("Unsupported integer ratio.");
    const auto zDKCalcQuotient = zDKCalcValue / zDKCalcDenominator;
    if (zDKCalcNumerator && zDKCalcQuotient > zDKCalcMaximumInteger / zDKCalcNumerator)
        throw std::invalid_argument("Combined statistic exceeds the exact integer range.");
    return zDKCalcAddIntegers(
        zDKCalcQuotient * zDKCalcNumerator,
        ((zDKCalcValue % zDKCalcDenominator) * zDKCalcNumerator) / zDKCalcDenominator
    );
}

inline zDKCalcInteger zDKCalcApplyMultiplier(zDKCalcInteger zDKCalcValue, double zDKCalcFactor)
{
    if (!std::isfinite(zDKCalcFactor) || zDKCalcFactor < 0 || zDKCalcFactor > 1e12)
        throw std::invalid_argument("Multiplier is outside the supported range.");
    auto zDKCalcWhole = static_cast<zDKCalcInteger>(zDKCalcFactor);
    auto zDKCalcFraction = static_cast<zDKCalcInteger>(
        std::llround((zDKCalcFactor - static_cast<double>(zDKCalcWhole)) * zDKCalcRatioScale)
    );
    if (zDKCalcFraction == zDKCalcRatioScale)
    {
        ++zDKCalcWhole;
        zDKCalcFraction = 0;
    }
    return zDKCalcAddIntegers(
        zDKCalcMultiplyRatio(zDKCalcValue, zDKCalcWhole, 1),
        zDKCalcMultiplyRatio(zDKCalcValue, zDKCalcFraction, zDKCalcRatioScale)
    );
}
} // namespace zDK
