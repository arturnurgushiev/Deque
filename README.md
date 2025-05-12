# Deque<T> Implementation

A simplified STL-like deque with iterators, exceptions safety, and O(1) operations.

## Features
- **Constructors**: 
  - Default, copy, from `int`, from `(int, const T&)`.
  - Assignment operator.
- **Element Access**:
  - `operator[]` (no bounds checking), `at()` (throws `std::out_of_range`).
- **Modifiers**:
  - `push_back()`, `pop_back()`, `push_front()`, `pop_front()` (amortized O(1)).
  - `insert()`, `erase()` (linear time).
- **Iterators**:
  - Random access iterators with arithmetic operations.
  - `const_iterator`, `reverse_iterator` support.
- **Exception Safety**:
  - All methods (except `insert`/`erase`) restore state on exceptions.

## Example
```cpp
#include "deque.h"

Deque<std::string> dq(3, "Hello");
dq.push_front("World");
std::cout << dq.at(0); // "World"
