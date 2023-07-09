# binaryninja-xnu

Early WIP

this will expand as I continue to burrow through kernel documentation/write-ups/code in an effort to understand
everything I'm doing here. consider this an effort to make my 'homework' take a form useful to other people

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

