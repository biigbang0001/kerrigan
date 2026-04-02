// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <governance/escrow_validator.h>
#include <governance/common.h>

#include <consensus/amount.h>
#include <key_io.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <util/underlying.h>

#include <algorithm>
#include <cmath>

CEscrowReleaseValidator::CEscrowReleaseValidator(const std::string& strHexData)
{
    if (!strHexData.empty()) {
        ParseStrHexData(strHexData);
    }
}

void CEscrowReleaseValidator::ParseStrHexData(const std::string& strHexData)
{
    std::vector<unsigned char> v = ParseHex(strHexData);
    if (v.size() > MAX_DATA_SIZE) {
        strErrorMessages = strprintf("data exceeds %lu characters;", MAX_DATA_SIZE);
        return;
    }
    ParseJSONData(std::string(v.begin(), v.end()), "Escrow release");
}

bool CEscrowReleaseValidator::Validate()
{
    if (!fJSONValid) {
        strErrorMessages += "JSON parsing error;";
        return false;
    }
    if (!ValidateType()) {
        strErrorMessages += "Invalid type;";
        return false;
    }
    if (!ValidateName()) {
        strErrorMessages += "Invalid name;";
        return false;
    }
    if (!ValidatePaymentAmount()) {
        strErrorMessages += "Invalid payment amount;";
        return false;
    }
    if (!ValidatePaymentAddress()) {
        strErrorMessages += "Invalid payment address;";
        return false;
    }
    if (!ValidateURL()) {
        strErrorMessages += "Invalid URL;";
        return false;
    }
    return true;
}

bool CEscrowReleaseValidator::ValidateType()
{
    int64_t nType;
    if (!GetDataValue("type", nType)) {
        strErrorMessages += "type field not found;";
        return false;
    }
    if (nType != ToUnderlying(GovernanceObject::ESCROW_RELEASE)) {
        strErrorMessages += strprintf("type is not %d;", ToUnderlying(GovernanceObject::ESCROW_RELEASE));
        return false;
    }
    return true;
}

bool CEscrowReleaseValidator::ValidatePaymentAmount()
{
    double dValue = 0.0;
    if (!GetDataValue("payment_amount", dValue)) {
        strErrorMessages += "payment_amount field not found;";
        return false;
    }
    if (!std::isfinite(dValue) || dValue <= 0.0) {
        strErrorMessages += "payment_amount is not a positive finite number;";
        return false;
    }
    if (dValue > static_cast<double>(MAX_MONEY) / COIN) {
        strErrorMessages += "payment_amount is out of range;";
        return false;
    }
    return true;
}

bool CEscrowReleaseValidator::ValidatePaymentAddress()
{
    std::string strPaymentAddress;
    if (!GetDataValue("payment_address", strPaymentAddress)) {
        strErrorMessages += "payment_address field not found;";
        return false;
    }
    if (std::find_if(strPaymentAddress.begin(), strPaymentAddress.end(), IsSpace) != strPaymentAddress.end()) {
        strErrorMessages += "payment_address can't have whitespaces;";
        return false;
    }
    CTxDestination dest = DecodeDestination(strPaymentAddress);
    if (!IsValidDestination(dest)) {
        strErrorMessages += "payment_address is invalid;";
        return false;
    }
    return true;
}

bool CEscrowReleaseValidator::ValidateURL()
{
    std::string strURL;
    if (!GetDataValue("url", strURL)) {
        strErrorMessages += "url field not found;";
        return false;
    }
    if (std::find_if(strURL.begin(), strURL.end(), IsSpace) != strURL.end()) {
        strErrorMessages += "url can't have whitespaces;";
        return false;
    }
    if (strURL.size() < 4U) {
        strErrorMessages += "url too short;";
        return false;
    }
    return true;
}

bool CEscrowReleaseValidator::GetPaymentAmount(double& dAmountRet) const
{
    try {
        const UniValue& uValue = objJSON["payment_amount"];
        if (uValue.isNum()) {
            dAmountRet = uValue.get_real();
            return true;
        }
    } catch (...) {}
    return false;
}

bool CEscrowReleaseValidator::GetPaymentAmountStr(std::string& strAmountRet) const
{
    try {
        const UniValue& uValue = objJSON["payment_amount"];
        if (uValue.isNum()) {
            strAmountRet = uValue.getValStr();
            return !strAmountRet.empty();
        }
    } catch (...) {}
    return false;
}

bool CEscrowReleaseValidator::GetPaymentAddress(std::string& strAddressRet) const
{
    try {
        strAddressRet = objJSON["payment_address"].get_str();
        return !strAddressRet.empty();
    } catch (...) {}
    return false;
}
