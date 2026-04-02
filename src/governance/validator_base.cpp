// Copyright (c) 2014-2025 The Dash Core developers
// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <governance/validator_base.h>

#include <tinyformat.h>
#include <util/strencodings.h>

void CGovernanceValidatorBase::ParseJSONData(const std::string& strJSONData, const char* strObjectLabel)
{
    fJSONValid = false;
    if (strJSONData.empty()) return;

    try {
        UniValue obj(UniValue::VOBJ);
        obj.read(strJSONData);
        if (obj.isObject()) {
            objJSON = obj;
        } else {
            throw std::runtime_error(strprintf("%s must be a JSON object", strObjectLabel));
        }
        fJSONValid = true;
    } catch (std::exception& e) {
        strErrorMessages += std::string(e.what()) + std::string(";");
    } catch (...) {
        strErrorMessages += "Unknown exception;";
    }
}

bool CGovernanceValidatorBase::ValidateName()
{
    std::string strName;
    if (!GetDataValue("name", strName)) {
        strErrorMessages += "name field not found;";
        return false;
    }

    if (strName.size() > MAX_NAME_SIZE) {
        strErrorMessages += strprintf("name exceeds %lu characters;", MAX_NAME_SIZE);
        return false;
    }

    if (strName.empty()) {
        strErrorMessages += "name cannot be empty;";
        return false;
    }

    static constexpr std::string_view strAllowedChars{"-_abcdefghijklmnopqrstuvwxyz0123456789"};

    strName = ToLower(strName);

    if (strName.find_first_not_of(strAllowedChars) != std::string::npos) {
        strErrorMessages += "name contains invalid characters;";
        return false;
    }

    return true;
}

bool CGovernanceValidatorBase::GetDataValue(const std::string& strKey, std::string& strValueRet)
{
    try {
        strValueRet = objJSON[strKey].get_str();
        return true;
    } catch (...) {}
    return false;
}

bool CGovernanceValidatorBase::GetDataValue(const std::string& strKey, int64_t& nValueRet)
{
    try {
        const UniValue uValue = objJSON[strKey];
        if (uValue.isNum()) {
            nValueRet = uValue.getInt<int64_t>();
            return true;
        }
    } catch (...) {}
    return false;
}

bool CGovernanceValidatorBase::GetDataValue(const std::string& strKey, double& dValueRet)
{
    try {
        const UniValue uValue = objJSON[strKey];
        if (uValue.isNum()) {
            dValueRet = uValue.get_real();
            return true;
        }
    } catch (...) {}
    return false;
}
