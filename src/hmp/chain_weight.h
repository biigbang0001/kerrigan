// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KERRIGAN_HMP_CHAIN_WEIGHT_H
#define KERRIGAN_HMP_CHAIN_WEIGHT_H

#include <cstdint>
#include <vector>

class CBLSPublicKey;
class CBlockIndex;
class CHMPPrivilegeTracker;

/**
 * Compute the seal multiplier for a block, in basis points.
 * Range: 10000 (no seal) to 19500 (full shade + cross-algo bonus).
 * @param pindex  Block index for deterministic MN list lookup (consensus path).
 *                Pass nullptr for non-consensus callers (RPC, tests).
 * @return multiplier in basis points (10000 = 1.0x)
 */
uint64_t ComputeSealMultiplier(const std::vector<CBLSPublicKey>& signerPubKeys,
                                const std::vector<uint8_t>& signerAlgos,
                                int blockAlgo,
                                const CHMPPrivilegeTracker* privilege,
                                const CBlockIndex* pindex = nullptr);

/** PRD v3 Section 7.2: required Elder agreement percentage by per-algo count. */
int GetRequiredAgreement(int totalElders);

#endif // KERRIGAN_HMP_CHAIN_WEIGHT_H
