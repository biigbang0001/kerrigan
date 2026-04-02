// Copyright (c) 2014-2023 The Dash Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_GOVERNANCE_VALIDATORS_H
#define BITCOIN_GOVERNANCE_VALIDATORS_H

#include <governance/validator_base.h>

#include <string>

class CProposalValidator : public CGovernanceValidatorBase
{
private:
    bool fAllowScript;

public:
    explicit CProposalValidator(const std::string& strDataHexIn = std::string(), bool fAllowScript = true);

    bool Validate(bool fCheckExpiration = true);

private:
    void ParseStrHexData(const std::string& strHexData);

    // Override base GetDataValue to log exception details
    bool GetDataValue(const std::string& strKey, std::string& strValueRet) override;
    bool GetDataValue(const std::string& strKey, int64_t& nValueRet) override;
    bool GetDataValue(const std::string& strKey, double& dValueRet) override;

    bool ValidateType();
    bool ValidateStartEndEpoch(bool fCheckExpiration = true);
    bool ValidatePaymentAmount();
    bool ValidatePaymentAddress();
    bool ValidateURL();

    bool CheckURL(const std::string& strURLIn);
};

#endif // BITCOIN_GOVERNANCE_VALIDATORS_H
