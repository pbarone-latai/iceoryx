// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iox/semantic_string.hpp"
#include "iox/user_name.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

// NOLINTBEGIN(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)
template <typename T>
struct TestValues;

template <>
struct TestValues<UserName>
{
    static constexpr uint64_t CAPACITY = platform::MAX_USER_NAME_LENGTH;
    static constexpr const char VALID_VALUES[][CAPACITY]{{"some-user"}, {"user2"}};
    static constexpr const char INVALID_CHARACTER_VALUES[][CAPACITY]{
        {"some-!user"}, {"*kasjd"}, {"_fuuuas"}, {"asd/asd"}, {";'1'fuuuu"}, {"argh/"}};
    static constexpr const char INVALID_CONTENT_VALUES[][CAPACITY]{
        {""}, {"-do-not-start-with-dash"}, {"5do-not-start-with-a-number"}};
    static constexpr const char TOO_LONG_CONTENT_VALUES[][CAPACITY * 2]{{"i-am-waaaaay-toooooooo-loooooooong"}};
};
constexpr const char TestValues<UserName>::VALID_VALUES[][TestValues<UserName>::CAPACITY];
constexpr const char TestValues<UserName>::INVALID_CHARACTER_VALUES[][TestValues<UserName>::CAPACITY];
constexpr const char TestValues<UserName>::INVALID_CONTENT_VALUES[][TestValues<UserName>::CAPACITY];
constexpr const char TestValues<UserName>::TOO_LONG_CONTENT_VALUES[][TestValues<UserName>::CAPACITY * 2];
// NOLINTEND(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)

template <typename T>
class SemanticString_test : public Test
{
  protected:
    using SutType = T;
};

using Implementations = Types<UserName>;

// cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay: we use compile time
//      string literals as test input. Those string literals are decaying into pointers in a safe
//      null-terminated fashion and are therefore safe to use.
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
TYPED_TEST_SUITE(SemanticString_test, Implementations, );

TYPED_TEST(SemanticString_test, InitializeWithValidStringLiteralWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "31a2cd17-ca02-486a-b173-3f1f219d8ca3");
    using SutType = typename TestFixture::SutType;

    auto sut = SutType::create("alwaysvalid");

    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->size(), Eq(11));
    EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));
    EXPECT_TRUE(sut->as_string() == "alwaysvalid");
}

TYPED_TEST(SemanticString_test, InitializeWithValidStringValueWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0100d764-628c-44ad-9af7-fe7a4540491a");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(false));
        EXPECT_THAT(sut->size(), Eq(strnlen(value, TestValues<SutType>::CAPACITY)));
        EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));
        EXPECT_THAT(sut->as_string(), Eq(string<SutType::capacity()>(TruncateToCapacity, value)));
    }
}

TYPED_TEST(SemanticString_test, InitializeWithStringContainingIllegalCharactersFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "14483f4e-d556-4770-89df-84d873428eee");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::INVALID_CHARACTER_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(true));
        ASSERT_THAT(sut.get_error(), Eq(SemanticStringError::ContainsInvalidCharacters));
    }
}

TYPED_TEST(SemanticString_test, InitializeWithStringContainingIllegalContentFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "9380f932-527f-4116-bd4f-dc8078b63330");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::INVALID_CONTENT_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(true));
        ASSERT_THAT(sut.get_error(), Eq(SemanticStringError::ContainsInvalidContent));
    }
}

TYPED_TEST(SemanticString_test, InitializeWithTooLongContentFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5597825-c559-48e7-96f3-5136fffc55d7");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::TOO_LONG_CONTENT_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity() * 2>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(true));
        ASSERT_THAT(sut.get_error(), Eq(SemanticStringError::ExceedsMaximumLength));
    }
}

TYPED_TEST(SemanticString_test, AppendValidContentToValidStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0994fccc-5baa-4408-b17e-e2955439608d");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        for (auto add_value : TestValues<SutType>::VALID_VALUES)
        {
            auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));
            ASSERT_THAT(sut.has_error(), Eq(false));

            EXPECT_THAT(sut->append(string<SutType::capacity()>(TruncateToCapacity, add_value)).has_error(), Eq(false));
            auto result_size =
                strnlen(value, TestValues<SutType>::CAPACITY) + strnlen(add_value, TestValues<SutType>::CAPACITY);
            EXPECT_THAT(sut->size(), result_size);
            EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));

            auto result = string<SutType::capacity()>(TruncateToCapacity, value);
            result.append(TruncateToCapacity, string<SutType::capacity()>(TruncateToCapacity, add_value));
            EXPECT_THAT(sut->as_string(), Eq(result));
        }
    }
}

TYPED_TEST(SemanticString_test, AppendInvalidContentToValidStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "fddf4a56-c368-4ff0-8727-e732d6ebc87f");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        for (auto invalid_value : TestValues<SutType>::INVALID_CHARACTER_VALUES)
        {
            auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));
            ASSERT_THAT(sut.has_error(), Eq(false));

            EXPECT_THAT(sut->append(string<SutType::capacity()>(TruncateToCapacity, invalid_value)).has_error(),
                        Eq(true));
            EXPECT_THAT(sut->size(), strnlen(value, TestValues<SutType>::CAPACITY));
            EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));

            auto result = string<SutType::capacity()>(TruncateToCapacity, value);
            EXPECT_THAT(sut->as_string(), Eq(result));
        }
    }
}

TYPED_TEST(SemanticString_test, AppendTooLongContentToValidStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8616fbf-601d-43b9-b4a3-f6b96acdf555");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        for (auto invalid_value : TestValues<SutType>::TOO_LONG_CONTENT_VALUES)
        {
            auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));
            ASSERT_THAT(sut.has_error(), Eq(false));

            EXPECT_THAT(sut->append(string<SutType::capacity()>(TruncateToCapacity, invalid_value)).has_error(),
                        Eq(true));
            EXPECT_THAT(sut->size(), strnlen(value, TestValues<SutType>::CAPACITY));
            EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));

            auto result = string<SutType::capacity()>(TruncateToCapacity, value);
            EXPECT_THAT(sut->as_string(), Eq(result));
        }
    }
}

TYPED_TEST(SemanticString_test, InsertValidContentToValidStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "56ea499f-5ac3-4ffe-abea-b56194cfd728");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        for (auto add_value : TestValues<SutType>::VALID_VALUES)
        {
            auto string_size = strnlen(value, TestValues<SutType>::CAPACITY);
            for (uint64_t insert_position = 0; insert_position < string_size; ++insert_position)
            {
                auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));
                ASSERT_THAT(sut.has_error(), Eq(false));

                auto add_value_size = strnlen(add_value, TestValues<SutType>::CAPACITY);
                EXPECT_THAT(sut->insert(insert_position,
                                        string<SutType::capacity()>(TruncateToCapacity, add_value),
                                        add_value_size)
                                .has_error(),
                            Eq(false));


                auto result_size = strnlen(value, TestValues<SutType>::CAPACITY) + add_value_size;
                EXPECT_THAT(sut->size(), result_size);
                EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));

                auto result = string<SutType::capacity()>(TruncateToCapacity, value);
                result.insert(
                    insert_position, string<SutType::capacity()>(TruncateToCapacity, add_value), add_value_size);
                EXPECT_THAT(sut->as_string(), Eq(result));
            }
        }
    }
}

TYPED_TEST(SemanticString_test, InsertInvalidContentToValidStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "35229fb8-e6e9-44d9-9d47-d00b71a4ce01");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        for (auto add_value : TestValues<SutType>::INVALID_CHARACTER_VALUES)
        {
            auto string_size = strnlen(value, TestValues<SutType>::CAPACITY);
            for (uint64_t insert_position = 0; insert_position < string_size; ++insert_position)
            {
                auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));
                ASSERT_THAT(sut.has_error(), Eq(false));

                auto add_value_size = strnlen(add_value, TestValues<SutType>::CAPACITY);
                EXPECT_THAT(sut->insert(insert_position,
                                        string<SutType::capacity()>(TruncateToCapacity, add_value),
                                        add_value_size)
                                .has_error(),
                            Eq(true));


                auto result_size = strnlen(value, TestValues<SutType>::CAPACITY);
                EXPECT_THAT(sut->size(), result_size);
                EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));

                auto result = string<SutType::capacity()>(TruncateToCapacity, value);
                EXPECT_THAT(sut->as_string(), Eq(result));
            }
        }
    }
}

TYPED_TEST(SemanticString_test, InsertTooLongContentToValidStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6939126-a878-4d7f-9fea-c2b438226e65");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        for (auto add_value : TestValues<SutType>::TOO_LONG_CONTENT_VALUES)
        {
            auto string_size = strnlen(value, TestValues<SutType>::CAPACITY);
            for (uint64_t insert_position = 0; insert_position < string_size; ++insert_position)
            {
                auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));
                ASSERT_THAT(sut.has_error(), Eq(false));

                auto add_value_size = strnlen(add_value, TestValues<SutType>::CAPACITY);
                EXPECT_THAT(sut->insert(insert_position,
                                        string<SutType::capacity()>(TruncateToCapacity, add_value),
                                        add_value_size)
                                .has_error(),
                            Eq(true));


                auto result_size = strnlen(value, TestValues<SutType>::CAPACITY);
                EXPECT_THAT(sut->size(), result_size);
                EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));

                auto result = string<SutType::capacity()>(TruncateToCapacity, value);
                EXPECT_THAT(sut->as_string(), Eq(result));
            }
        }
    }
}
// NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)


} // namespace
