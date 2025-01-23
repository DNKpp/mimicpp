//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_CONTAINER_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_CONTAINER_TYPE_PRINTER_HPP

#include "mimic++/printing/TypePrinter.hpp"

#include <concepts>
#include <cstddef>
#include <map>
#include <memory>
#include <memory_resource>
#include <queue>
#include <set>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimicpp::printing::detail
{
    // just do it this way, so we do not have to pull ranges in
    template <typename T>
    concept container = requires {
        typename T::value_type;
        typename T::size_type;
        typename T::iterator;
    };

    // used for array, span and the like
    // should also handle c++26s inplace_vector
    // see: https://en.cppreference.com/w/cpp/container/inplace_vector
    template <template <typename, auto> typename Template, typename T, auto n>
        requires container<Template<T, n>>
    struct template_type_printer<Template<T, n>>
        : public basic_template_type_printer<
              decltype([] {
                  return format::format(
                      "{}<{}, {}>",
                      pretty_template_name<Template<T, n>>(),
                      mimicpp::print_type<T>(),
                      n);
              })>
    {
    };

    template <typename T>
    struct template_type_printer<std::span<T>>
        : public basic_template_type_printer<
              decltype([] { return format::format("std::span<{}>", mimicpp::print_type<T>()); })>
    {
    };

    template <typename T>
    StringStreamT print_std_container_prefix(auto&& baseName, const bool isPmr)
    {
        StringStreamT out{};
        out << "std::" << (isPmr ? "pmr::" : "");
        std::ranges::copy(baseName, std::ostreambuf_iterator{out});
        out << '<';
        mimicpp::print_type<T>(std::ostreambuf_iterator{out});

        return out;
    }

    template <typename Allocator>
    StringT print_std_container_suffix(auto& out, const bool isDefaultAllocator)
    {
        if (!isDefaultAllocator)
        {
            out << ", ";
            mimicpp::print_type<Allocator>(std::ostreambuf_iterator{out});
        }

        out << '>';

        return std::move(out).str();
    }

    template <typename T, typename Allocator, CharT... base>
    struct std_sequence_container_printer_fn
    {
        [[nodiscard]]
        StringT operator()() const
        {
            constexpr bool isPmr = std::same_as<std::pmr::polymorphic_allocator<T>, Allocator>;
            constexpr bool isDefaultAllocator = isPmr || std::same_as<std::allocator<T>, Allocator>;

            StringStreamT out = print_std_container_prefix<T>(std::array{base...}, isPmr);
            return print_std_container_suffix<Allocator>(out, isDefaultAllocator);
        }
    };

    template <typename T, typename Allocator>
    struct template_type_printer<std::vector<T, Allocator>>
        : basic_template_type_printer<
              std_sequence_container_printer_fn<T, Allocator, 'v', 'e', 'c', 't', 'o', 'r'>>
    {
    };

    template <typename T, typename Allocator>
    struct template_type_printer<std::deque<T, Allocator>>
        : basic_template_type_printer<
              std_sequence_container_printer_fn<T, Allocator, 'd', 'e', 'q', 'u', 'e'>>
    {
    };

    template <typename T, typename Allocator>
    struct template_type_printer<std::list<T, Allocator>>
        : basic_template_type_printer<
              std_sequence_container_printer_fn<T, Allocator, 'l', 'i', 's', 't'>>
    {
    };

    template <typename T, typename Allocator>
    struct template_type_printer<std::forward_list<T, Allocator>>
        : basic_template_type_printer<
              std_sequence_container_printer_fn<T, Allocator, 'f', 'o', 'r', 'w', 'a', 'r', 'd', '_', 'l', 'i', 's', 't'>>
    {
    };

    template <typename T, typename Compare, typename Allocator, CharT... base>
    struct std_set_printer_fn
    {
        [[nodiscard]]
        StringT operator()() const
        {
            constexpr bool isPmr = std::same_as<std::pmr::polymorphic_allocator<T>, Allocator>;
            constexpr bool isDefaultAllocator = isPmr || std::same_as<std::allocator<T>, Allocator>;

            StringStreamT out = print_std_container_prefix<T>(std::array{base...}, isPmr);

            if constexpr (!std::same_as<std::less<T>, Compare> || !isDefaultAllocator)
            {
                out << ", ";
                mimicpp::print_type<Compare>(std::ostreambuf_iterator{out});
            }

            return print_std_container_suffix<Allocator>(out, isDefaultAllocator);
        }
    };

    template <typename T, typename Compare, typename Allocator>
    struct template_type_printer<std::set<T, Compare, Allocator>>
        : basic_template_type_printer<
              std_set_printer_fn<T, Compare, Allocator, 's', 'e', 't'>>
    {
    };

    template <typename T, typename Compare, typename Allocator>
    struct template_type_printer<std::multiset<T, Compare, Allocator>>
        : basic_template_type_printer<
              std_set_printer_fn<T, Compare, Allocator, 'm', 'u', 'l', 't', 'i', 's', 'e', 't'>>
    {
    };

    template <typename T, typename Hash, typename Compare, typename Allocator, CharT... base>
    struct std_unordered_set_printer_fn
    {
        [[nodiscard]]
        StringT operator()() const
        {
            constexpr bool isPmr = std::same_as<std::pmr::polymorphic_allocator<T>, Allocator>;
            constexpr bool isDefaultAllocator = isPmr || std::same_as<std::allocator<T>, Allocator>;
            constexpr bool printCompare = !std::same_as<std::equal_to<T>, Compare>
                                       || !isDefaultAllocator;
            constexpr bool printHash = !std::same_as<std::hash<T>, Hash>
                                    || printCompare;

            StringStreamT out = print_std_container_prefix<T>(std::array{base...}, isPmr);

            if constexpr (printHash)
            {
                out << ", ";
                mimicpp::print_type<Hash>(std::ostreambuf_iterator{out});
            }

            if constexpr (printCompare)
            {
                out << ", ";
                mimicpp::print_type<Compare>(std::ostreambuf_iterator{out});
            }

            return print_std_container_suffix<Allocator>(out, isDefaultAllocator);
        }
    };

    template <typename T, typename Hash, typename Compare, typename Allocator>
    struct template_type_printer<std::unordered_set<T, Hash, Compare, Allocator>>
        : basic_template_type_printer<
              std_unordered_set_printer_fn<T, Hash, Compare, Allocator, 'u', 'n', 'o', 'r', 'd', 'e', 'r', 'e', 'd', '_', 's', 'e', 't'>>
    {
    };

    template <typename T, typename Hash, typename Compare, typename Allocator>
    struct template_type_printer<std::unordered_multiset<T, Hash, Compare, Allocator>>
        : basic_template_type_printer<
              std_unordered_set_printer_fn<T, Hash, Compare, Allocator, 'u', 'n', 'o', 'r', 'd', 'e', 'r', 'e', 'd', '_', 'm', 'u', 'l', 't', 'i', 's', 'e', 't'>>
    {
    };

    template <typename T, CharT... base>
    struct std_default_container_adapter_printer_fn
    {
        [[nodiscard]]
        StringT operator()() const
        {
            StringStreamT out{};
            out << "std::";
            std::ranges::copy(std::array{base...}, std::ostreambuf_iterator{out});
            out << '<';
            mimicpp::print_type<T>(std::ostreambuf_iterator{out});
            out << '>';

            return std::move(out).str();
        }
    };

    template <typename T>
    struct template_type_printer<std::queue<T>>
        : basic_template_type_printer<
              std_default_container_adapter_printer_fn<T, 'q', 'u', 'e', 'u', 'e'>>
    {
    };

    template <typename T>
    struct template_type_printer<std::stack<T>>
        : basic_template_type_printer<
              std_default_container_adapter_printer_fn<T, 's', 't', 'a', 'c', 'k'>>
    {
    };
}

#endif
