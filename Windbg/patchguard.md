# PatchGuard

Kernel Patch Protection (KPP)，内核修补程序保护（KPP），非正式的称为PatchGuard，俗称PG

## 怎样触发
- 修改系统服务表
- 修改中断描述符表（IDT）
- 修改全局描述符表（GDT）
- 使用内核未分配的内核堆栈
- 修改或修补内核本身，或HAL或NDIS内核库
