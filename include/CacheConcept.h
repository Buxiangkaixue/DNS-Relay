//
// Created by stellaura on 03/07/24.
//

#ifndef DNS_RELAY_CACHECONCEPT_H
#define DNS_RELAY_CACHECONCEPT_H

#include <concepts>

template <typename Cache, typename Key, typename Value>
concept CacheConcept =
    requires(Cache cache, const Key &key, const Value &value) {
      { cache.get(key) } -> std::convertible_to<Value>;
      { cache.put(key, value) };
    };

#endif // DNS_RELAY_CACHECONCEPT_H
