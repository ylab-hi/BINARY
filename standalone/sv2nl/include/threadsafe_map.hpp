//
// Created by li002252 on 9/2/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_INCLUDE_THREADSAFE_MAP_HPP_
#define BUILDALL_STANDALONE_SV2NL_INCLUDE_THREADSAFE_MAP_HPP_

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
namespace sv2nl {

  template <typename Key, typename Value> class ThreadSafeMap {
  public:
    using key_type = Key;
    using value_type = Value;
    using iterator_type = typename std::unordered_map<key_type, value_type>::iterator;

    ThreadSafeMap() = default;

    ThreadSafeMap(ThreadSafeMap const& other) = delete;
    ThreadSafeMap& operator=(ThreadSafeMap const& other) = delete;

    [[nodiscard]] bool find(key_type const& key, value_type& value) const {
      std::shared_lock<std::shared_mutex> lock(mutex_);
      auto it = map_.find(key);
      if (it == map_.end()) {
        return false;
      }
      value = it->second;
      return true;
    }

    void insert(key_type const& key, value_type const& value) {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      map_.insert({key, value});
    }

  private:
    std::unordered_map<key_type, value_type> map_;
    mutable std::shared_mutex mutex_;
  };

}  // namespace sv2nl

#endif  // BUILDALL_STANDALONE_SV2NL_INCLUDE_THREADSAFE_MAP_HPP_
