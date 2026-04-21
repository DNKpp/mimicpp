//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_PARSER2_STATE_HPP
#define MIMICPP_PRINTING_TYPE_NAME_PARSER2_STATE_HPP

#pragma once

#include "Parser2State.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/NameLexer.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <functional>
    #include <memory>
    #include <optional>
    #include <utility>
    #include <variant>
    #include <vector>
#endif

namespace mimicpp::printing::type::parsing::v2::state
{
    struct TypeId;

    struct RecursiveTypeId
    {
    public:
        ~RecursiveTypeId() noexcept;

        [[nodiscard]]
        RecursiveTypeId(RecursiveTypeId const&);
        RecursiveTypeId& operator=(RecursiveTypeId const&);

        [[nodiscard]]
        RecursiveTypeId(RecursiveTypeId&&) noexcept;
        RecursiveTypeId& operator=(RecursiveTypeId&&) noexcept;

        [[nodiscard]]
        explicit RecursiveTypeId(TypeId type);

        [[nodiscard]]
        TypeId& type() noexcept
        {
            return *m_type;
        }

        [[nodiscard]]
        TypeId const& type() const noexcept
        {
            return *m_type;
        }

        friend bool operator==(RecursiveTypeId const& lhs, RecursiveTypeId const& rhs);

    private:
        std::shared_ptr<TypeId> m_type{};
    };

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
        friend bool operator==(CVQualifierSeq const&, CVQualifierSeq const&) = default;

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

    struct ConstantExpression
    {
        [[nodiscard]]
        friend bool operator==(ConstantExpression const&, ConstantExpression const&) = default;
    };

    using TemplateArgument = std::variant<
        ConstantExpression,
        RecursiveTypeId>;

    using TemplateArgumentList = std::vector<TemplateArgument>;

    struct SimpleTemplateId
    {
        lexing::identifier name{};
        TemplateArgumentList args{};

        [[nodiscard]]
        friend bool operator==(SimpleTemplateId const&, SimpleTemplateId const&) = default;
    };

    struct ParametersAndQualifiers
    {
        [[nodiscard]]
        friend bool operator==(ParametersAndQualifiers const&, ParametersAndQualifiers const&) = default;
    };

    struct OperatorFunctionId
    {
        lexing::operator_or_punctuator op;

        [[nodiscard]]
        friend bool operator==(OperatorFunctionId const&, OperatorFunctionId const&) = default;
    };

    using UnqualifiedId = std::variant<
        lexing::identifier,
        SimpleTemplateId,
        OperatorFunctionId>;

    // This models more or less: https://eel.is/c++draft/expr.prim.id.qual#nt:nested-name-specifier
    struct ScopeSequence
    {
        bool explicitRoot{};
        std::vector<UnqualifiedId> scopes{};

        [[nodiscard]]
        friend bool operator==(ScopeSequence const&, ScopeSequence const&) = default;
    };

    // see: https://eel.is/c++draft/expr.prim.id.qual#nt:qualified-id
    struct QualifiedId
    {
        ScopeSequence scopes{};
        UnqualifiedId identifier{};

        [[nodiscard]]
        friend bool operator==(QualifiedId const&, QualifiedId const&) = default;
    };

    struct ArrayDeclarator
    {
        std::optional<ConstantExpression> size{};

        [[nodiscard]]
        friend bool operator==(ArrayDeclarator const&, ArrayDeclarator const&) = default;
    };

    struct FunctionDeclarator
    {
        // params-and-qualifiers

        [[nodiscard]]
        friend bool operator==(FunctionDeclarator const&, FunctionDeclarator const&) = default;
    };

    struct PointerDeclarator
    {
        std::optional<CVQualifierSeq> qualifiers{};
        std::optional<ScopeSequence> scopes{};

        [[nodiscard]]
        friend bool operator==(PointerDeclarator const&, PointerDeclarator const&) = default;
    };

    struct ReferenceDeclarator
    {
        RefQualifier qualifier{};

        [[nodiscard]]
        friend bool operator==(ReferenceDeclarator const&, ReferenceDeclarator const&) = default;
    };

    using PtrOperator = std::variant<
        ReferenceDeclarator,
        PointerDeclarator>;

    struct AbstractDeclarator
    {
        struct Layer
        {
            std::vector<PtrOperator> decorations{};

            using Core = std::variant<
                std::monostate,
                ArrayDeclarator,
                FunctionDeclarator,
                std::unique_ptr<Layer>>;
            Core core{};

            [[nodiscard]]
            friend bool operator==(Layer const&, Layer const&) = default;
        };

        Layer root{};

        [[nodiscard]]
        friend bool operator==(AbstractDeclarator const&, AbstractDeclarator const&) = default;
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
        friend bool operator==(BuiltinType const&, BuiltinType const&) = default;

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
        CVQualifierSeq leadingQualifications{};
        BaseType base;
        AbstractDeclarator declarator{};

        [[nodiscard]]
        friend bool operator==(TypeId const&, TypeId const&) = default;
    };

    inline RecursiveTypeId::~RecursiveTypeId() noexcept = default;
    inline RecursiveTypeId::RecursiveTypeId(RecursiveTypeId const&) = default;
    inline RecursiveTypeId& RecursiveTypeId::operator=(RecursiveTypeId const&) = default;
    inline RecursiveTypeId::RecursiveTypeId(RecursiveTypeId&&) noexcept = default;
    inline RecursiveTypeId& RecursiveTypeId::operator=(RecursiveTypeId&&) noexcept = default;

    inline RecursiveTypeId::RecursiveTypeId(TypeId type)
        : m_type{std::make_shared<TypeId>(std::move(type))}
    {
    }

    [[nodiscard]]
    inline bool operator==(RecursiveTypeId const& lhs, RecursiveTypeId const& rhs)
    {
        return lhs.type() == rhs.type();
    }
}

#endif
