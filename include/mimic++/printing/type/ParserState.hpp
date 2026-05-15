//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PARSER_STATE_HPP
#define MIMICPP_PRINTING_TYPE_PARSER_STATE_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/Lexer.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/CopyableBox.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <array>
    #include <functional>
    #include <optional>
    #include <utility>
    #include <variant>
    #include <vector>
#endif

namespace mimicpp::printing::type::parsing::v2::state
{
    struct TypeId;

    template <typename T>
    class Recursive
    {
    public:
        constexpr ~Recursive();
        constexpr Recursive(Recursive const&);
        constexpr Recursive& operator=(Recursive const&);
        constexpr Recursive(Recursive&&) noexcept;
        constexpr Recursive& operator=(Recursive&&) noexcept;

        [[nodiscard]]
        explicit(false) constexpr Recursive(T value)
            : m_inner{std::move(value)}
        {
        }

        [[nodiscard]]
        constexpr T& operator*() noexcept
        {
            return *m_inner;
        }

        [[nodiscard]]
        constexpr T const& operator*() const noexcept
        {
            return *m_inner;
        }

        [[nodiscard]]
        constexpr bool operator==(Recursive const&) const;

    private:
        util::CopyableBox<T> m_inner;
    };

    template <typename T>
    Recursive(T) -> Recursive<T>;

    template <typename T>
    Recursive(Recursive<T>) -> Recursive<T>;

    // see: https://eel.is/c++draft/class.pre#nt:class-key
    enum ClassKey
    {
        id_class = 0,
        id_struct,
        id_union
    };

    // see: https://eel.is/c++draft/dcl.enum#nt:enum-key
    enum EnumKey
    {
        id_enum = 0,
        id_enum_class,
        id_enum_struct
    };

    enum class CVQualifier
    {
        id_const = 0,
        id_volatile
    };

    struct CVQualifierSeq
    {
        bool isConst{false};
        bool isVolatile{false};

        [[nodiscard]]
        bool operator==(CVQualifierSeq const&) const = default;

        [[nodiscard]]
        constexpr bool apply(CVQualifier const qualifier) noexcept
        {
            switch (qualifier)
            {
            case CVQualifier::id_const:
                return !std::exchange(isConst, true);

            case CVQualifier::id_volatile:
                return !std::exchange(isVolatile, true);

            default:
                util::unreachable();
            }
        }
    };

    enum RefQualifier
    {
        id_ref = 0,
        id_refref
    };

    using ConstantExpression = lexing::literal;

    struct Identifier
    {
        std::string_view content{};
        bool isSynthetic{false};

        [[nodiscard]]
        bool operator==(Identifier const&) const = default;
    };

    struct FunctionDeclarator
    {
        std::vector<Recursive<TypeId>> params{};
        CVQualifierSeq qualifiers{};
        std::optional<RefQualifier> refQualifier{};
        bool isNoexcept{false};

        [[nodiscard]]
        bool operator==(FunctionDeclarator const&) const = default;
    };

    using TemplateArgument = std::variant<
        ConstantExpression,
        Recursive<TypeId>>;

    using TemplateArgumentList = std::vector<TemplateArgument>;

    struct OperatorFunctionId
    {
        using Symbol = std::variant<
            lexing::operator_or_punctuator,
            std::pair<lexing::keyword, bool /*isArray*/>,
            std::array<lexing::operator_or_punctuator, 2u>>;
        Symbol symbol;

        [[nodiscard]]
        constexpr bool is_call() const noexcept
        {
            if (auto const* const doubleOp = std::get_if<std::array<lexing::operator_or_punctuator, 2u>>(&symbol))
            {
                return lexing::operator_or_punctuator{"("} == doubleOp->front()
                    && lexing::operator_or_punctuator{")"} == doubleOp->back();
            }

            return false;
        }

        [[nodiscard]]
        bool operator==(OperatorFunctionId const&) const = default;
    };

    struct ConversionFunctionId
    {
        Recursive<TypeId> target;

        [[nodiscard]]
        bool operator==(ConversionFunctionId const&) const = default;
    };

    struct LambdaFunctionId
    {
        std::string_view content;
        bool isMutable{false};

        [[nodiscard]]
        bool operator==(LambdaFunctionId const&) const = default;
    };

    struct DestructorFunctionId
    {
        Identifier name;

        [[nodiscard]]
        bool operator==(DestructorFunctionId const&) const = default;
    };

    struct UnqualifiedId
    {
        using Name = std::variant<
            Identifier,
            OperatorFunctionId,
            ConversionFunctionId,
            LambdaFunctionId,
            DestructorFunctionId>;
        Name name{};
        std::optional<TemplateArgumentList> templateArgs{};
        std::optional<FunctionDeclarator> functionDeclarator{};

        [[nodiscard]]
        bool operator==(UnqualifiedId const&) const = default;
    };

    // This models more or less: https://eel.is/c++draft/expr.prim.id.qual#nt:nested-name-specifier
    struct ScopeSequence
    {
        bool explicitRoot{};
        std::vector<UnqualifiedId> scopes{};

        [[nodiscard]]
        bool operator==(ScopeSequence const&) const = default;
    };

    using TypeKey = std::variant<ClassKey, EnumKey>;

    // see: https://eel.is/c++draft/expr.prim.id.qual#nt:qualified-id
    struct QualifiedId
    {
        ScopeSequence scopes{};
        UnqualifiedId identifier{};
        std::optional<TypeKey> typeKey{};

        [[nodiscard]]
        bool operator==(QualifiedId const&) const = default;
    };

    struct ArrayDeclarator
    {
        std::optional<ConstantExpression> size{};

        [[nodiscard]]
        bool operator==(ArrayDeclarator const&) const = default;
    };

    struct CallConvention
    {
        lexing::identifier name{};

        [[nodiscard]]
        bool operator==(CallConvention const&) const = default;
    };

    struct PointerDeclarator
    {
        std::optional<CVQualifierSeq> qualifiers{};
        std::optional<ScopeSequence> scopes{};
        std::optional<CallConvention> callConvention{};

        [[nodiscard]]
        bool operator==(PointerDeclarator const&) const = default;
    };

    struct ReferenceDeclarator
    {
        RefQualifier qualifier{};

        [[nodiscard]]
        bool operator==(ReferenceDeclarator const&) const = default;
    };

    using PtrOperator = std::variant<
        ReferenceDeclarator,
        PointerDeclarator>;

    struct AbstractDeclarator
    {
        struct Layer
        {
            std::vector<PtrOperator> decorations{};
            std::optional<Recursive<Layer>> nested{};
            std::optional<FunctionDeclarator> function{};
            std::vector<ArrayDeclarator> arrays{};

            [[nodiscard]]
            bool operator==(Layer const&) const = default;
        };

        Layer root{};

        [[nodiscard]]
        bool operator==(AbstractDeclarator const&) const = default;
    };

    struct BuiltinType
    {
        std::optional<lexing::keyword> base{};

        enum class SizeSpec : std::int8_t
        {
            id_short = 0,
            id_long,
            id_longlong
        };

        std::optional<SizeSpec> sizeSpec{};

        enum class SignedSpec : std::int8_t
        {
            id_signed = 0,
            id_unsigned
        };

        std::optional<SignedSpec> signedSpec{};

        [[nodiscard]]
        MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES bool try_apply(lexing::keyword const& keyword)
        {
            return try_apply_size_spec(keyword)
                || try_apply_signed_spec(keyword)
                || try_apply_base(keyword);
        }

        [[nodiscard]]
        bool operator==(BuiltinType const&) const = default;

    private:
        [[nodiscard]]
        constexpr bool try_apply_base(lexing::keyword const& keyword) noexcept
        {
            // Todo: verify correct type keywords
            if (!base)
            {
                base = keyword;
                return true;
            }

            return false;
        }

        [[nodiscard]]
        MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES bool try_apply_signed_spec(lexing::keyword const& keyword) noexcept
        {
            std::optional const spec = std::invoke([&]() -> std::optional<SignedSpec> {
                if (lexing::keyword{"unsigned"} == keyword)
                {
                    return SignedSpec::id_unsigned;
                }

                if (lexing::keyword{"signed"} == keyword)
                {
                    return SignedSpec::id_signed;
                }

                return std::nullopt;
            });

            if (spec
                && !signedSpec)
            {
                signedSpec = spec;
                return true;
            }

            return false;
        }

        [[nodiscard]]
        MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES bool try_apply_size_spec(lexing::keyword const& keyword) noexcept
        {
            std::optional const spec = std::invoke([&]() -> std::optional<SizeSpec> {
                if (lexing::keyword{"long"} == keyword)
                {
                    return SizeSpec::id_long;
                }

                if (lexing::keyword{"short"} == keyword)
                {
                    return SizeSpec::id_short;
                }

                // This is a msvc symbol
                if (lexing::keyword{"__int64"} == keyword)
                {
                    return SizeSpec::id_longlong;
                }

                return std::nullopt;
            });

            if (spec)
            {
                if (!sizeSpec)
                {
                    sizeSpec = spec;
                    return true;
                }

                if (*sizeSpec == SizeSpec::id_long
                    && *spec == SizeSpec::id_long)
                {
                    sizeSpec = SizeSpec::id_longlong;
                    return true;
                }
            }

            return false;
        }
    };

    using BaseType = std::variant<
        QualifiedId,
        BuiltinType>;

    struct TypeId
    {
        CVQualifierSeq qualifications{};
        BaseType base;
        AbstractDeclarator declarator{};

        [[nodiscard]]
        bool operator==(TypeId const&) const = default;
    };

    struct FunctionId
    {
        std::optional<TypeId> returnType{};
        std::optional<CallConvention> callConvention{};
        QualifiedId identifier{};

        [[nodiscard]]
        bool operator==(FunctionId const&) const = default;
    };

    template <typename T>
    constexpr Recursive<T>::~Recursive() = default;
    template <typename T>
    constexpr Recursive<T>::Recursive(Recursive const&) = default;
    template <typename T>
    constexpr Recursive<T>& Recursive<T>::operator=(Recursive const&) = default;
    template <typename T>
    constexpr Recursive<T>::Recursive(Recursive&&) noexcept = default;
    template <typename T>
    constexpr Recursive<T>& Recursive<T>::operator=(Recursive&&) noexcept = default;
    template <typename T>
    constexpr bool Recursive<T>::operator==(Recursive const&) const = default;
}

#endif
