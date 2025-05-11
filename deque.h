#include <array>
#include <iterator>
#include <vector>

template <typename T>
class Deque {
 private:
  std::vector<T*> pointer_;
  size_t size_;
  static const int kBlockSize_ = 32;

  struct Index {
    int pointer_index = 0;
    int inner_index = 1;
    Index() = default;

    Index(int pointer_index, int inner_index)
        : pointer_index(pointer_index), inner_index(inner_index) {}

    Index& operator+=(int number) {
      inner_index += number;
      pointer_index +=
          inner_index / kBlockSize_ -
          static_cast<int>(inner_index < 0 && inner_index % kBlockSize_ != 0);
      inner_index = ((inner_index % kBlockSize_) + kBlockSize_) % kBlockSize_;
      return *this;
    }

    Index& operator+=(Index other) {
      *this += other.inner_index + other.pointer_index * kBlockSize_;
      return *this;
    }

    Index& operator-=(int number) {
      *this += -number;
      return *this;
    }

    Index& operator-=(Index other) {
      *this += {-other.pointer_index, -other.inner_index};
      return *this;
    }

    Index& operator++() {
      *this += 1;
      return *this;
    }

    Index& operator--() {
      *this += -1;
      return *this;
    }

    Index operator++(int) {
      Index copy = *this;
      ++*this;
      return copy;
    }

    Index operator--(int) {
      Index copy = *this;
      --*this;
      return copy;
    }

    Index operator+(int number) const {
      Index result = *this;
      result += number;
      return result;
    }

    Index operator-(int number) const {
      Index result = *this;
      result -= number;
      return result;
    }

    Index operator+(Index other) const {
      Index result = *this;
      result += other;
      return result;
    }

    Index operator-(Index other) const {
      Index result = *this;
      result -= other;
      return result;
    }

    bool operator==(Index other) {
      return pointer_index == other.pointer_index &&
             inner_index == other.inner_index;
    }

    bool operator!=(Index other) { return !(*this == other); }

    bool operator>(Index other) {
      return pointer_index > other.pointer_index ||
             (pointer_index == other.pointer_index &&
              inner_index > other.inner_index);
    }

    bool operator<(Index other) { return other > *this; }

    bool operator>=(Index other) { return !(*this < other); }

    bool operator<=(Index other) { return !(*this > other); }

    explicit operator int() {
      return pointer_index * kBlockSize_ + inner_index;
    }
  };

  Index begin_;

  void IncreaseCap() {
    size_t size_pointer = pointer_.size();
    pointer_.resize(3 * size_pointer);
    for (size_t i = 0; i < size_pointer; ++i) {
      pointer_[i + size_pointer] = pointer_[i];
    }
    for (size_t i = 2 * size_pointer; i < 3 * size_pointer; ++i) {
      pointer_[i] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    }
    for (size_t i = 0; i < size_pointer; ++i) {
      pointer_[i] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    }
    begin_.pointer_index += size_pointer;
  }

  template <bool IsConst>
  class BasicIterator {
   private:
    using vector_iterator_type =
        std::conditional_t<IsConst, typename std::vector<T*>::const_iterator,
                           typename std::vector<T*>::iterator>;
    vector_iterator_type iter_;
    Index index_;
    friend class Deque<T>;

   public:
    using difference_type = int;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using value_type = T;
    using iterator_category = std::random_access_iterator_tag;
    operator BasicIterator<true>() const { return {iter_, index_}; }

    BasicIterator() = default;

    BasicIterator(vector_iterator_type iter, Index index)
        : iter_(iter), index_(index) {}

    BasicIterator& operator+=(int number) {
      Index copy = index_;
      index_ += number;
      int help = index_.pointer_index - copy.pointer_index;
      iter_ += help;
      return *this;
    }

    BasicIterator& operator-=(int number) {
      *this += -number;
      return *this;
    }

    BasicIterator operator+(int number) const {
      BasicIterator copy_iterator = *this;
      copy_iterator += number;
      return copy_iterator;
    }

    BasicIterator operator-(int number) const {
      BasicIterator copy_iterator = *this;
      copy_iterator -= number;
      return copy_iterator;
    }

    difference_type operator-(const BasicIterator& other) {
      Index help = index_ - other.index_;
      return help.pointer_index * kBlockSize_ + help.inner_index;
    }

    reference operator*() { return (*iter_)[index_.inner_index]; }

    pointer operator->() const { return &((*iter_)[index_.inner_index]); }

    BasicIterator& operator++() {
      *this += 1;
      return *this;
    }

    BasicIterator& operator--() {
      *this -= 1;
      return *this;
    }

    BasicIterator operator++(int) {
      BasicIterator copy = *this;
      ++*this;
      return copy;
    }

    BasicIterator operator--(int) {
      BasicIterator copy = *this;
      --*this;
      return copy;
    }

    bool operator==(BasicIterator other) {
      return iter_ == other.iter_ && index_ == other.index_;
    }

    bool operator>(BasicIterator other) { return index_ > other.index_; }

    bool operator<(BasicIterator other) { return other > *this; }

    bool operator>=(BasicIterator other) { return !(*this < other); }

    bool operator<=(BasicIterator other) { return !(*this > other); }

    bool operator!=(BasicIterator other) { return !(*this == other); }
  };

  void Free(bool need_to_throw) {
    if (size_ == 0) {
      return;
    }
    for (size_t i = 0; i < pointer_.size(); ++i) {
      for (size_t j = (i == 0) ? 1 : 0; j < kBlockSize_; ++j) {
        (pointer_[i] + j)->~T();
        --size_;
        if (size_ == 0) {
          delete[] reinterpret_cast<char*>(pointer_[i]);
          if (need_to_throw) {
            throw;
          }
          return;
        }
      }
      delete[] reinterpret_cast<char*>(pointer_[i]);
    }
  }

 public:
  using iterator = BasicIterator<false>;
  using const_iterator = BasicIterator<true>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  iterator begin() {
    return iterator(pointer_.begin() + begin_.pointer_index, begin_);
  }

  const_iterator begin() const { return cbegin(); }

  iterator end() {
    iterator result = begin();
    result += size_;
    return result;
  }

  const_iterator end() const { return cend(); }

  const_iterator cbegin() const {
    return const_iterator(pointer_.begin() + begin_.pointer_index, begin_);
  }

  const_iterator cend() const {
    const_iterator result = cbegin();
    result += size_;
    return result;
  }

  reverse_iterator rbegin() { return reverse_iterator(end()); }

  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator rbegin() const { return crbegin(); }

  const_reverse_iterator rend() const { return crend(); }

  /*Deque(int size, const T& value = T()) : size_(size) {
    pointer_.resize(size / kBlockSize_ + 1);
    for (T*& pointer : pointer_) {
      pointer = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
      for (int i = 0; i < kBlockSize_; ++i) {
        new (pointer + i) T(value);
      }
    }
  }*/

  explicit Deque(size_t size, const T& value = T()) {
    size_ = 0;
    pointer_.resize(size / kBlockSize_ + 1);
    try {
      for (size_t i = 0; i < pointer_.size(); ++i) {
        pointer_[i] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
        for (size_t j = (i == 0) ? 1 : 0; j < kBlockSize_; ++j) {
          new (pointer_[i] + j) T(value);
          ++size_;
          if (size_ == size) {
            return;
          }
        }
      }
    } catch (...) {
      while (size_ > 0) {
        pop_back();
      }
      for (size_t i = 0; i < pointer_.size(); ++i) {
        delete[] reinterpret_cast<char*>(pointer_[i]);
      }
      throw;
    }
  }

  Deque& operator=(const Deque& other) {
    if (this == &other) {
      return *this;
    }
    try {
      Deque deque_to_swap(other);
      std::swap(deque_to_swap.size_, size_);
      std::swap(deque_to_swap.pointer_, pointer_);
      std::swap(deque_to_swap.begin_, begin_);
      return *this;
    } catch (...) {
      throw;
    }
  }

  Deque() : size_(0) {
    pointer_.resize(1);
    pointer_[0] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
  }

  T& operator[](size_t i) {
    Index index = begin_ + i;
    return pointer_[index.pointer_index][index.inner_index];
  }

  const T& operator[](size_t i) const {
    Index index = begin_ + i;
    return pointer_[index.pointer_index][index.inner_index];
  }

  T& at(size_t i) {
    if (i >= size_) {
      throw std::out_of_range("invalid index");
    }
    Index index = begin_ + i;
    return pointer_[index.pointer_index][index.inner_index];
  }

  const T& at(size_t i) const {
    if (i >= size_) {
      throw std::out_of_range("invalid index");
    }
    Index index = begin_ + i;
    return pointer_[index.pointer_index][index.inner_index];
  }

  Deque(const Deque& other) {
    size_ = 0;
    pointer_.resize(other.size() / kBlockSize_ + 1);
    for (T*& pointer : pointer_) {
      pointer = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    }
    try {
      for (size_t i = 0; i < other.size(); ++i) {
        Index index = begin_ + i;
        new (pointer_[index.pointer_index] + index.inner_index) T(other[i]);
        ++size_;
      }
    } catch (...) {
      while (size_ > 0) {
        pop_back();
      }
      for (size_t i = 0; i < pointer_.size(); ++i) {
        delete[] reinterpret_cast<char*>(pointer_[i]);
      }
      throw;
    }
  }

  void push_back(const T& value) {
    Index index_help = begin_ + size_;
    try {
      new (pointer_[index_help.pointer_index] + index_help.inner_index) T(value);
    } catch (...) {
      throw;
    }
    ++index_help;
    if (index_help.pointer_index >= static_cast<int>(pointer_.size())) {
      IncreaseCap();
    }
    ++size_;
  }

  void push_front(const T& value) {
    Index index = begin_ - 1;
    try {
      new (pointer_[index.pointer_index] + index.inner_index) T(value);
    } catch (...) {
      throw;
    }
    begin_ = index;
    if (begin_.pointer_index == 0 && begin_.inner_index == 0) {
      IncreaseCap();
    }
    ++size_;
  }

  void pop_front() {
    (pointer_[begin_.pointer_index] + begin_.inner_index)->~T();
    begin_ += 1;
    --size_;
  }

  void pop_back() {
    Index help = begin_ + (size_ - 1);
    (pointer_[help.pointer_index] + help.inner_index)->~T();
    --size_;
  }

  size_t size() const { return size_; }

  void insert(iterator iter, const T& value) {
    int index(iter.index_ - begin_);
    push_back(value);
    for (int i = static_cast<int>(size_ - 1); i > index; --i) {
      std::swap((*this)[i], (*this)[i - 1]);
    }
  }

  void erase(iterator iter) {
    int index(iter.index_ - begin_);
    for (int i = index; i + 1 < static_cast<int>(size_); ++i) {
      std::swap((*this)[i], (*this)[i + 1]);
    }
    pop_back();
  }

  ~Deque() {
    while (size_ > 0) {
      pop_back();
    }
    for (size_t i = 0; i < pointer_.size(); ++i) {
      delete[] reinterpret_cast<char*>(pointer_[i]);
    }
  }
};
