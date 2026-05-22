# mini-business-system 使用指南

## 构建

```sh
cd mini-business-system
make
```

输出 `build/libbusiness.a` 静态库。

## 编译示例

```sh
make examples    # 编译 3 个示例
make run-examples # 运行全部示例
```

## 编译演示

```sh
make demos       # 编译 2 个演示程序
make run-demos   # 运行全部演示
```

## 单独编译（不使用 Makefile）

```sh
gcc -std=c99 -Iinclude -c src/domain_model.c -o domain_model.o
gcc -std=c99 -Iinclude -c src/workflow_engine.c -o workflow_engine.o
gcc -std=c99 -Iinclude -c src/rule_engine.c -o rule_engine.o
gcc -std=c99 -Iinclude -c src/state_machine.c -o state_machine.o
gcc -std=c99 -Iinclude -c src/approval_process.c -o approval_process.o
ar rcs libbusiness.a domain_model.o workflow_engine.o rule_engine.o state_machine.o approval_process.o

# 编译并链接示例
gcc -std=c99 -Iinclude examples/example1_domain_model.c -L. -lbusiness -o example1
./example1
```

## API 快速参考

### Domain Model

```c
#include "domain_model.h"

// 创建限界上下文
dm_bounded_context ctx;
dm_bounded_context_init(&ctx, "Sales");

// 添加术语
dm_glossary_add(&ctx.glossary, "Order", "A request to purchase products");

// 创建实体
dm_entity order_entity;
order_data data = {"ORD-001", 5000, "CREATED"};
dm_entity_init(&order_entity, 1001, "Order", &data, sizeof(data));

// 创建值对象
int amount = 5000;
dm_value_object money;
dm_value_object_init(&money, "Money", &amount, sizeof(int), NULL);

// 创建聚合根
dm_aggregate_root order_ar;
dm_aggregate_root_init(&order_ar, 2001, "Order", &data, sizeof(data));

// 发布领域事件
dm_domain_event ev;
dm_domain_event_init(&ev, 1, "OrderCreated", 2001, "Order", "payload", 7);
dm_aggregate_root_add_event(&order_ar, &ev);

// 配置上下文映射
dm_context_map cm;
dm_context_map_init(&cm, "Sales-Billing", "Sales", "Billing", DM_MAP_CUSTOMER_SUPPLIER);
dm_bounded_context_add_map(&ctx, &cm);
```

### Workflow Engine

```c
#include "workflow_engine.h"

// 初始化引擎
wf_engine eng;
wf_engine_init(&eng);

// 定义流程
wf_process_definition pd;
wf_process_definition_init(&pd, "SimpleProcess", "Simple Process");

// 添加节点
wf_node start, task, end;
wf_node_init(&start, 1, "Start", WF_NODE_START);
wf_node_init(&task, 2, "Approve", WF_NODE_MANUAL_TASK);
wf_node_init(&end, 3, "End", WF_NODE_END);

wf_node_add_outgoing(&start, 2);
wf_node_add_incoming(&task, 1);
wf_node_add_outgoing(&task, 3);
wf_node_add_incoming(&end, 2);

wf_process_definition_add_node(&pd, &start);
wf_process_definition_add_node(&pd, &task);
wf_process_definition_add_node(&pd, &end);

// 部署并启动
wf_engine_deploy(&eng, &pd);
uint64_t id = wf_engine_start(&eng, "SimpleProcess");

// 完成任务
wf_engine_complete_task(&eng, id, 2);

// 检查是否完成
wf_process_instance *pi = wf_engine_get_instance(&eng, id);
if (wf_process_instance_is_complete(pi)) {
    printf("Done!\n");
}
```

### Rule Engine

```c
#include "rule_engine.h"

// 初始化引擎
re_engine eng;
re_engine_init(&eng);

// 定义规则
re_rule rule;
re_rule_init(&rule, "HighValueDiscount", 10);

// 添加条件
re_condition cond;
re_condition_init(&cond, "amount", RE_OP_GT, 10000);
re_rule_add_condition(&rule, &cond);

// 添加动作
re_action action;
re_action_init_print(&action, "Apply 10% discount!");
re_rule_add_action(&rule, &action);

re_engine_add_rule(&eng, &rule);

// 插入事实并执行
re_fact fact;
re_fact_init(&fact, 1, "Order");
re_fact_set_field(&fact, "amount", 15000);
re_engine_add_fact(&eng, &fact);

re_engine_run(&eng);

// 查看审计
size_t count;
const re_audit_entry *audit = re_engine_get_audit(&eng, &count);
```

### State Machine

```c
#include "state_machine.h"

sm_state_machine sm;
sm_state_machine_init(&sm, NULL);

// 定义状态
sm_state s1, s2;
sm_state_init(&s1, 1, "Idle", SM_STATE_INITIAL);
sm_state_init(&s2, 2, "Active", SM_STATE_NORMAL);

sm_state_add_entry_action(&s1, my_entry_fn, my_ctx);

// 定义转换
sm_transition t;
sm_transition_init(&t, "start", 2);
sm_state_add_transition(&s1, &t);

sm_state_machine_add_state(&sm, &s1);
sm_state_machine_add_state(&sm, &s2);

// 启动并发送事件
sm_state_machine_start(&sm, 1);
sm_state_machine_send_event(&sm, "start");

printf("Current state: %d\n", sm_state_machine_get_current(&sm));
```

### Approval Process

```c
#include "approval_process.h"

ap_engine eng;
ap_engine_init(&eng);

// 创建审批
uint64_t id = ap_engine_create(&eng, "Expense Report");

// 构建审批级别
ap_approval_level l0;
ap_approval_level_init(&l0, 0, AP_LEVEL_SEQUENTIAL);

ap_approver mgr;
ap_approver_init(&mgr, "Wang Wei", "Manager", 10000);
ap_approval_level_add_approver(&l0, &mgr);

// 添加条件（金额 > 5000 需额外审批）
ap_condition cond;
ap_condition_init(&cond, AP_COND_AMOUNT_GT, 5000);
ap_approval_level_add_condition(&l0, &cond);

// 设置超时（24小时）
ap_approval_level_set_timeout(&l0, 86400000);

ap_approval_instance_add_level(ap_engine_get(&eng, id), &l0);

// 提交
ap_engine_submit(&eng, id, "employee_li", 7500, "Engineering");

// 审批
ap_approval_instance *ai = ap_engine_get(&eng, id);
ap_approval_instance_approve(ai, "Wang Wei");

// 查看审计
size_t count;
const ap_audit_entry *log = ap_approval_instance_get_audit(ai, &count);
```

## 集成示例

将五大模块串联使用时，典型模式为：

1. **领域模型**定义业务对象和规则边界
2. **规则引擎**根据业务事实确定处理路径（例如：金额分类）
3. **状态机**追踪业务对象生命周期
4. **工作流引擎**编排处理步骤
5. **审批流程**执行人工审批环节

参见 `demos/demo1_full_system.c` 了解完整集成示例。

## 容量配置

各模块最大容量可在对应头文件中调整：

| 模块 | 宏 | 默认值 |
|------|-----|--------|
| Domain Model | `DM_MAX_*` | 16-128 |
| Workflow | `WF_MAX_*` | 8-128 |
| Rule Engine | `RE_MAX_*` | 16-512 |
| State Machine | `SM_MAX_*` | 8-128 |
| Approval | `AP_MAX_*` | 16-512 |

## 注意事项

- 本库为教学/原型用途，未做生产级优化
- 所有数据结构为静态分配，注意容量限制
- 无线程安全保护
- 无持久化机制，重启后数据丢失
- 建议在实际项目中进行适配和扩展
