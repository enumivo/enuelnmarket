#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::buy(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  
  enu_balance = enu_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  enu_balance = enu_balance-received;  

  // get ELN balance
  double eln_balance = enumivo::token(N(eln.coin)).
	   get_balance(_self, enumivo::symbol_type(ELN_SYMBOL).name()).amount;

  eln_balance = eln_balance/10000;

  // get ELN supply
  double eln_supply = enumivo::token(N(eln.coin)).
	   get_supply(enumivo::symbol_type(ELN_SYMBOL).name()).amount;

  eln_supply = eln_supply/10000;

  double buy = eln_balance*(pow(1+(received/enu_balance), 0.5) - 1);
  
  auto to = transfer.from;

  auto quantity = asset(10000*buy, ELN_SYMBOL);

  action(permission_level{_self, N(active)}, N(eln.coin), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Buy ELN with ENU")))
      .send();
}

void ex::sell(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ELN balance
  double eln_balance = enumivo::token(N(eln.coin)).
	   get_balance(_self, enumivo::symbol_type(ELN_SYMBOL).name()).amount;
  
  eln_balance = eln_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  eln_balance = eln_balance-received;  

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;

  enu_balance = enu_balance/10000;

  // get ELN supply
  double eln_supply = enumivo::token(N(eln.coin)).
	   get_supply(enumivo::symbol_type(ELN_SYMBOL).name()).amount;

  eln_supply = eln_supply/10000;

  double sell = -enu_balance*(pow(1-(received/eln_balance), 1/0.5) - 1);

  auto to = transfer.from;

  auto quantity = asset(10000*sell, ENU_SYMBOL);

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Sell ELN for ENU")))
      .send();
}

void ex::apply(account_name contract, action_name act) {

  bool pause = true;

  if (act == N(transfer) && pause) {
    auto transfer = unpack_action_data<currency::transfer>();
    if (transfer.from == N(aiden.pearce))
      return;
  }
  
  enumivo_assert(!pause && act != N(testaccount1)), "Paused pending review.");

  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "Must send ENU");
    buy(transfer);
    return;
  }

  if (contract == N(eln.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ELN_SYMBOL,
                 "Must send ELN");
    sell(transfer);
    return;
  }

  if (act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(false, "Must send ELN or ENU");
    sell(transfer);
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enueln(receiver);
  enueln.apply(code, action);
  enumivo_exit(0);
}
}
