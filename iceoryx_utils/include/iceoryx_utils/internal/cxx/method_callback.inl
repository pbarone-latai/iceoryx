// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_CXX_METHOD_CALLBACK_INL
#define IOX_UTILS_CXX_METHOD_CALLBACK_INL

namespace iox
{
namespace cxx
{
template <typename ReturnValue, typename ClassType, typename... Args>
ReturnValue
constMethodCallbackCaller(void* classPtr, ReturnValue (GenericClass::*methodPtr)(Args...) const, Args... args)
{
    return ((*reinterpret_cast<ClassType*>(classPtr))
            .*reinterpret_cast<ReturnValue (ClassType::*)(Args...) const>(methodPtr))(args...);
}


template <typename ReturnValue, typename ClassType, typename... Args>
ReturnValue methodCallbackCaller(void* classPtr, ReturnValue (GenericClass::*methodPtr)(Args...), Args... args)
{
    return ((*reinterpret_cast<ClassType*>(classPtr))
            .*reinterpret_cast<ReturnValue (ClassType::*)(Args...)>(methodPtr))(args...);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline ConstMethodCallback<ReturnValue, Args...>::ConstMethodCallback(ClassType* classPtr,
                                                                      ReturnValue (ClassType::*methodPtr)(Args...)
                                                                          const) noexcept
    : m_classPtr(classPtr)
    , m_methodPtr(reinterpret_cast<ReturnValue (GenericClass::*)(Args...) const>(methodPtr))
    , m_callback(constMethodCallbackCaller<ReturnValue, ClassType, Args...>)
{
}

template <typename ReturnValue, typename... Args>
inline ReturnValue ConstMethodCallback<ReturnValue, Args...>::operator()(Args... args) const noexcept
{
    return m_callback(m_classPtr, m_methodPtr, args...);
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::operator==(const ConstMethodCallback& rhs) const noexcept
{
    return (m_classPtr == rhs.m_classPtr && m_methodPtr == rhs.m_methodPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline MethodCallback<ReturnValue, Args...>::MethodCallback(ClassType* classPtr,
                                                            ReturnValue (ClassType::*methodPtr)(Args...)) noexcept
    : m_classPtr(classPtr)
    , m_methodPtr(reinterpret_cast<ReturnValue (GenericClass::*)(Args...)>(methodPtr))
    , m_callback(methodCallbackCaller<ReturnValue, ClassType, Args...>)
{
}

template <typename ReturnValue, typename... Args>
inline ReturnValue MethodCallback<ReturnValue, Args...>::operator()(Args... args) noexcept
{
    return m_callback(m_classPtr, m_methodPtr, args...);
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::operator==(const MethodCallback& rhs) const noexcept
{
    return (m_classPtr == rhs.m_classPtr && m_methodPtr == rhs.m_methodPtr);
}


} // namespace cxx
} // namespace iox
#endif
