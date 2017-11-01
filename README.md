# owner
The idea is to have a `unique_ptr` style owning pointer and the ability to hand out non-owning pointers that know when the object gets deleted. This smart pointer should be treated as an addition to `unique_ptr` and `shared_ptr`, as it's covering the gray area between those two and has it's unique areas of application.

I haven't investigated the performance yet, but I assume due to reference counting *and* managing the validity of the pointer it will be slightly slower than `unique_ptr` and `shared_ptr`. The only exception is accessing the pointer, which shouldn't have any overhead.

## examples
Basic usage:

    owner<int> ownerOfInt = make_owner<int>( 4 );
    weak<int>  weakToInt  = ownerOfInt.get_non_owner();
    // weakToInt.ptr_is_valid() ==> true
    
    ownerOfInt.destroy();
    // weakToInt.ptr_is_valid() ==> false

enable_weak_from_this:

    class MyClass : public enable_weak_from_this<MyClass> {
    
        void someFunc(OtherClass& o) {
          o.set_parent( this.get_non_owner() ); // Requires MyClass to be wrapped by owner<> or it returns a weak<nullptr>
        }
    }
    
## application fields
I use this smartpointer for example in a case where I have a `World` that owns multiple `GameObject` and a `Renderer`. `GameObject` and `World` knows nothing about `Renderer` and `Renderer` doesn't really care about `World`, how long the `GameObject` lives, if it gets replaced, or even deleted. All it cares about is that *if* it observes a `GameObject` that its valid and accessable.
    
## faq

*Why not use `std::shared_ptr<>` and `std::weak_ptr<>`?*
- With `shared_ptr<>` and `weak_ptr<>` as soon as you give out one of both to code you don't control, you have no control over the object lifetime anymore. If that external code decides to turn the `weak_ptr` into a `shared_ptr` that external code from now on decides when the object gets deleted. With `owner<>` and `weak<>` the code that actually owns the object decides when it gets deleted.

*Why do I get memory leaks when using owner/weak with maps?*
- Use `map.emplace()` (C++11) to insert objects into the map
- Make sure your object has a destructor defined, for example in inheritance don't forget to declare virtual destructors (`virtual ~MyClass() = default;` is enough)

*How can I get `weak<>` from `this`?*
- The class need to inheret from `enable_weak_from_this<T>` with `T` being the class itself to get the protected method `get_non_owner()`. Be aware that this method only returns a valid `weak<>` if the object's lifetime is managed by `owner<>`.

*Why does `this.get_non_owner()` result in a `weak<>` nullptr?*
- This happens if the object's lifetime is not managed by `owner<>` and/or the object's class doesn't inherit from `enable_weak_from_this<>`.

*How can I have multiple `owner<>` in a vector with different pointer types?*
- Use `owner_t` or `weak_t`, for example `vector<owner_t>` or `vector<weak_t>`.

## todo features

- provide custom deleter
- implicit conversion from unique_ptr
- non_owner class, that does nothing except flagging that the pointer is not owned.
