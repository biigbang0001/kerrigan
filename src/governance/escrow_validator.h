// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_GOVERNANCE_ESCROW_VALIDATOR_H
#define BITCOIN_GOVERNANCE_ESCROW_VALIDATOR_H

#include <governance/validator_base.h>

#include <string>

class CScript;

class CEscrowReleaseValidator : public CGovernanceValidatorBase
{
public:
    explicit CEscrowReleaseValidator(const std::string& strDataHexIn = std::string());

    bool Validate();

    bool GetPaymentAmount(double& dAmountRet) const;
    bool GetPaymentAmountStr(std::string& strAmountRet) const;
    bool GetPaymentAddress(std::string& strAddressRet) const;

private:
    void ParseStrHexData(const std::string& strHexData);

    bool ValidateType();
    bool ValidatePaymentAmount();
    bool ValidatePaymentAddress();
    bool ValidateURL();
};

#endif // BITCOIN_GOVERNANCE_ESCROW_VALIDATOR_H
