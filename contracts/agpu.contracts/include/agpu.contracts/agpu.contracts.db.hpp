#pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

#include <utils.hpp>

#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>

namespace amax {

using namespace std;
using namespace eosio;

#define GLOBAL_TBL(name) struct [[eosio::table(name), eosio::contract("agpucontracts")]]
#define AGPU_TBL struct [[eosio::table, eosio::contract("agpucontracts")]]

#define SYMBOL(sym_code, precision) symbol(symbol_code(sym_code), precision)
static constexpr symbol ACPU_SYMBOL = SYMBOL("ACPU", 8);
static constexpr name   ACPU_MINING = "acpuminedapp"_n;

namespace NodeStatus {
   static constexpr eosio::name ENABLE{ "enable"_n };
   static constexpr eosio::name DISABLE{ "disable"_n };
} // namespace NodeStatus

/// global table
GLOBAL_TBL("global") global_t {
   name     admin;             // admin account
   name     bank;              // bank account
   name     usdt_contract;     // usdt contract account
   symbol   usdt_symbol;       // usdt symbol
   uint64_t node_id       = 0; // next node id
   uint64_t order_id      = 0; // next order id
   uint64_t invite_period = 10;

   EOSLIB_SERIALIZE(global_t, (admin)(bank)(usdt_contract)(usdt_symbol)(node_id)(order_id)(invite_period))
};

typedef eosio::singleton<"global"_n, global_t> global_singleton;

/// node table
// scope: contract account
AGPU_TBL node_t {
   uint64_t       node_id;     // node id
   asset          price;       // node price
   uint64_t       max_sale;    // max sale count
   uint64_t       total_saled; // total saled count
   name           status;      // node status
   time_point_sec start_time;  // start timestamp
   time_point_sec create_time; // create timestamp
   time_point_sec update_time; // update timestamp

   node_t() {}
   node_t(const uint64_t& i) : node_id(i) {}

   uint64_t primary_key() const { return node_id; }
   uint64_t scope() const { return 0; }

   typedef multi_index<"nodes"_n, node_t> tbl_t;

   EOSLIB_SERIALIZE(node_t, (node_id)(price)(max_sale)(total_saled)(status)(start_time)(create_time)(update_time))
};

/// node total table
// scope: user account
AGPU_TBL node_total_t {
   uint64_t       node_id;     // node id
   uint64_t       total;       // buy total
   time_point_sec create_time; // create timestamp
   time_point_sec update_time; // update timestamp

   node_total_t() {}
   node_total_t(const uint64_t& i) : node_id(i) {}

   uint64_t primary_key() const { return node_id; }
   uint64_t scope() const { return 0; }

   typedef multi_index<"nodetotals"_n, node_total_t> tbl_t;

   EOSLIB_SERIALIZE(node_total_t, (node_id)(total)(create_time)(update_time))
};

/// invite table
// scope: contract account
AGPU_TBL invite_t {
   name           user;             // user account
   name           inviter;          // inviter account
   uint64_t       invite_count = 0; // invite count
   time_point_sec create_time;      // create timestamp
   time_point_sec update_time;      // update timestamp

   invite_t() {}
   invite_t(const name& n) : user(n) {}

   uint64_t primary_key() const { return user.value; }
   uint64_t scope() const { return 0; }

   typedef multi_index<"invites"_n, invite_t> tbl_t;

   EOSLIB_SERIALIZE(invite_t, (user)(inviter)(invite_count)(create_time)(update_time))
};

/// order table
// scope: user account
AGPU_TBL order_t {
   uint64_t       order_id;    // order id
   uint64_t       node_id;     // node id
   name           user;        // user account
   name           inviter;     // inviter account
   asset          price;       // order price
   time_point_sec create_time; // create timestamp

   order_t() {}
   order_t(const uint64_t& i) : order_id(i) {}

   uint64_t primary_key() const { return order_id; }
   uint64_t scope() const { return 0; }

   typedef multi_index<"orders"_n, order_t> tbl_t;

   EOSLIB_SERIALIZE(order_t, (order_id)(node_id)(user)(inviter)(price)(create_time))
};

AGPU_TBL user_mining_site_t {
   name           account;                                   // 账号
   uint16_t       level          = 0;                        // 级别
   uint64_t       personal_num   = 0;                        // 个人矿机数量
   uint64_t       main_force_num = 0;                        // 主力矿机数量
   name           main_force_account;                        // 主力矿机账号
   uint64_t       assist_num        = 0;                     // 助力矿机数量 团队总矿机 - 主力矿机
   uint64_t       assist_member_num = 0;                     // 助力矿机人数 总邀请 - 1
   uint64_t       team_total_num    = 0;                     // 团队总矿机数量
   uint64_t       total_num         = 0;                     // 总矿机数量 个人+主力+助力
   asset          claimed_reward    = asset(0, ACPU_SYMBOL); // 总领取收益
   time_point_sec created_at;                                // 创建时间
   time_point_sec updated_at;                                // 更新时间
   time_point_sec upgraded_at;                               // 级别更新时间

   uint64_t primary_key() const { return account.value; }
   uint64_t scope() const { return 0; }

   user_mining_site_t() {}
   user_mining_site_t(const name& i) : account(i) {}

   EOSLIB_SERIALIZE(
         user_mining_site_t,
         (account)(level)(personal_num)(main_force_num)(main_force_account)(assist_num)(assist_member_num)(team_total_num)(total_num)(claimed_reward)(created_at)(updated_at)(upgraded_at))

   typedef eosio::multi_index<"usermisite"_n, user_mining_site_t> idx_t;
};

} // namespace amax
