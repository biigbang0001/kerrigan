// Copyright (c) 2026 Kerrigan Network
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <governance/escrow_validator.h>
#include <governance/common.h>
#include <evo/evodb.h>
#include <key.h>
#include <key_io.h>
#include <pubkey.h>
#include <script/script.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

static std::string MakeEscrowReleaseHex(const std::string& name, const std::string& address,
                                         double amount, const std::string& url)
{
    std::string json = "{";
    json += "\"type\":3,";
    json += "\"name\":\"" + name + "\",";
    json += "\"payment_address\":\"" + address + "\",";
    json += "\"payment_amount\":" + std::to_string(amount) + ",";
    json += "\"url\":\"" + url + "\"";
    json += "}";
    return HexStr(std::vector<unsigned char>(json.begin(), json.end()));
}

static std::string MakeProposalHex(int type, const std::string& name, const std::string& address,
                                    double amount, const std::string& url)
{
    std::string json = "{";
    json += "\"type\":" + std::to_string(type) + ",";
    json += "\"name\":\"" + name + "\",";
    json += "\"payment_address\":\"" + address + "\",";
    json += "\"payment_amount\":" + std::to_string(amount) + ",";
    json += "\"url\":\"" + url + "\"";
    json += "}";
    return HexStr(std::vector<unsigned char>(json.begin(), json.end()));
}

// Generate a valid regtest address from a random key
static std::string MakeRegtestAddress()
{
    CKey key;
    key.MakeNewKey(true);
    CTxDestination dest = PKHash(key.GetPubKey());
    return EncodeDestination(dest);
}

BOOST_FIXTURE_TEST_SUITE(escrow_release_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(escrow_release_validator_valid)
{
    std::string addr = MakeRegtestAddress();
    std::string hex = MakeEscrowReleaseHex("exchange-listing", addr, 50000.0, "https://kerrigan.network/proposals/001");
    CEscrowReleaseValidator validator(hex);
    BOOST_CHECK_MESSAGE(validator.Validate(), "Expected valid, got: " + validator.GetErrorMessages());
}

BOOST_AUTO_TEST_CASE(escrow_release_validator_invalid_type)
{
    std::string addr = MakeRegtestAddress();
    // type=1 is PROPOSAL, not ESCROW_RELEASE -- should fail type check
    std::string hex = MakeProposalHex(1, "test", addr, 100.0, "https://example.com");
    CEscrowReleaseValidator validator(hex);
    BOOST_CHECK(!validator.Validate());
}

BOOST_AUTO_TEST_CASE(escrow_release_validator_missing_amount)
{
    std::string addr = MakeRegtestAddress();
    // JSON without payment_amount field
    std::string json = "{\"type\":3,\"name\":\"test\",\"payment_address\":\"" + addr + "\","
                        "\"url\":\"https://example.com\"}";
    std::string hex = HexStr(std::vector<unsigned char>(json.begin(), json.end()));
    CEscrowReleaseValidator validator(hex);
    BOOST_CHECK(!validator.Validate());
}

BOOST_AUTO_TEST_CASE(escrow_release_validator_negative_amount)
{
    std::string addr = MakeRegtestAddress();
    std::string hex = MakeEscrowReleaseHex("test", addr, -100.0, "https://example.com");
    CEscrowReleaseValidator validator(hex);
    BOOST_CHECK(!validator.Validate());
}

BOOST_AUTO_TEST_CASE(escrow_release_validator_empty_name)
{
    std::string addr = MakeRegtestAddress();
    std::string hex = MakeEscrowReleaseHex("", addr, 100.0, "https://example.com");
    CEscrowReleaseValidator validator(hex);
    BOOST_CHECK(!validator.Validate());
}

// Test OP_RETURN parsing for the KRGN marker
BOOST_AUTO_TEST_CASE(escrow_op_return_format)
{
    // Build a valid OP_RETURN: OP_RETURN OP_PUSHBYTES_36 "KRGN" <32-byte hash>
    uint256 testHash = uint256S("abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
    CScript opReturnScript;
    std::vector<unsigned char> data;
    data.push_back('K');
    data.push_back('R');
    data.push_back('G');
    data.push_back('N');
    data.insert(data.end(), testHash.begin(), testHash.end());
    opReturnScript << OP_RETURN;
    opReturnScript << data;

    // Verify the script is 38 bytes: 1 (OP_RETURN) + 1 (push 36) + 4 (KRGN) + 32 (hash)
    BOOST_CHECK_EQUAL(opReturnScript.size(), 38U);
    BOOST_CHECK_EQUAL(opReturnScript[0], OP_RETURN);
    BOOST_CHECK_EQUAL(opReturnScript[1], 0x24); // push 36 bytes
    BOOST_CHECK_EQUAL(opReturnScript[2], 'K');
    BOOST_CHECK_EQUAL(opReturnScript[3], 'R');
    BOOST_CHECK_EQUAL(opReturnScript[4], 'G');
    BOOST_CHECK_EQUAL(opReturnScript[5], 'N');

    // Extract the hash back
    uint256 extractedHash;
    memcpy(extractedHash.begin(), opReturnScript.data() + 6, 32);
    BOOST_CHECK_EQUAL(extractedHash, testHash);
}

// Test anti-replay via evoDB
BOOST_AUTO_TEST_CASE(escrow_replay_prevention)
{
    // Create a temporary evodb for testing
    util::DbWrapperParams db_params{m_args.GetDataDirNet() / "evodb_test", true, false, 1 << 20};
    CEvoDB testEvoDB(db_params);

    uint256 proposalHash = uint256S("1111111111111111111111111111111111111111111111111111111111111111");

    // Should not be executed initially
    BOOST_CHECK(!IsEscrowReleaseExecuted(testEvoDB, proposalHash));

    // Mark as executed
    MarkEscrowReleaseExecuted(testEvoDB, proposalHash, 1000);
    BOOST_CHECK(IsEscrowReleaseExecuted(testEvoDB, proposalHash));

    // Unmark (reorg simulation)
    UnmarkEscrowReleaseExecuted(testEvoDB, proposalHash);
    BOOST_CHECK(!IsEscrowReleaseExecuted(testEvoDB, proposalHash));
}

BOOST_AUTO_TEST_CASE(escrow_release_accessor_methods)
{
    std::string addr = MakeRegtestAddress();
    std::string hex = MakeEscrowReleaseHex("test-release", addr, 25000.5, "https://example.com");
    CEscrowReleaseValidator validator(hex);
    BOOST_CHECK_MESSAGE(validator.Validate(), "Expected valid, got: " + validator.GetErrorMessages());

    double amount = 0;
    BOOST_CHECK(validator.GetPaymentAmount(amount));
    BOOST_CHECK_CLOSE(amount, 25000.5, 0.001);

    std::string extractedAddr;
    BOOST_CHECK(validator.GetPaymentAddress(extractedAddr));
    BOOST_CHECK_EQUAL(extractedAddr, addr);
}

BOOST_AUTO_TEST_SUITE_END()
