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
        : public util::CopyableBox<T>
    {
    public:
        using util::CopyableBox<T>::CopyableBox;

        constexpr ~Recursive();
    };

    template <typename T>
    Recursive(T) -> Recursive<T>;

    template <typename T>
    Recursive(Recursive<T>) -> Recursive<T>;

    enum ClassKey
    {
        id_class = 0,
        id_struct,
        id_union
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
        constexpr bool operator==(CVQualifierSeq const&) const;

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
        constexpr bool operator==(Identifier const&) const;
    };

    struct FunctionDeclarator
    {
        std::vector<Recursive<TypeId>> params{};
        CVQualifierSeq qualifiers{};
        std::optional<RefQualifier> refQualifier{};
        bool isNoexcept{false};

        [[nodiscard]]
        constexpr bool operator==(FunctionDeclarator const&) const;
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
        constexpr bool operator==(OperatorFunctionId const&) const;
    };

    struct ConversionFunctionId
    {
        Recursive<TypeId> target;

        [[nodiscard]]
        constexpr bool operator==(ConversionFunctionId const&) const;
    };

    struct DestructorFunctionId
    {
        Identifier name;

        [[nodiscard]]
        constexpr bool operator==(DestructorFunctionId const&) const;
    };

    struct UnqualifiedId
    {
        using Name = std::variant<
            Identifier,
            OperatorFunctionId,
            ConversionFunctionId,
            DestructorFunctionId>;
        Name name{};
        std::optional<TemplateArgumentList> templateArgs{};
        std::optional<FunctionDeclarator> functionDeclarator{};

        [[nodiscard]]
        constexpr bool operator==(UnqualifiedId const&) const;
    };

    // This models more or less: https://eel.is/c++draft/expr.prim.id.qual#nt:nested-name-specifier
    struct ScopeSequence
    {
        bool explicitRoot{};
        std::vector<UnqualifiedId> scopes{};

        [[nodiscard]]
        constexpr bool operator==(ScopeSequence const&) const;
    };

    // see: https://eel.is/c++draft/expr.prim.id.qual#nt:qualified-id
    struct QualifiedId
    {
        ScopeSequence scopes{};
        UnqualifiedId identifier{};

        [[nodiscard]]
        constexpr bool operator==(QualifiedId const&) const;
    };

    struct ArrayDeclarator
    {
        std::optional<ConstantExpression> size{};

        [[nodiscard]]
        constexpr bool operator==(ArrayDeclarator const&) const;
    };

    struct PointerDeclarator
    {
        std::optional<CVQualifierSeq> qualifiers{};
        std::optional<ScopeSequence> scopes{};

        [[nodiscard]]
        constexpr bool operator==(PointerDeclarator const&) const;
    };

    struct ReferenceDeclarator
    {
        RefQualifier qualifier{};

        [[nodiscard]]
        constexpr bool operator==(ReferenceDeclarator const&) const;
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
            constexpr bool operator==(Layer const&) const;
        };

        Layer root{};

        [[nodiscard]]
        constexpr bool operator==(AbstractDeclarator const&) const;
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
        constexpr bool try_apply(lexing::keyword const& keyword)
        {
            return try_apply_size_spec(keyword)
                || try_apply_signed_spec(keyword)
                || try_apply_base(keyword);
        }

        [[nodiscard]]
        constexpr bool operator==(BuiltinType const&) const;

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
        constexpr bool try_apply_signed_spec(lexing::keyword const& keyword) noexcept
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
        constexpr bool try_apply_size_spec(lexing::keyword const& keyword) noexcept
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
        constexpr bool operator==(TypeId const&) const = default;
    };

    struct FunctionId
    {
        std::optional<TypeId> returnType{};
        QualifiedId identifier{};

        [[nodiscard]]
        constexpr bool operator==(FunctionId const&) const = default;
    };

    template <typename T>
    constexpr Recursive<T>::~Recursive() = default;

    constexpr bool CVQualifierSeq::operator==(CVQualifierSeq const&) const = default;
    constexpr bool Identifier::operator==(Identifier const&) const = default;
    constexpr bool FunctionDeclarator::operator==(FunctionDeclarator const&) const = default;
    constexpr bool OperatorFunctionId::operator==(OperatorFunctionId const&) const = default;
    constexpr bool ConversionFunctionId::operator==(ConversionFunctionId const&) const = default;
    constexpr bool DestructorFunctionId::operator==(DestructorFunctionId const&) const = default;
    constexpr bool UnqualifiedId::operator==(UnqualifiedId const&) const = default;
    constexpr bool ScopeSequence::operator==(ScopeSequence const&) const = default;
    constexpr bool QualifiedId::operator==(QualifiedId const&) const = default;
    constexpr bool ArrayDeclarator::operator==(ArrayDeclarator const&) const = default;
    constexpr bool PointerDeclarator::operator==(PointerDeclarator const&) const = default;
    constexpr bool ReferenceDeclarator::operator==(ReferenceDeclarator const&) const = default;
    constexpr bool AbstractDeclarator::Layer::operator==(Layer const&) const = default;
    constexpr bool AbstractDeclarator::operator==(AbstractDeclarator const&) const = default;
    constexpr bool BuiltinType::operator==(BuiltinType const&) const = default;
}

#endif
