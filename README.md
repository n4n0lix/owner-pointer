# owner
The idea is to have a `unique_ptr<>`, but also the ability to hand out non-owning pointers to the same object, who know when that `unique_ptr<>` gets deleted. 

Et voilá, `owner<>` and `weak<>` were born! I'm currently using it in my private game-engine project with `vector< owner<> >`, `vector< weak<> >` `map< XXX, owner<> >`, `map< weak<>, XXX >` and `map< XXX, weak<> >`.

## faq
*Why do I get memory leaks when using owner/weak with maps?*
- Use `map.emplace()` (C++11) to insert objects into the map
- Make sure your object has a destructor defined, even if its just `~MyClass() = default;`
