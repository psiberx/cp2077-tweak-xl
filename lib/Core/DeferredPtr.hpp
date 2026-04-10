#pragma once
#include "Stl.hpp"

namespace Core
{
/**
 * @brief A wrapper around a shared pointer that allows deferred assignment and shared mutability of the underlying
 * pointer. This is intended to be used in cases where a pointer needs to be shared across multiple components but
 * cannot be immediately assigned at the time of construction. This is useful when dealing with circular dependencies or
 * when the pointer needs to be assigned after construction due to initialization order constraints. The DeferredPtr
 * allows for the pointer to be assigned at a later time while still being safely shared across multiple components, and
 * it provides convenient access to the underlying pointer through operator overloads.
 *
 * @tparam T The type of the underlying pointer.
 */
template<typename T>
class DeferredPtr
{
public:
    /**
     * @brief Constructs a DeferredPtr instance with a null pointer. The underlying pointer can be assigned later using
     * the assignment operator or by directly modifying the shared pointer returned by the GetShared() member function.
     */
    DeferredPtr() noexcept
        : m_slot(Core::MakeShared<SharedPtr<T>>(nullptr))
    {
    }

    /**
     * @brief Constructs a DeferredPtr instance with a null pointer. This constructor is provided to allow for implicit
     * conversion from nullptr, enabling syntax such as `DeferredPtr<T> ptr = nullptr;` to create a DeferredPtr with a
     * null pointer.
     */
    DeferredPtr(std::nullptr_t) noexcept
        : DeferredPtr()
    {
    }

    /**
     * @brief Constructs a DeferredPtr instance that shares ownership of the given shared pointer. The underlying
     * pointer can be accessed and modified through this DeferredPtr instance, and any changes to the shared pointer
     * will be reflected across all DeferredPtr instances that share ownership of the same pointer.
     *
     * @param aInstance The shared pointer to share ownership of.
     */
    DeferredPtr(SharedPtr<T> aInstance) noexcept
        : m_slot(Core::MakeShared<SharedPtr<T>>(std::move(aInstance)))
    {
    }

    /**
     * @brief Constructs a DeferredPtr instance that shares ownership of a new shared pointer created from the given
     * instance. The underlying pointer can be accessed and modified through this DeferredPtr instance, and any changes
     * to the shared pointer will be reflected across all DeferredPtr instances that share ownership of the same
     * pointer.
     *
     * @param aInstance The instance to create a shared pointer from and share ownership of.
     */
    explicit DeferredPtr(const T& aInstance) noexcept
        : m_slot(Core::MakeShared<SharedPtr<T>>(Core::MakeShared<T>(aInstance)))
    {
    }

    /**
     * @brief Constructs a DeferredPtr instance that shares ownership of a new shared pointer created from the given
     * instance. The underlying pointer can be accessed and modified through this DeferredPtr instance, and any changes
     * to the shared pointer will be reflected across all DeferredPtr instances that share ownership of the same
     * pointer.
     *
     * @param aInstance The instance to create a shared pointer from and share ownership of. This constructor is
     * provided to allow for move semantics when constructing a DeferredPtr from an instance, enabling syntax such as
     * `DeferredPtr<T> ptr = T(...);`.
     */
    explicit DeferredPtr(T&& aInstance) noexcept
        : m_slot(Core::MakeShared<SharedPtr<T>>(Core::MakeShared<T>(std::move(aInstance))))
    {
    }

    /**
     * @brief Checks whether this DeferredPtr instance currently has a non-null pointer assigned. This is determined by
     * checking whether the shared pointer contained within the DeferredPtr is non-null. If this function returns true,
     * it means that the DeferredPtr has a valid instance assigned and the underlying pointer can be safely accessed.
     *
     * @return Whether this DeferredPtr instance currently has a non-null pointer assigned.
     */
    [[nodiscard]] bool HasInstance() const noexcept
    {
        return static_cast<bool>(*m_slot);
    }

    /**
     * @brief Provides an explicit conversion operator to bool that checks whether this DeferredPtr instance currently
     * has a non-null pointer assigned. This allows for convenient syntax such as `if (ptr) { ... }` to check for the
     * presence of an instance in the DeferredPtr.
     */
    explicit operator bool() const noexcept
    {
        return HasInstance();
    }

    /**
     * @brief Provides access to the underlying pointer through the arrow operator. This allows for convenient syntax
     * such as `ptr->member` to access members of the underlying pointer.
     *
     * @return A pointer to the underlying instance of type T.
     */
    T* operator->() const noexcept
    {
        return m_slot->get();
    }

    /**
     * @brief Provides an implicit conversion operator to SharedPtr<T> that returns the shared pointer contained within
     * this DeferredPtr. This allows for convenient syntax such as `SharedPtr<T> shared = ptr;` to obtain a shared
     * pointer to the underlying instance.
     */
    operator SharedPtr<T>() const noexcept
    {
        return *m_slot;
    }

    /**
     * @brief Sets the underlying pointer to null, effectively clearing any instance assigned to this DeferredPtr. After
     * calling this function, HasInstance() will return false. This function can be used to reset the DeferredPtr
     * to an empty state when the instance it holds is no longer needed or valid.
     *
     * @return A reference to the shared pointer contained within this DeferredPtr.
     */
    DeferredPtr& operator=(std::nullptr_t) noexcept
    {
        *m_slot = nullptr;
        return *this;
    }

    /**
     * @brief Assigns a new shared pointer to this DeferredPtr, sharing ownership of the given instance. After calling
     * this function, HasInstance() will return true if the given shared pointer is non-null, and the underlying pointer
     * will be updated to reflect the new instance. This function can be used to assign a new instance to the
     * DeferredPtr after it has been constructed, allowing for deferred assignment of the underlying pointer.
     *
     * @param aInstance The shared pointer to share ownership of and assign to this DeferredPtr.
     * @return A reference to this DeferredPtr instance after the assignment is complete.
     */
    DeferredPtr& operator=(const SharedPtr<T>& aInstance) noexcept
    {
        *m_slot = aInstance;
        return *this;
    }

    /**
     * @brief Assigns a new shared pointer to this DeferredPtr, sharing ownership of the given instance. After calling
     * this function, HasInstance() will return true if the given shared pointer is non-null, and the underlying pointer
     * will be updated to reflect the new instance. This function can be used to assign a new instance to the
     * DeferredPtr after it has been constructed, allowing for deferred assignment of the underlying pointer.
     *
     * @param aInstance The shared pointer to share ownership of and assign to this DeferredPtr.
     * @return A reference to this DeferredPtr instance after the assignment is complete.
     */
    DeferredPtr& operator=(SharedPtr<T>&& aInstance) noexcept
    {
        *m_slot = std::move(aInstance);
        return *this;
    }

    /**
     * @brief Provides access to the underlying pointer through the dereference operator. This allows for convenient
     * syntax such as `*ptr` to access the underlying instance.
     *
     * @return A reference to the underlying instance of type T.
     */
    T& operator*()
    {
        if (!HasInstance())
            return nullptr;

        return **m_slot;
    }

    /**
     * @brief Provides access to the underlying pointer through the dereference operator. This allows for convenient
     * syntax such as `*ptr` to access the underlying instance.
     *
     * @return A const reference to the underlying instance of type T.
     */
    const T& operator*() const
    {
        if (!HasInstance())
            return nullptr;

        return **m_slot;
    }

private:
    /**
     * @brief A shared pointer to a shared pointer of type T that serves as the underlying storage for the DeferredPtr.
     * This allows for deferred assignment and shared mutability of the underlying pointer, as multiple DeferredPtr
     * instances can share ownership of the same shared pointer.
     *
     * Any changes to the shared pointer will be reflected across all instances that share ownership of it. The outer
     * shared pointer allows for the shared pointer to be modified and reassigned while still being safely shared across
     * multiple DeferredPtr instances, enabling the deferred assignment of the underlying pointer that is the primary
     * purpose of this class.
     */
    SharedPtr<SharedPtr<T>> m_slot;
};

/**
 * @brief A helper function to create a DeferredPtr instance with a new shared pointer created from the given arguments.
 * This function simplifies the creation of a DeferredPtr by allowing for the construction of the underlying instance
 * directly from the provided arguments, without needing to manually create a shared pointer first.
 *
 * The function forwards the given arguments to the constructor of T to create a new instance, which is then wrapped in
 * a shared pointer and assigned to a new DeferredPtr instance. This allows for convenient syntax such as `auto ptr =
 * MakeDeferred<T>(args...);` to create a DeferredPtr with a new instance of T constructed from the provided arguments.
 *
 * @tparam T The type of the underlying pointer that the DeferredPtr will hold.
 * @tparam Args The types of the arguments to forward to the constructor of T when creating the underlying instance.
 * @param aArgs The arguments to forward to the constructor of T when creating the underlying instance.
 * @return A DeferredPtr instance that holds a new shared pointer to an instance of T constructed from the provided
 * arguments.
 */
template<typename T, typename... Args>
auto MakeDeferred(Args&&... aArgs)
{
    return DeferredPtr<T>{std::allocate_shared<T>(TiltedPhoques::StlAllocator<T>(), std::forward<Args>(aArgs)...)};
}

} // namespace Core
