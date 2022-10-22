#pragma once

#include <cstddef>

#include "Platform.h"

#ifdef _WIN32
#include <x86RetSpoof.h>
#include "../RetSpoofGadgets.h"
#endif

#include <Platform/RetSpoofInvoker.h>

namespace VirtualMethod
{
    template <typename T, std::size_t Idx, typename... Args>
    [[deprecated("Use VirtualCallable class instead")]] constexpr T call(void* classBase, Args... args) noexcept
    {
#ifdef _WIN32
        return retSpoofGadgets.client.invokeThiscall<T, Args...>(std::uintptr_t(classBase), (*reinterpret_cast<std::uintptr_t**>(classBase))[Idx], args...);
#else
        return (*reinterpret_cast<T(THISCALL_CONV***)(void*, Args...)>(classBase))[Idx](classBase, args...);
#endif
    }
}

class VirtualCallable {
public:
    VirtualCallable(RetSpoofInvoker invoker, std::uintptr_t thisptr)
        : invoker{ invoker }, thisptr{ thisptr } {}

    template <typename ReturnType, std::size_t Idx, typename... Args>
    constexpr ReturnType call(Args... args) const noexcept
    {
        return invoker.invokeThiscall<ReturnType, Args...>(thisptr, (*reinterpret_cast<std::uintptr_t**>(thisptr))[Idx], args...);
    }

    [[nodiscard]] std::uintptr_t getThis() const noexcept
    {
        return thisptr;
    }

    [[nodiscard]] RetSpoofInvoker getInvoker() const noexcept
    {
        return invoker;
    }

private:
    RetSpoofInvoker invoker;
    std::uintptr_t thisptr;
};

template <typename T, typename POD>
class VirtualCallableFromPOD {
public:
    VirtualCallableFromPOD(RetSpoofInvoker invoker, POD* pod)
        : invoker{ invoker }, thisptr{ std::uintptr_t(pod) } {}

    [[nodiscard]] static T from(RetSpoofInvoker invoker, POD* pod) noexcept
    {
        return T{ VirtualCallableFromPOD{ invoker, pod } };
    }

    [[nodiscard]] POD* getPOD() const noexcept
    {
        return reinterpret_cast<POD*>(thisptr);
    }

    template <typename ReturnType, std::size_t Idx, typename... Args>
    constexpr ReturnType call(Args... args) const noexcept
    {
        return invoker.invokeThiscall<ReturnType, Args...>(thisptr, (*reinterpret_cast<std::uintptr_t**>(thisptr))[Idx], args...);
    }

    [[nodiscard]] std::uintptr_t getThis() const noexcept
    {
        return thisptr;
    }

    [[nodiscard]] RetSpoofInvoker getInvoker() const noexcept
    {
        return invoker;
    }

private:
    RetSpoofInvoker invoker;
    std::uintptr_t thisptr;
};

#define VIRTUAL_METHOD2(returnType, name, idx, args, argsRaw) \
returnType name args const noexcept \
{ \
    return call<returnType, idx>argsRaw; \
}

#ifdef _WIN32
#define VIRTUAL_METHOD2_V(returnType, name, idx, args, argsRaw) VIRTUAL_METHOD2(returnType, name, idx, args, argsRaw)
#else
#define VIRTUAL_METHOD2_V(returnType, name, idx, args, argsRaw) VIRTUAL_METHOD2(returnType, name, idx + 1, args, argsRaw)
#endif
