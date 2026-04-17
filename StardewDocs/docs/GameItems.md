# Game Items

Game items are defined in the data file ./WfAssets/items.xml.

You can find the schema for it in ./WfAssets/items_schema.xml. You construct the items by setting callbacks and config data for them, you set the callbacks as C function names and optionally a dll to load to get them from, eventually it'll support lua functions too.

You can produce a "Baked" build where his items.xml file is used to generate C code, in a non baked build `dlopen`,`dlsym` and their windows equivalents are used to load function pointers by name. The idea of the baked build is for possible platforms where this might be problematic such as wasm.

The script used to generate c code is `./game/ma_baker.nu`, a nushell script. This is a nice hybrid between a shell and a functional programming language that works on windows and linux and I may use it more extensively for tooling and build automation in future.

See [this page](./Build.md) for how to enable the "baked" build.