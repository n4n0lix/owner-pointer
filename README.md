# owner
The idea is to have a `unique_ptr<>`, but also the ability to hand out non-owning pointers to the same object, who know when that `unique_ptr<>` gets deleted. 

Et voilá, `owner<>` and `weak<>` were born! I'm currently using it in my private game-engine project with `vector< owner<> >`, `vector< weak<> >` `map< XXX, owner<> >`, `map< weak<>, XXX >` and `map< XXX, weak<> >`.

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
    
## overhead
Resource overhead per managed resource:
- 1 bool* if the managed resource is still valid
- 1 uint32_t* for reference counting

Performance overhead per managed resource:
- Overhead at owner<> creation
- Overhead at owner<> destruction
- Overhead at weak<> creation
- Overhead at weak<> copying 
- Overhead at weak<> destruction
- No overhead at accessing the managed resource

*... this might be wrong, but if you really care about the overhead just have a look into the code yourself :)*
    
## faq
*Why do I get memory leaks when using owner/weak with maps?*
- Use `map.emplace()` (C++11) to insert objects into the map
- Make sure your object has a destructor defined, for example in inheritance don't forget to declare virtual destructors (`virtual ~MyClass() = default;` is enough)

*How can I get `weak<>` from `this`?*
- The class need to inheret from `enable_weak_from_this<T>` with `T` being the class itself to get the protected method `get_non_owner()`. Be aware that this method only returns a valid `weak<>` if the object's lifetime is managed by `owner<>`.

*Why does `this.get_non_owner()` result in a `weak<>` nullptr?*
- This happens if the object's lifetime is not managed by `owner<>`.

*How can I have multiple `owner<>` in a vector with different pointer types?*
- Use `owner_t` or `weak_t`, for example `vector<owner_t>` or `vector<weak_t>`.
