#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif

#include "atm.hpp"
#include "catch.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
//                             Helper Definitions //
/////////////////////////////////////////////////////////////////////////////////////////////

bool CompareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1);
  std::ifstream f2(p2);

  if (f1.fail() || f2.fail()) {
    return false;  // file problem
  }

  std::string f1_read;
  std::string f2_read;
  while (f1.good() || f2.good()) {
    f1 >> f1_read;
    f2 >> f2_read;
    if (f1_read != f2_read || (f1.good() && !f2.good()) ||
        (!f1.good() && f2.good()))
      return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Test Cases
/////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Example: Create a new account", "[ex-1]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto accounts = atm.GetAccounts();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);

  Account sam_account = accounts[{12345678, 1234}];
  REQUIRE(sam_account.owner_name == "Sam Sepiol");
  REQUIRE(sam_account.balance == 300.30);

  auto transactions = atm.GetTransactions();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);
  std::vector<std::string> empty;
  REQUIRE(transactions[{12345678, 1234}] == empty);
}

TEST_CASE("Example: Simple widthdraw", "[ex-2]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.WithdrawCash(12345678, 1234, 20);
  auto accounts = atm.GetAccounts();
  Account sam_account = accounts[{12345678, 1234}];

  REQUIRE(sam_account.balance == 280.30);
}

TEST_CASE("Example: Print Prompt Ledger", "[ex-3]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto& transactions = atm.GetTransactions();
  transactions[{12345678, 1234}].push_back(
      "Withdrawal - Amount: $200.40, Updated Balance: $99.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $40000.00, Updated Balance: $40099.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $32000.00, Updated Balance: $72099.900");
  atm.PrintLedger("./prompt.txt", 12345678, 1234);
  REQUIRE(CompareFiles("./ex-1.txt", "./prompt.txt"));
} 

TEST_CASE("RegisterAccount: Check for duplicate account handling", "[adversarial]") {
  Atm atm;
  atm.RegisterAccount(1111, 1234, "User A", 100.0);
  CHECK_THROWS_AS(atm.RegisterAccount(1111, 1234, "User B", 200.0), std::invalid_argument);
}

TEST_CASE("WithdrawCash: Negative amounts and overdrafts", "[adversarial]") {
  Atm atm;
  atm.RegisterAccount(2222, 1234, "User B", 100.0);
  
  SECTION("Negative withdrawal should throw invalid_argument") {
    CHECK_THROWS_AS(atm.WithdrawCash(2222, 1234, -50.0), std::invalid_argument);
  }
  
  SECTION("Overdraft should throw runtime_error") {
    CHECK_THROWS_AS(atm.WithdrawCash(2222, 1234, 150.0), std::runtime_error);
  }
}

TEST_CASE("DepositCash: Negative amounts", "[adversarial]") {
  Atm atm;
  atm.RegisterAccount(3333, 1234, "User C", 100.0);
  
  CHECK_THROWS_AS(atm.DepositCash(3333, 1234, -50.0), std::invalid_argument);
}

TEST_CASE("Transaction History: Ensure Withdraw/Deposit record transactions", "[adversarial]") {
  Atm atm;
  atm.RegisterAccount(4444, 1234, "User D", 100.0);
  atm.DepositCash(4444, 1234, 50.0);
  atm.WithdrawCash(4444, 1234, 20.0);
  
  auto& transactions = atm.GetTransactions();
  REQUIRE(transactions[{4444, 1234}].size() == 2);
}

TEST_CASE("PrintLedger: Verify non-existent account", "[adversarial]") {
  Atm atm;
  CHECK_THROWS_AS(atm.PrintLedger("test.txt", 9999, 9999), std::invalid_argument);
}
