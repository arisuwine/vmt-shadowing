# vmt-shadowing
A slightly improved implementation of https://github.com/thamelodev/VShadow/tree/master

## Example
```cpp
#include "vmt.hpp"

Object* obj = /* ... */; // Target instance with VMT

using Func = void(__fastcall*)(void*, int); // Function signature

Func FuncOriginal; // Saved original pointer
void __fastcall hkFunction(void* self, int arg) {
    // ...
    FuncOriginal(self, arg);
}

// Setup
VMTSHadowing* vmt_hook = new VMTSHadowing(obj); // Create hook manager
FuncOriginal = vmt_hook->Hook<Func>(11, hkFunction); // Hook vtable function

// Unhooking
vmt_hook->UnHook(11); // Restore specific function
vmt_hook->UnHookAll(); // Restore all hooked functions

// Cleanup
vmt_hook->Shutdown(); // Restore original vtable and free shadow vtable (the destructor also calls Shutdown())
```
