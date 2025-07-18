#pragma once

#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>

#include <string>

#include <agpu.contracts/agpu.contracts.db.hpp>
#include <wasm_db.hpp>

namespace amax {

using std::string;
using std::vector;

using namespace eosio;
using namespace wasm::db;

class [[eosio::contract("agpucontracts")]] agpu : public contract {

 public:
   using contract::contract;

   agpu(eosio::name receiver, eosio::name code, datastream<const char*> ds)
       : contract(receiver, code, ds), _global(get_self(), get_self().value), _db(_self) {
      _gstate = _global.exists() ? _global.get() : global_t{};
   }

   ~agpu() { _global.set(_gstate, get_self()); }

   ACTION init(const name& admin, const name& bank, const name& usdt_contract, const symbol& usdt_symbol);

   ACTION addnode(const asset& price, const uint64_t& max_sale, const uint32_t& start_time);

   ACTION setnode(const uint64_t& node_id, const asset& price, const uint64_t& max_sale, const uint32_t& start_time);

   ACTION delnode(const uint64_t& node_id);

   ACTION settotalsale(const uint64_t& node_id, const uint64_t& total_saled);

   ACTION setnodestate(const uint64_t& node_id, const name& status);

   ACTION signup(const name& user, const name& inviter);

   ACTION signbind(const name& user, const name& inviter);

   ACTION signedit(const name& user, const name& inviter);

   ACTION signdel(const name& user);

   ACTION addorder(const uint64_t& node_id, const name& user, const asset& quantity);

   ACTION delorder(const uint64_t& order_id, const name& user);

   [[eosio::on_notify("*::transfer")]] void on_transfer(const name& from, const name& to, const asset& quantity, const string& memo);

 private:
   global_singleton _global;
   global_t         _gstate;
   dbc              _db;

   void _buy(const uint64_t& node_id, const name& user, const asset& quantity);
};

} // namespace amax
