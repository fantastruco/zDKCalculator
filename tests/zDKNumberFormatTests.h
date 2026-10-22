#pragma once
#include "zDKUINumberFormat.h"

void zDKTestNumberFormatting()
{
    using namespace zDKUI;
    zDKTestExpect(zDKUIGroupNumberText("1000") == "1,000", "Thousands group at the fourth digit");
    zDKTestExpect(zDKUIGroupNumberText("1000000") == "1,000,000", "Millions use both separators");
    zDKTestExpect(
        zDKUIGroupNumberText("-1234567.50") == "-1,234,567.50",
        "Grouping preserves sign and decimal precision"
    );
    zDKTestExpect(zDKUIGroupNumberText("1234567e+12") == "1,234,567e+12", "Grouping preserves exponent text");
    zDKTestExpect(zDKUIGroupNumberText("1,234,567") == "1,234,567", "Pasted grouping is stable");
    zDKTestExpect(
        zDKUIGroupNumberText("1000.") == "1,000.", "Typing a decimal point preserves incomplete input"
    );
    zDKTestExpect(zDKUIGroupNumberText("-") == "-", "Typing a negative sign preserves incomplete input");
    zDKTestExpect(
        zDKUINumberGroupedPosition("1,234,567", zDKUINumberRawPosition("1234567", 4)) == 5,
        "Inserted separator preserves the semantic caret position"
    );
    double zDKTestNumber = 0;
    zDKTestExpect(
        zDKUIParseNumber("1,234,567.5", zDKTestNumber) && zDKTestNumber == 1234567.5,
        "Grouped decimals parse without losing their value"
    );
    zDKTestExpect(
        zDKUIParseNumber("-1,234.50", zDKTestNumber) && zDKTestNumber == -1234.5,
        "Grouped negative numbers parse"
    );
    zDKTestExpect(
        zDKUIParseNumber("+1,234e2", zDKTestNumber) && zDKTestNumber == 123400,
        "Scientific input remains supported"
    );
    zDKTestExpect(
        !zDKUIParseNumber("1e-", zDKTestNumber) && zDKTestNumber == 123400,
        "Incomplete exponent does not replace the model value"
    );
    zDKTestExpect(!zDKUIParseNumber("1e9999", zDKTestNumber), "Out-of-range input does not become infinity");
    int zDKTestInteger = 0;
    zDKTestExpect(
        zDKUIParseInteger("2,147,483,647", zDKTestInteger) && zDKTestInteger == 2147483647,
        "Grouped integers parse exactly"
    );
    zDKTestExpect(
        !zDKUIParseInteger("2,147,483,648", zDKTestInteger) && zDKTestInteger == 2147483647,
        "Integer overflow preserves the existing value"
    );
    zDKTestExpect(
        zDKUIParseInteger("", zDKTestInteger) && zDKTestInteger == 0,
        "Clearing an integer field resets its value"
    );
}
