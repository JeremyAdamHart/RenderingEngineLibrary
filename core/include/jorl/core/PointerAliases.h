#pragma once
#include <memory>

namespace renderlib {

template<typename T>
using sptr = std::shared_ptr<T>;

template<typename T, class... Args>
sptr<T> make(Args&&... args) { return std::make_shared<T>(args...); }

}