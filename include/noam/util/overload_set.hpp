#pragma once

namespace noam {
/**
 * @brief overload_set<F...> is a functor whose overload set is the union of the
 * overload sets of { F::operator()... }
 *
 * @tparam Functions Defines the types of the functors used to construct this
 * overload set
 */
template <class... Functions>
struct overload_set : Functions... {
    using Functions::operator()...;
};
template <class... Functions>
overload_set(Functions...) -> overload_set<Functions...>;
} // namespace noam
