// Copyright (c) 2014-2025 The Dash Core developers
// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_GOVERNANCE_VALIDATOR_BASE_H
#define BITCOIN_GOVERNANCE_VALIDATOR_BASE_H

#include <string>

#include <univalue.h>

class CGovernanceValidatorBase
{
public:
    virtual ~CGovernanceValidatorBase() = default;

    const std::string& GetErrorMessages() const { return strErrorMessages; }

protected:
    static constexpr size_t MAX_DATA_SIZE = 512;
    static constexpr size_t MAX_NAME_SIZE = 40;

    UniValue objJSON{UniValue::VOBJ};
    bool fJSONValid{false};
    std::string strErrorMessages;

    void ParseJSONData(const std::string& strJSONData, const char* strObjectLabel);

    bool ValidateName();

    // Data access helpers. Default implementations silently catch exceptions.
    // CProposalValidator overrides these to log exception details.
    virtual bool GetDataValue(const std::string& strKey, std::string& strValueRet);
    virtual bool GetDataValue(const std::string& strKey, int64_t& nValueRet);
    virtual bool GetDataValue(const std::string& strKey, double& dValueRet);
};

#endif // BITCOIN_GOVERNANCE_VALIDATOR_BASE_H
