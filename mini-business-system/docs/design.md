# mini-business-system 设计文档

## 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│                    Business System                          │
├───────────┬───────────┬───────────┬──────────┬─────────────┤
│ Domain    │ Workflow  │ Rule      │ State    │ Approval    │
│ Model     │ Engine    │ Engine    │ Machine  │ Process     │
├───────────┼───────────┼───────────┼──────────┼─────────────┤
│ 限界上下文 │ BPMN 流程  │ 产生式规则  │ 状态图   │ 审批工作流  │
│ 实体/值对象│ 流程实例   │ 正向链推理  │ 层次状态  │ 多级审批    │
│ 聚合根    │ 网关分叉合 │ 决策表     │ 历史状态  │ 条件路由    │
│ 领域事件  │ 定时边界   │ 审计追踪   │ 并行状态  │ 超时升级    │
│ 上下文映射 │           │ 规则版本   │ 事件延迟  │ 委托       │
└───────────┴───────────┴───────────┴──────────┴─────────────┘
```

## 模块设计

### 1. 领域模型 (Domain Model)

DDD 战术设计模式的 C 语言实现。

**核心概念：**

| 概念 | 数据结构 | 说明 |
|------|----------|------|
| Bounded Context | `dm_bounded_context` | 限界上下文，包含聚合、术语表、上下文映射 |
| Entity | `dm_entity` | 具有唯一标识的领域对象 |
| Value Object | `dm_value_object` | 不可变、通过值比较的对象 |
| Aggregate Root | `dm_aggregate_root` | 聚合根，维护领域事件一致性边界 |
| Domain Event | `dm_domain_event` | 领域事件，记录已发生的业务事实 |
| Context Map | `dm_context_map` | 上下文映射关系 |

**上下文映射类型：**
- `DM_MAP_PARTNERSHIP` — 合作关系：两个团队协作
- `DM_MAP_SHARED_KERNEL` — 共享内核：共享部分模型
- `DM_MAP_CUSTOMER_SUPPLIER` — 客户-供应商：上下游依赖
- `DM_MAP_CONFORMIST` — 遵奉者：下游完全遵循上游
- `DM_MAP_ANTICORRUPTION_LAYER` — 防腐层：翻译层隔离
- `DM_MAP_OPEN_HOST_SERVICE` — 开放主机服务：协议定义访问
- `DM_MAP_PUBLISHED_LANGUAGE` — 发布语言：共享文档化语言

### 2. 工作流引擎 (Workflow Engine)

BPMN 2.0 子集的轻量级实现。

**节点类型：**

| 类型 | 枚举值 | 说明 |
|------|--------|------|
| Start | `WF_NODE_START` | 流程起点 |
| Task | `WF_NODE_TASK` | 用户任务 |
| Manual Task | `WF_NODE_MANUAL_TASK` | 人工任务 |
| Service Task | `WF_NODE_SERVICE_TASK` | 自动执行任务 |
| Exclusive Gateway | `WF_NODE_EXCLUSIVE_GATEWAY` | 排他网关(XOR)，条件分支 |
| Parallel Gateway | `WF_NODE_PARALLEL_GATEWAY` | 并行网关(Fork/Join) |
| End | `WF_NODE_END` | 流程终点 |
| Timer Event | `WF_NODE_TIMER_EVENT` | 定时边界事件 |

**令牌机制：**
每个流程实例通过 token 在节点间传递来推动流程执行：
- 排他网关：评估条件表达式选择一条路径
- 并行网关(Fork)：创建多个 token 并行执行
- 并行网关(Join)：等待所有 token 到达后汇合
- 任务节点：等待人工完成任务后继续

**变量系统：**
`wf_process_instance_set_var/get_var` 支持在流程实例中存储键值对变量，用于网关条件表达式评估。

### 3. 规则引擎 (Rule Engine)

正向链推理的生产式规则引擎。

**推理流程：**
```
事实插入 → 规则匹配(Rete网络模拟) → 冲突消解(优先级/显著性) → 执行动作 → 循环
```

**规则结构：**
- `conditions[]` — 条件列表（AND 关系），每个条件指定 field、操作符、期望值
- `actions[]` — 动作列表，支持 ASSERT(断言新事实)、RETRACT(撤销)、MODIFY(修改)、PRINT(输出)

**操作符：**
`==`, `!=`, `>`, `>=`, `<`, `<=`, `CONTAINS`

**决策表：**
`re_decision_table` 提供表格形式的规则表达，适合业务人员配置。条件列为维度，行为为结果。

**规则版本化：**
`re_rule_versioning` 跟踪规则的多个版本，支持版本回滚。

**审计追踪：**
每次规则触发都会记录 `re_audit_entry`，包括时间戳、规则名、执行详情。

### 4. 状态机 (State Machine)

Harel Statecharts 的部分实现。

**状态类型：**
- `SM_STATE_INITIAL` — 初始状态
- `SM_STATE_NORMAL` — 普通状态
- `SM_STATE_FINAL` — 终态
- `SM_STATE_HISTORY_SHALLOW` — 浅历史状态
- `SM_STATE_HISTORY_DEEP` — 深历史状态

**状态动作：**
- Entry Actions — 进入状态时执行
- Exit Actions — 离开状态时执行
- Do Actions — 状态活跃期间可重复执行

**转换：**
`事件 → [守卫条件] → [转换动作] → 目标状态`

**层次状态：**
状态可以有子状态，形成嵌套结构。事件先匹配当前状态，未匹配则向上冒泡到父状态。

**并行状态：**
通过 `orthogonal` 字段支持正交区域。

**事件延迟：**
`deferred_events` 列表中的事件在状态活跃期间被延迟，直到进入不延迟该事件的状态再处理。

### 5. 审批流程 (Approval Process)

企业级审批工作流实现。

**审批流程：**
```
Draft → Submitted → In Review → Approved/Rejected
                                → Escalated
                                → Delegated
                                → Timed Out
```

**审批级别类型：**
- `AP_LEVEL_SEQUENTIAL` — 串行：审批人依次审批
- `AP_LEVEL_PARALLEL` — 并行：多人同时审批，需要全部或指定数量通过
- `AP_LEVEL_DYNAMIC` — 动态：根据条件决定是否激活该级别
- `AP_LEVEL_ANY_ONE` — 任一：任何一人审批即可

**条件路由：**
- `AP_COND_AMOUNT_GT/GTE/LT` — 金额阈值条件
- `AP_COND_TYPE_EQUALS/DEPT_EQUALS` — 类型/部门匹配
- `AP_COND_CUSTOM` — 自定义条件函数

**高级特性：**
- **委托：** `ap_approval_instance_delegate()` 将审批权委托给其他人
- **超时自动升级：** `ap_approval_instance_check_timeout()` 检测超时并自动升级
- **审计日志：** 记录每一步操作的 actor、action、状态变化

## 数据流

```
用户请求 → 领域模型(验证业务规则)
       → 规则引擎(确定审批路径)
       → 工作流引擎(编排流程)
       → 状态机(追踪生命周期)
       → 审批流程(执行审批)
       → 审计日志(记录全链路)
```

## 限制

- 所有数组使用静态分配（栈/数据段），无动态内存分配
- 最大容量由各模块的 `*_MAX_*` 常量定义
- 并发未实现，单线程使用
- 持久化未实现，所有数据在内存中

## 依赖

- C99 标准库：`<stddef.h>`, `<stdint.h>`, `<string.h>`, `<stdlib.h>`, `<stdio.h>`, `<time.h>`
- 无外部依赖
