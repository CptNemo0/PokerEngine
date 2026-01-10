#ifndef SERVER_UTILITY_SORTED_VECTOR_H
#define SERVER_UTILITY_SORTED_VECTOR_H

#include <algorithm>
#include <vector>

namespace common::utility {

template <class T>
class sorted_vector {
  public:
    using container_type = std::vector<T>;
    using size_type = container_type::size_type;
    using reference = container_type&;
    using const_reference = const container_type&;

    sorted_vector() {
      data_.reserve(10);
    }

    void insert(const T& value) {
      const auto position = std::upper_bound(data_.begin(), data_.end(), value);
      data_.insert(position, value);
    }

    reference underlying() {
      return data_;
    }

    const_reference underlying() const {
      return data_;
    }

    constexpr const T* data() const {
      return data_.data();
    }

    void reserve(size_type new_capacity) {
      data_.reserve(new_capacity);
    }

    size_type size() const {
      return data_.size();
    }

    void clear() {
      data_.clear();
    }

  private:
    container_type data_;
};

} // namespace common::utility

#endif // !SERVER_UTILITY_SORTED_VECTOR_H
