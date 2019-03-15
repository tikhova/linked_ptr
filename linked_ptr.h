#ifndef LINKED_PTR_H
#define LINKED_PTR_H

#include <cassert>
#include <utility>
#include <functional>

namespace smart_ptr {

template <typename T>
class linked_ptr;

namespace details {

    struct linked_ptr_base {
        linked_ptr_base() noexcept {
            _left = this;
            _right = this;
        }

        // this element is the only one in the list
        bool unique() const noexcept {
            return _left == this && _right == this;
        }

        void swap(linked_ptr_base& other) noexcept {
            // nothing to swap if the elements are unique
            if (unique() && other.unique())
                return;

            std::swap(_right, other._right);
            std::swap(_left, other._left);

            if (!unique())
                _right->_left = _left->_right = this;
            if (!other.unique())
                other._right->_left = other._left->_right = &other;
        }

        // insert this element after rhs
        // is used only in linked_ptr constructors
        void insert_after(linked_ptr_base& rhs) noexcept {
            assert(unique());
            _right = rhs._right;
            _right->_left = this;
            _left = &rhs;
            rhs._right = this;
        }

        void erase() noexcept {
            _right->_left = _left;
            _left->_right = _right;
            _right = _left = this;
        }

        linked_ptr_base* _left;
        linked_ptr_base* _right;
    };

} // namespace details

template <typename T>
class linked_ptr {
    template <typename Y>
    friend class linked_ptr;

public:
    using element_type = T;

private:
    template <typename Y>
    using type_compatible = std::enable_if_t<std::is_convertible<Y*, T*>::value>;
    T* _ptr = nullptr;
    mutable details::linked_ptr_base base;

public:
    // Constructors
    linked_ptr() noexcept = default;

    explicit linked_ptr(std::nullptr_t) noexcept : linked_ptr() {}

    linked_ptr(const linked_ptr& rhs) noexcept {
        base.insert_after(rhs.base);
        _ptr = rhs.get();
    }

    template <typename Y, typename = type_compatible<Y> >
    explicit linked_ptr(Y* ptr) noexcept : _ptr(static_cast<T*>(ptr)) {}

    template <typename Y, typename = type_compatible<Y> >
    linked_ptr(const linked_ptr<Y>& rhs) noexcept {
        base.insert_after(rhs.base);
        _ptr = static_cast<T*>(rhs.get());
    }

    ~linked_ptr() {
        reset();
    }

    // Info

    T* get() const noexcept {
        return _ptr;
    }

    bool unique() const noexcept {
        return base.unique();
    }

    // Modification

    template <typename Y, typename = type_compatible<Y> >
    void reset(Y* ptr) {
        if (unique()) delete _ptr;
        else base.erase();
        _ptr = ptr;
    }

    void reset() noexcept {
        if (unique()) delete _ptr;
        else base.erase();
        _ptr = nullptr;
    }

    void swap(linked_ptr& other) noexcept {
        if (_ptr == other._ptr)
            return;

        base.swap(other.base);
        std::swap(_ptr, other._ptr);
    }

    // Operators

    template <typename Y, typename = type_compatible<Y> >
    linked_ptr& operator=(const linked_ptr<Y>& rhs) noexcept {
        reset(rhs.get());
        insert_after(const_cast<linked_ptr<Y>&>(rhs));
        return *this;
    }

    /// Access operators
    T& operator*() const noexcept {
        return *_ptr;
    }

    T* operator->() const noexcept {
        return _ptr;
    }

    /// Logical expression
    explicit operator bool() const noexcept {
        return _ptr != nullptr;
    }
}; // linked_ptr

/// Logic operators
template <typename T, typename Y>
bool operator==(const linked_ptr<T>& lhs, const linked_ptr<Y>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template <typename T, typename Y>
bool operator!=(const linked_ptr<T>& lhs, const linked_ptr<Y>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename T, typename Y>
bool operator<(const linked_ptr<T>& lhs, const linked_ptr<Y>& rhs) noexcept {
    return std::less<>()(static_cast<void*>(lhs.get()), static_cast<void*>(rhs.get()));
}

template <typename T, typename Y>
bool operator>(const linked_ptr<T>& lhs, const linked_ptr<Y>& rhs) noexcept {
    return !(lhs < rhs) && !(lhs == rhs);
}

template <typename T, typename Y>
bool operator<=(const linked_ptr<T>& lhs, const linked_ptr<Y>& rhs) noexcept {
    return !(lhs > rhs);
}

template <typename T, typename Y>
bool operator>=(const linked_ptr<T>& lhs, const linked_ptr<Y>& rhs) noexcept {
    return !(lhs < rhs);
}

} // namespace smart_ptr

#endif // LINKED_PTR_H
