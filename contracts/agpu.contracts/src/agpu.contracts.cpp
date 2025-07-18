#include <agpu.contracts/agpu.contracts.hpp>
#include <amax.token.hpp>
#include <utils.hpp>

namespace amax {

using namespace std;

// clang-format off
#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + to_string((int)code) + string("]] ") + msg); }
// clang-format on

/// @brief contract account initializes the project configuration
/// @param admin - admin account name
/// @param bank - bank account name
/// @param usdt_contract - usdt contract account name
/// @param usdt_symbol - usdt symbol
void agpu::init(const name& admin, const name& bank, const name& usdt_contract, const symbol& usdt_symbol) {
   require_auth(_self);

   CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not found: " + admin.to_string())
   CHECKC(is_account(bank), err::ACCOUNT_INVALID, "bank not found: " + bank.to_string())
   CHECKC(is_account(usdt_contract), err::ACCOUNT_INVALID, "usdt_contract not found: " + usdt_contract.to_string())
   CHECKC(usdt_symbol.is_valid(), err::PARAM_ERROR, "invalid usdt_symbol: " + usdt_symbol.code().to_string())

   _gstate.admin         = admin;
   _gstate.bank          = bank;
   _gstate.usdt_contract = usdt_contract;
   _gstate.usdt_symbol   = usdt_symbol;
}

/// @brief add node action only for admin
/// @param price - node price
/// @param max_sale - max sale count
void agpu::addnode(const asset& price, const uint64_t& max_sale, const uint32_t& start_time) {
   require_auth(_gstate.admin);

   CHECKC(price.is_valid() && price.amount > 0, err::PARAM_ERROR, "invalid price");
   CHECKC(max_sale > 0, err::PARAM_ERROR, "invalid max_sale" + to_string(max_sale));
   CHECKC(start_time >= current_time_point().sec_since_epoch(), err::PARAM_ERROR, "start_time must be in the future");

   uint64_t node_id = ++_gstate.node_id;
   node_t   node(node_id);
   CHECKC(!_db.get(node), err::RECORD_FOUND, "node found: " + to_string(node_id));

   node.price       = price;
   node.max_sale    = max_sale;
   node.total_saled = 0;
   node.status      = NodeStatus::ENABLE;
   node.start_time  = time_point_sec(start_time);
   node.create_time = current_time_point();
   node.update_time = current_time_point();
   _db.set(node);
}

/// @brief set node action only for admin
/// @param node_id - node id
/// @param price - node price
/// @param max_sale - max sale count
void agpu::setnode(const uint64_t& node_id, const asset& price, const uint64_t& max_sale, const uint32_t& start_time) {
   require_auth(_gstate.admin);

   CHECKC(node_id > 0, err::PARAM_ERROR, "invalid node_id" + to_string(node_id));
   CHECKC(price.is_valid() && price.amount > 0, err::PARAM_ERROR, "invalid price");
   CHECKC(max_sale > 0, err::PARAM_ERROR, "invalid max_sale" + to_string(max_sale));
   CHECKC(start_time >= current_time_point().sec_since_epoch(), err::PARAM_ERROR, "start_time must be in the future");

   node_t node(node_id);
   CHECKC(_db.get(node), err::RECORD_NOT_FOUND, "node not found: " + to_string(node_id));

   node.price       = price;
   node.max_sale    = max_sale;
   node.start_time  = time_point_sec(start_time);
   node.update_time = current_time_point();
   _db.set(node);
}

/// @brief delete node action only for admin
/// @param node_id - node id
void agpu::delnode(const uint64_t& node_id) {
   require_auth(_gstate.admin);

   CHECKC(node_id > 0, err::PARAM_ERROR, "invalid node_id" + to_string(node_id));

   node_t node(node_id);
   CHECKC(_db.get(node), err::RECORD_NOT_FOUND, "node not found: " + to_string(node_id));
   CHECKC(node.status == NodeStatus::DISABLE, err::PARAM_ERROR, "node is enable: " + to_string(node_id));

   _db.del(node);
}

/// @brief set node total saled count only for admin
/// @param node_id - node id
/// @param total_saled - total saled count
void agpu::settotalsale(const uint64_t& node_id, const uint64_t& total_saled) {
   require_auth(_gstate.admin);

   CHECKC(node_id > 0, err::PARAM_ERROR, "invalid node_id" + to_string(node_id));
   CHECKC(total_saled > 0, err::PARAM_ERROR, "invalid total_saled" + to_string(total_saled));

   node_t node(node_id);
   CHECKC(_db.get(node), err::RECORD_NOT_FOUND, "node not found: " + to_string(node_id));

   node.total_saled = total_saled;
   node.update_time = current_time_point();
   _db.set(node);
}

/// @brief set node status only for admin
/// @param node_id - node id
/// @param status - node status (enable or disable)
void agpu::setnodestate(const uint64_t& node_id, const name& status) {
   require_auth(_gstate.admin);

   CHECKC(node_id > 0, err::PARAM_ERROR, "invalid node_id" + to_string(node_id));
   CHECKC(status == NodeStatus::ENABLE || status == NodeStatus::DISABLE, err::PARAM_ERROR, "invalid state" + status.to_string());

   node_t node(node_id);
   CHECKC(_db.get(node), err::RECORD_NOT_FOUND, "node not found: " + to_string(node_id));

   node.status      = status;
   node.update_time = current_time_point();
   _db.set(node);
}

/// @brief signup action
/// @param user - user account name
/// @param inviter - inviter account name
void agpu::signup(const name& user, const name& inviter) {
   CHECKC(has_auth(user) || has_auth(_gstate.admin), err::PARAM_ERROR, "missing authority");

   CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not found: " + user.to_string())
   CHECKC(is_account(inviter), err::ACCOUNT_INVALID, "inviter not found: " + inviter.to_string())
   CHECKC(user != inviter, err::PARAM_ERROR, "user and inviter is same")

   invite_t use(user);
   CHECKC(!_db.get(use), err::RECORD_FOUND, "user invite is exist: " + user.to_string());

   use.inviter      = inviter;
   use.invite_count = 0;
   use.create_time  = current_time_point();
   use.update_time  = current_time_point();
   _db.set(use);

   if (inviter != _gstate.bank) {
      user_mining_site_t::idx_t mining_site(ACPU_MINING, ACPU_MINING.value);
      auto                      site_itr = mining_site.find(inviter.value);
      CHECKC(site_itr != mining_site.end(), err::RECORD_NOT_FOUND, "invalid inviter");
      CHECKC(site_itr->account == inviter, err::PARAM_ERROR, "inviter not match");
      CHECKC(site_itr->level > 0, err::PARAM_ERROR, "invalid inviter level");

      invite_t invite(inviter);
      CHECKC(_db.get(invite), err::RECORD_NOT_FOUND, "inviter not exist: " + inviter.to_string());
      invite.invite_count += 1;
      invite.update_time = current_time_point();
      _db.set(invite);
   }
}

void agpu::signbind(const name& user, const name& inviter) {
   require_auth(_gstate.admin);

   CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not found: " + user.to_string())
   CHECKC(is_account(inviter), err::ACCOUNT_INVALID, "inviter not found: " + inviter.to_string())
   CHECKC(user != inviter, err::PARAM_ERROR, "user and inviter is same")

   invite_t use(user);
   CHECKC(!_db.get(use), err::RECORD_FOUND, "user invite is exist: " + user.to_string());

   use.inviter      = inviter;
   use.invite_count = 0;
   use.create_time  = current_time_point();
   use.update_time  = current_time_point();
   _db.set(use);

   if (inviter != _gstate.bank) {
      invite_t invite(inviter);
      if (!_db.get(invite)) {
         invite.inviter      = _gstate.bank;
         invite.invite_count = 1;
         invite.create_time  = current_time_point();
      } else {
         invite.invite_count += 1;
      }
      invite.update_time = current_time_point();
      _db.set(invite);
   }
}

/// @brief signedit action only for admin
void agpu::signedit(const name& user, const name& inviter) {
   require_auth(_gstate.admin);

   CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not found: " + user.to_string())
   CHECKC(is_account(inviter), err::ACCOUNT_INVALID, "inviter not found: " + inviter.to_string())
   CHECKC(user != inviter, err::PARAM_ERROR, "user and inviter is same")

   invite_t use(user);
   CHECKC(_db.get(use), err::RECORD_FOUND, "user invite not exist: " + user.to_string());
   name user_invite = use.inviter;

   use.inviter     = inviter;
   use.update_time = current_time_point();
   _db.set(use);

   if (user_invite != _gstate.bank) {
      invite_t old_invite(user_invite);
      CHECKC(_db.get(old_invite), err::RECORD_FOUND, "user old invite not exist: " + user_invite.to_string());
      CHECKC(user_invite != inviter, err::PARAM_ERROR, "user.inviter and inviter is same")

      old_invite.invite_count -= 1;
      old_invite.update_time = current_time_point();
      _db.set(old_invite);
   }

   if (inviter != _gstate.bank) {
      user_mining_site_t::idx_t mining_site(ACPU_MINING, ACPU_MINING.value);
      auto                      site_itr = mining_site.find(inviter.value);
      CHECKC(site_itr != mining_site.end(), err::RECORD_NOT_FOUND, "invalid inviter");
      CHECKC(site_itr->account == inviter, err::PARAM_ERROR, "inviter not match");
      CHECKC(site_itr->level > 0, err::PARAM_ERROR, "invalid inviter level");

      invite_t invite(inviter);
      CHECKC(_db.get(invite), err::RECORD_NOT_FOUND, "inviter not exist: " + inviter.to_string());
      invite.invite_count += 1;
      invite.update_time = current_time_point();
      _db.set(invite);
   }
}

/// @brief signdel action only for admin
void agpu::signdel(const name& user) {
   require_auth(_gstate.admin);

   CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not found: " + user.to_string())

   invite_t use(user);
   CHECKC(_db.get(use), err::RECORD_NOT_FOUND, "user invite is not exist: " + user.to_string());

   _db.del(use);
}

/// @brief buy node action
/// @param from - from account name
/// @param to - to account name
/// @param quantity - transfer quantity
/// @param memo - buy:node_id
void agpu::on_transfer(const name& from, const name& to, const asset& quantity, const string& memo) {
   if (from == _self || to != _self)
      return;

   CHECKC(is_account(from), err::ACCOUNT_INVALID, "from not found: " + from.to_string())
   CHECKC(is_account(to), err::ACCOUNT_INVALID, "to not found: " + to.to_string())
   CHECKC(quantity.is_valid() && quantity.amount > 0, err::QUANTITY_INVALID, "invalid quantity")

   auto params = split(memo, ":");
   CHECKC(params.size() >= 2, err::MEMO_FORMAT_ERROR, "invalid memo");
   auto action_name = name(params[0]);
   auto node_id     = uint64_t(atoi(params[1].data()));

   node_t node(node_id);
   CHECKC(_db.get(node), err::RECORD_NOT_FOUND, "node not found: " + to_string(node_id))

   switch (action_name.value) {
      case "buy"_n.value: {
         CHECKC(get_first_receiver() == _gstate.usdt_contract, err::PARAM_ERROR,
                "invalid usdt contract" + _gstate.usdt_contract.to_string());
         CHECKC(quantity.symbol == _gstate.usdt_symbol, err::SYMBOL_MISMATCH, "invalid usdt symbol: " + quantity.symbol.code().to_string());
         CHECKC(quantity.amount == node.price.amount, err::QUANTITY_INVALID, "invalid quantity: " + quantity.to_string());
         CHECKC(node.status == NodeStatus::ENABLE, err::PARAM_ERROR, "node not enable: " + to_string(node_id));
         CHECKC(node.start_time < current_time_point(), err::PARAM_ERROR, "node not start: " + to_string(node_id));
         CHECKC(node.total_saled + 1 <= node.max_sale, err::OVERSIZED, "node saled count exceeded: " + to_string(node.max_sale));

         TRANSFER(get_first_receiver(), _gstate.bank, quantity, memo);

         node.total_saled += 1;
         _db.set(node);

         _buy(node_id, from, quantity);
         break;
      }
      default: {
         CHECKC(false, err::MEMO_FORMAT_ERROR, "invalid action name")
         break;
      }
   }
}

/// @brief buy node helper function
/// @param node_id - node id
/// @param user - user account name
/// @param quantity - transfer quantity
void agpu::_buy(const uint64_t& node_id, const name& user, const asset& quantity) {
   invite_t invite(user);
   CHECKC(_db.get(invite), err::RECORD_NOT_FOUND, "user invite not found: " + user.to_string());

   uint64_t order_id = ++_gstate.order_id;
   order_t  order(order_id);
   CHECKC(!_db.get(user.value, order), err::RECORD_FOUND, "order found: " + to_string(order_id));

   order.node_id     = node_id;
   order.user        = user;
   order.inviter     = invite.inviter;
   order.price       = quantity;
   order.create_time = current_time_point();
   _db.set(user.value, order, false);

   node_total_t node_total(node_id);
   if (!_db.get(user.value, node_total)) {
      node_total.node_id     = node_id;
      node_total.total       = 1;
      node_total.create_time = current_time_point();
      node_total.update_time = current_time_point();
      _db.set(user.value, node_total, false);
   } else {
      node_total.total += 1;
      node_total.update_time = current_time_point();
      _db.set(user.value, node_total, true);
   }
}

/// @brief add order action only for admin
/// @param node_id - node id
/// @param user - user account name
/// @param quantity - transfer quantity
void agpu::addorder(const uint64_t& node_id, const name& user, const asset& quantity) {
   require_auth(_gstate.admin);

   CHECKC(node_id > 0, err::PARAM_ERROR, "invalid node_id" + to_string(node_id));
   CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not found: " + user.to_string());
   CHECKC(quantity.is_valid() && quantity.amount > 0, err::QUANTITY_INVALID, "invalid quantity")

   node_t node(node_id);
   CHECKC(_db.get(node), err::RECORD_NOT_FOUND, "node not found: " + to_string(node_id))

   CHECKC(quantity.symbol == _gstate.usdt_symbol, err::SYMBOL_MISMATCH, "invalid usdt symbol: " + quantity.symbol.code().to_string());
   CHECKC(quantity.amount == node.price.amount, err::QUANTITY_INVALID, "invalid quantity: " + quantity.to_string());
   CHECKC(node.status == NodeStatus::ENABLE, err::PARAM_ERROR, "node not enable: " + to_string(node_id));
   CHECKC(node.total_saled + 1 <= node.max_sale, err::OVERSIZED, "node saled count exceeded: " + to_string(node.max_sale));

   node.total_saled += 1;
   _db.set(node);

   _buy(node_id, user, quantity);
}

/// @brief delete order action only for admin
/// @param order_id - order id
/// @param user - user account name
void agpu::delorder(const uint64_t& order_id, const name& user) {
   require_auth(_gstate.admin);

   CHECKC(order_id > 0, err::PARAM_ERROR, "invalid order_id" + to_string(order_id));
   CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not found: " + user.to_string());

   order_t order(order_id);
   CHECKC(_db.get(user.value, order), err::RECORD_NOT_FOUND, "order not found: " + to_string(order_id))

   _db.del(user.value, order);

   invite_t invite(user);
   CHECKC(_db.get(invite), err::RECORD_NOT_FOUND, "user invite not found: " + user.to_string());

   uint64_t     node_id = order.node_id;
   node_total_t node_total(node_id);
   CHECKC(_db.get(user.value, node_total), err::RECORD_NOT_FOUND, "node total not found: " + to_string(node_id));

   node_total.total -= 1;
   node_total.update_time = current_time_point();
   _db.set(user.value, node_total, true);
}

} // namespace amax