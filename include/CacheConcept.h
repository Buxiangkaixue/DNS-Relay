#pragma once

#include <concepts>

template <typename Cache, typename Key, typename Value>
concept CacheConcept =
    requires(Cache cache, const Key &key, const Value &value) {
      { cache.get(key) } -> std::convertible_to<Value>;
      { cache.put(key, value) };
    };
