// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KERRIGAN_HMP_BROOD_LOOKUP_H
#define KERRIGAN_HMP_BROOD_LOOKUP_H

#include <hmp/privilege.h>
#include <evo/deterministicmns.h>
#include <evo/dmn_types.h>

// Queries the MN list for BroodNode operators (registered as MnType::Evo).
// Sole bridge between HMP and evo/; privilege tracker depends only on the interface.
class CBroodNodeLookup : public IMasternodeLookup
{
    CDeterministicMNManager& m_dmnman;
public:
    explicit CBroodNodeLookup(CDeterministicMNManager& dmnman) : m_dmnman(dmnman) {}

    bool IsBroodNodeOperator(const CBLSPublicKey& operatorPubKey,
                             const CBlockIndex* pindex = nullptr) const override
    {
        auto mnList = pindex ? m_dmnman.GetListForBlock(pindex)
                             : m_dmnman.GetListAtChainTip();
        bool found = false;
        mnList.ForEachMN(true /* onlyValid (non-banned) */, [&](auto& dmn) {
            if (found) return;
            if (dmn.pdmnState->pubKeyOperator.Get() == operatorPubKey &&
                dmn.nType == MnType::Evo) {
                found = true;
            }
        });
        return found;
    }
};

#endif // KERRIGAN_HMP_BROOD_LOOKUP_H
