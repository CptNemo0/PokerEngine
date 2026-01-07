#ifndef COMMON_UTILITY_ENUM_INDEXABLE_ARRAY_H_
#define COMMON_UTILITY_ENUM_INDEXABLE_ARRAY_H_

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <vcruntime_typeinfo.h>

namespace common::utility {

// Enum Indexable Array is intended to be used as a mapping between an enum
// class and other type. It's main strength is std::array as an underlying type
// - no dynamic allocation and low cache pollution.
template <class EnumType, class StoredType, std::size_t N>
  requires std::is_enum_v<EnumType> &&
           std::unsigned_integral<std::underlying_type_t<EnumType>>
class enum_indexable_array {
  public:
    using underlying_type = std::underlying_type_t<EnumType>;
    using size_type = std::array<underlying_type, N>::size_type;

    constexpr const StoredType& operator[](size_type index) const {
      return data_[index];
    }

    constexpr StoredType& operator[](size_type index) {
      return data_[index];
    }

    constexpr const StoredType& operator[](EnumType index) const {
      return data_[std::to_underlying(index)];
    }

    constexpr StoredType& operator[](EnumType index) {
      return data_[std::to_underlying(index)];
    }

    constexpr const StoredType& front() const {
      return data_[0];
    }

    constexpr StoredType& front() {
      return data_.front();
    }

    constexpr const StoredType& back() const {
      return data_.back();
    }

    constexpr StoredType& back() {
      return data_.back();
    }

    constexpr size_type size() const {
      return data_.size();
    }

    constexpr const StoredType* data() const {
      return data_.data();
    }

    constexpr StoredType* data() {
      return data_.data();
    }

    constexpr bool empty() {
      return data_.data();
    }

    template <class Pred>
    std::optional<EnumType> return_enum_for(Pred pred) const {
      static_assert(static_cast<size_type>(EnumType::kMemberCount) == N,
                    "Enum contains invalid number of members");
      for (auto i = 0uz; i < size(); i++) {
        if (pred(data_[i])) {
          return static_cast<EnumType>(i);
        }
      }
      return std::nullopt;
    }

  private:
    std::array<StoredType, N> data_;
};

} // namespace common::utility

#endif // !COMMON_UTILITY_ENUM_INDEXABLE_ARRAY_H_
