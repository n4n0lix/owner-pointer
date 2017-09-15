# owner
The idea is to have a `unique_ptr<>`, but also the ability to hand out non-owning pointers to the same object, who know when that `unique_ptr<>` gets deleted.
