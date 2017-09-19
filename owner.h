#pragma once

#define OWNER_MEM_LEAK_DET
#define OWNER_VECTOR_EXT

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                        Includes                        */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Std-Includes
#include <utility>

#ifdef  OWNER_VECTOR_EXT
    #include <vector>
#endif

// MEMORY LEAK DETECTION
#ifdef OWNER_MEM_LEAK_DET
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    #ifdef _DEBUG
        #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
        #define new DEBUG_NEW
    #endif
#endif

// Other Includes


template<typename T> class owner;

//
//   weak
////////////////////////////////////////////////////////////////

template<typename T>
class weak
{
    template<typename T>             friend class owner;
    template<typename T, typename U> friend weak<T> static_weak_cast(weak<U> u);

public:
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*                        Public                          */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    // Constructor
    weak() : weak(nullptr) { }

    // Constructor
    weak(std::nullptr_t) {
        // Create state
        make_null();
    }

    // Copy-Constructor
    weak(const weak<T>& orig) {
        // Copy state
        _ptr = orig._ptr;
        _ptrValid = orig._ptrValid;
        _ptrRefCounter = orig._ptrRefCounter;

        // Increase ref counter
        ref_count_inc();
    }

    // Move-Constructor
    weak(weak<T>&& orig) {
        // Move state
        _ptr = orig._ptr;
        _ptrValid = orig._ptrValid;
        _ptrRefCounter = orig._ptrRefCounter;

        // Initialize orig to nullptr
        orig.make_null();
    }

    // Destructor
    ~weak() {
        ref_count_dec();
    }

    // Copy assignment
    weak<T>&       operator=(const weak<T>& orig) {
        // Clear state
        destroy();

        // Copy state
        _ptr = orig._ptr;
        _ptrValid = orig._ptrValid;
        _ptrRefCounter = orig._ptrRefCounter;

        // Increase ref counter
        ref_count_inc();

        return *this;
    }

    // Move assignment
    template<typename U> // needed for move assign inherent types
    weak<T>&       operator=(weak<U>&& orig) {
        // Clear state
        destroy();

        // Move state
        _ptr = orig._ptr;
        _ptrValid = orig._ptrValid;
        _ptrRefCounter = orig._ptrRefCounter;

        // Initialize orig to nullptr
        orig.make_null();

        return *this;
    }

    inline T*   operator->() const { return _ptr; }
    inline T*   get()        const { return _ptr; }

    inline bool operator== (const weak &y) const { return _ptr == y._ptr; }
    inline bool operator!= (const weak &y) const { return !(_ptr == y._ptr); }

    template<typename U>
    inline bool operator== (const owner<U> &y) const { return _ptr == y.get(); }

    template<typename U>
    inline bool operator!= (const owner<U> &y) const { return !(_ptr == y.get()); }

    // Determines if it's safe to use the ptr retrieved from get(). Does not ensure that the ptr is not null!
    inline bool ptr_is_valid() const
    {
        return (_ptr == nullptr) ? true : (*_ptrValid);
    }

private:
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*                        Private                         */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    // Constructor
    weak(T* ptr, bool* ptrValid, uint32_t* ptrInsCounter) {
        _ptr = ptr;
        _ptrValid = ptrValid;
        _ptrRefCounter = ptrInsCounter;

        // Increase ref counter
        ref_count_inc();
    }

    inline void make_null() {
        _ptr = nullptr;
        _ptrValid = nullptr;
        _ptrRefCounter = nullptr;
    }

    inline void destroy() {
        // Decrease ref counter
        ref_count_dec();

        // Delete state
        make_null();
    }

    inline uint32_t ref_count()
    {
        return (_ptr == nullptr) ? 1 : *_ptrRefCounter;
    }

    inline void ref_count_inc() {
        if ( _ptr == nullptr ) return;

        (*_ptrRefCounter)++;
    }

    inline void ref_count_dec() {
        if ( _ptr == nullptr ) return;

        (*_ptrRefCounter)--;
        if ((*_ptrRefCounter) == 0) {
            delete _ptrValid;
            delete _ptrRefCounter;
        }
    }

    T*          _ptr;
    bool*       _ptrValid;
    uint32_t*   _ptrRefCounter;

};

//
//   owner
////////////////////////////////////////////////////////////////

template<typename T>
class owner
{
    template<typename T> friend owner<T> make_owner(T* ptr);

public:

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*                        Public                          */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    // Constructor
    owner() : owner(nullptr) {}

    owner(std::nullptr_t) {
        // Nullpointer State
        make_null();
    }

    // Constructor
    explicit owner(T* ptr) {
        // Create new State
        _ptr = ptr;
        _ptrValid = new bool( true );
        _ptrRefCounter = new uint32_t( 0 );

        // Increase ref counter
        ref_count_inc();
    }

    // Copy-Constructor
    owner(const owner<T>&  orig) = delete;

    // Move-Constructor
    template<typename U> // needed for move assign inherent types
    owner(owner<U>&& orig) {
        // Move state
        _ptr = orig._ptr;
        _ptrValid = orig._ptrValid;
        _ptrRefCounter = orig._ptrRefCounter;

        // Initialize orig to nullptr
        orig._ptr = nullptr;
        orig._ptrValid = nullptr;
        orig._ptrRefCounter = nullptr;
    }

    // Destructor
    ~owner() {
        destroy();
    }

    inline T* operator->() const { return _ptr; }
    inline T* get() const { return _ptr; }
    T* release() {
        T* oldPtr = _ptr;

        destroy();

        return oldPtr;
    }

    void destroy() {
        if ( _ptr != nullptr ) {
            invalidate_pointer();
            ref_count_dec();
            delete _ptr;
        }

        make_null();
    }

    // Copy Assignment
    owner<T>&       operator=(const owner<T>& orig) = delete;

    owner<T>&       operator=(const std::nullptr_t&) {
        destroy();
        return *this;
    }

    // Move assignment
    template<typename U> // needed for move assign inherent types
    owner<T>&       operator=(owner<U>&& orig) {
        // Clear this state
        destroy();

        // Move state
        _ptr = orig._ptr;
        _ptrValid = orig._ptrValid;
        _ptrRefCounter = orig._ptrRefCounter;

        // Initialize orig to nullptr
        orig.make_null();

        return *this;
    }

    template<typename U>
    bool            operator== (const weak<U> &y) const { return _ptr == y.get(); }

    template<typename U>
    bool            operator!= (const weak<U> &y) const { return !(_ptr == y.get()); }

    bool            operator== (const owner& other) const { return (_ptr == other._ptr); }
    bool            operator!= (const owner& other) const { return !(this == other); }

    bool            operator== (const std::nullptr_t) const { return _ptr == nullptr; }
    bool            operator!= (const std::nullptr_t) const { return _ptr != nullptr; }

    inline weak<T>  get_non_owner() {
        if (_ptr == nullptr)
            return weak<T>();
        else
            return weak<T>(_ptr, _ptrValid, _ptrRefCounter);
    }

    inline uint32_t ref_count() { return *_ptrRefCounter; }

private:
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*                        Private                         */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    template <typename U>  friend class owner; // needed for move assign inherent types

    inline void invalidate_pointer() {
        if ( _ptrValid != nullptr ) {
            (*_ptrValid) = false;
        }
    }

    inline void ref_count_dec() {
        (*_ptrRefCounter)--;
        
        if ( (*_ptrRefCounter) == 0 ) {
            delete _ptrValid;
            delete _ptrRefCounter;
        }
    }

    inline void ref_count_inc() {
        (*_ptrRefCounter)++;
    }

    inline void make_null() {
        _ptr = nullptr;
        _ptrValid = nullptr;
        _ptrRefCounter = nullptr;
    }

    T*          _ptr;
    bool*       _ptrValid;
    uint32_t*   _ptrRefCounter;

};

//
// utility
/////////////////////////////////////////////////////////////////

template<typename T, typename... Args>
owner<T> make_owner(Args&&... args) {
    return owner<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename U>
weak<T> static_weak_cast(weak<U> u) {
    if (u.get() == nullptr)
        return weak<T>(nullptr);
    else
        return weak<T>((T*)u._ptr, u._ptrValid, u._ptrRefCounter);
}

template<typename T>
struct weak_less {
    bool operator() (const weak<T>& x, const weak<T>& y) const {
        return x.get() < y.get();
    }
};

#ifdef  OWNER_VECTOR_EXT
template<typename T>
owner<T> extract_owner(std::vector<owner<T>>& vec, weak<T> ptr) {
    auto it = vec.begin();
    for (; it != vec.end(); ++it) {
        if (it->get() == ptr.get()) break;
    }

    if (it != vec.end()) {
        auto retval = std::move(*it);
        vec.erase(it);
        return std::move(retval);
    }

    return nullptr;
}

template<typename T>
bool contains_owner(const std::vector<owner<T>>& vec, weak<T> ptr) {
    auto it = vec.begin();
    for (; it != vec.end(); ++it) {
        if (it->get() == ptr.get()) break;
    }

    return it != vec.end();
}
#endif
