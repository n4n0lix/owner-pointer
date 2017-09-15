# owner
The idea is to have a `unique_ptr<>`, but also the ability to hand out non-owning pointers to the same object, who know when that `unique_ptr<>` gets deleted. 

Et voilá, `owner<>` and `weak<>` were born! I'm currently using it in my private game-engine project with `vector< owner<> >`, `vector< weak<> >` `map< XXX, owner<> >`, `map< weak<>, XXX >` and `map< XXX, weak<> >` and haven't encountered any memory leaks yet. The source code currently has a lot of duplicate code, this will change when I feel confident enough that the code works as intended. Currently it helps me keeping oversight on what happens where and to track down memory leaks.
