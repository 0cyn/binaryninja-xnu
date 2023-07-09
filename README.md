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

## Current Feature Set

##### Type Helper:
* Single-interaction helper for external methods

This component strives to give developers the tools to quickly type methods appropriately.

Available under the command palette (and keybindable) as "XNU Tools - Types - <type of function>"

##### Workflow (core.function.darwinKernel) for:
* Consolidating vreg initialization (1 LLIL+ line instead of 16)
* Removing PAC intrinsics from LLIL onward
* (hackily?) Hotfixing tailcalls to unknown destinations (e.g. `this->someVirtFunc(blah)` getting lifted as HLIL `jump(x6)` instead becomes a proper and typeable call)

## Future

A _lot_ of automatic type generation and naming via libkern's introspection APIs is possible.

Following in the footsteps of [iometa](https://github.com/Siguza/iometa)'s with automated 
dumping of this information and applying it to individual kexts (where possible) or kernelcaches will elevate kernel RE heavily.

#### Automatic detection

Plugin should detect when:
* A BinaryView of any BinaryViewType is of some sort of xnu-kernel related object
* It has not had an initial config pass ran on it

It should ask to enable certain features, prompt if the workflow isn't enabled, and run initial passes.

#### Workflow

A workflow action should eventually be added to rewrite calls made to vtable methods when we know
what code the virtual call will hit.

#### Typing

Huge swaths of IOKit and the kernel itself are open source, and type information publicly available
should be automatically defined in applicable BinaryViews

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

