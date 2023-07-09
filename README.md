# binaryninja-xnu

Early WIP

this will expand as I continue to burrow through kernel documentation/write-ups/code in an effort to understand
everything I'm doing here. 

### Build

```shell
mkdir -p build && cd build
# For headless builds, omit -DUI_BUILD=ON argument entirely or set it to OFF
cmake -DCMAKE_BUILD_TYPE=Release -DUI_BUILD=ON ../
cmake --build . --target=install -j 8 
```

### Current Feature Set

##### Type Helper:
* Single-interaction helper for external methods

This component strives to give developers the tools to quickly type methods appropriately

##### Workflow (core.function.darwinKernel) for:
* Consolidating vreg initialization (1 LLIL+ line instead of 16)
* Removing PAC intrinsics from LLIL onward
* (hackily?) Hotfixing tailcalls to unknown destinations (e.g. `this->someVirtFunc(blah)` getting lifted as HLIL `jump(x6)` instead becomes a proper and typeable call)

---

<p align=center>
<img src=".github/vri.png" alt="vri" width=110px>
</p> 
<p align=center>
plugin by cynder
</p>
<p align=center>
part of the <a href="https://github.com/cxnder/ksuite">ksuite</a> toolkit
</p>

