# mini-product-eng 使用指南

## 概述

`mini-product-eng` 是一个 C99 产品工程库，提供产品全生命周期管理的核心数据结构和算法。适用于嵌入式系统、工具开发和教育场景。

## 快速开始

```c
#include "product_lifecycle.h"
#include "roadmap_plan.h"
#include "spec_design.h"
#include "stakeholder_mgmt.h"
#include "release_mgmt.h"
```

编译链接：

```sh
make all
```

## 1. 产品生命周期管理

跟踪产品从构思到退市的完整过程，每个阶段有明确的准入/准出条件和门禁决策。

```c
ProductLifecycle app;
product_lifecycle_init(&app, "MyProduct", "Vision statement here");

// 定义 MVP 功能（MoSCoW 优先级）
product_lifecycle_add_mvp_feature(&app, "Core feature A", MVP_MUST_HAVE);
product_lifecycle_add_mvp_feature(&app, "Nice feature B", MVP_SHOULD_HAVE);

// 阶段门禁评估
product_lifecycle_evaluate_gate(&app, PHASE_DISCOVERY, true);
product_lifecycle_advance_phase(&app);

// 检查状态
printf("Phase: %s\n", product_lifecycle_phase_name(app.current_phase));
printf("MVP ready: %s\n", product_lifecycle_is_mvp_ready(&app) ? "Yes" : "No");
```

**阶段流程：**

```
Discovery → Definition → Design → Development → Testing → Launch → Growth → Maturity → Decline
```

每个阶段门禁都需要显式通过（Go/No-Go）才能前进。可以调用 `product_lifecycle_can_regress()` 检查是否允许回退。

## 2. 路线图与 RICE 优先级

使用 RICE 评分模型对功能进行量化优先级排序。

```c
ProductRoadmap roadmap;
roadmap_init(&roadmap, 2, 2026);

// 添加功能：Reach(用户数), Impact(0-3), Confidence(0-1), Effort(人月)
roadmap_add_feature(&roadmap, "AI search", "LLM-based search", 1000.0, 3.0, 0.8, 2.0);
roadmap_add_feature(&roadmap, "Dark mode", "System dark theme", 5000.0, 1.0, 1.0, 1.0);

// RICE = (Reach × Impact × Confidence) / Effort
roadmap_prioritize_by_rice(&roadmap);  // 排序 + 分配 NOW/NEXT/LATER
```

**时间范围分配：**
- **NOW**: RICE >= 5.0（当前季度承诺交付）
- **NEXT**: RICE >= 2.0（下个季度规划）
- **LATER**: RICE < 2.0（远期愿景）

**主题与 OKR：**

```c
roadmap_add_theme(&roadmap, "Platform Foundation", TIME_HORIZON_NOW);
const char *krs[] = {"99.9% uptime", "P95 < 100ms", "0 critical vulns"};
roadmap_set_okr(&roadmap.themes[0], "Build reliable platform", krs, 3);
```

**机会评分：** `score = importance + max(importance - satisfaction_gap, 0)` — 用于发现未满足需求。

**干系人沟通：** `roadmap_generate_stakeholder_message()` 生成适合分享的路线图摘要。

## 3. 规格与设计

### 用户故事

```c
UserStory us;
user_story_init(&us, "analyst", "import CSV data",
                "I can visualize data without coding");
user_story_add_acceptance(&us, "a CSV file is selected",
                          "I click Import", "a chart auto-generates");
```

### PRD（产品需求文档）

```c
PRD prd;
prd_init(&prd, "Feature X PRD", "Problem statement here");
// 设置 target_user, scope, out_of_scope, risks 等字段
prd_add_story(&prd, &us);
prd_print_summary(&prd);
```

### 技术规格

```c
TechnicalSpec ts;
memset(&ts, 0, sizeof(ts));

// API 端点
tech_spec_add_endpoint(&ts, "/api/v1/charts", "POST", 201);
tech_spec_add_endpoint(&ts, "/api/v1/charts/{id}", "GET", 200);

// 数据模型
tech_spec_add_model(&ts, "Chart");
tech_spec_add_model(&ts, "Dashboard");

// UI 原型
tech_spec_add_mockup(&ts, "Dashboard Home", "Grid layout with cards");
```

### 设计评审

10 项检查清单：需求清晰度、边界情况、错误处理、安全审查、无障碍、国际化、性能约束、依赖分析、向后兼容、监控计划。

```c
DesignReview dr;
design_review_init(&dr);
dr.requirements_clarity = true;
// ... 设置各项 ...
double score = design_review_score(&dr);  // 0.0 ~ 1.0
```

## 4. 干系人管理

### 权力×兴趣矩阵

```c
Stakeholder s;
stakeholder_init(&s, "CTO", "Chief Tech Officer", 0.9, 0.95);
// 自动分配到象限并设置参与策略
printf("Quadrant: %s\n", s.quadrant);  // "Key Players"
printf("Strategy: %s\n", s.engagement_strategy);
```

四个象限：
- **Key Players** (高权力×高兴趣): 密切合作
- **Meet Needs** (高权力×低兴趣): 保持满意
- **Show Consideration** (低权力×高兴趣): 保持知情
- **Monitor** (低权力×低兴趣): 最低投入

### RACI 矩阵

```c
RaciMatrix rm;
raci_matrix_init(&rm);
raci_add_item(&rm, "Approve PRD", "PM", "VP Product");
raci_add_consulted(&rm, 0, "CTO");
raci_add_informed(&rm, 0, "CEO");
```

### 沟通计划

根据干系人象限自动生成：

```c
CommunicationPlan plans[5];
comm_plan_generate(stakeholders, 5, plans);
// Key Players → Weekly 1:1, PM owner
// Meet Needs → Monthly email, PM owner
// Show Consideration → Bi-weekly newsletter, Marketing owner
// Monitor → Quarterly wiki page, EM owner
```

### 升级决策

```c
Escalation e;
escalation_init(&e, "Budget overrun 15%", 2);
escalation_resolve(&e, "Reallocated from Q3 buffer");
```

## 5. 发布管理

### 语义版本 (SemVer)

```c
SemVer v;
semver_parse("2.1.0-beta.1+build.42", &v);
semver_bump_minor(&v);  // → 2.2.0
semver_bump_major(&v);  // → 3.0.0

int cmp = semver_compare(&v1, &v2);  // -1, 0, or 1
```

### 发布火车

```c
ReleaseTrain rt;
release_train_init(&rt, "Sprint 42 Train", 1, 8, 0, 2);
release_train_lock(&rt);  // 锁定范围
bool ready = release_train_is_ready(&rt);
```

### 功能开关 (Feature Flags)

```c
FeatureFlag ff;
feature_flag_init(&ff, "new_ui", "alice", "Redesigned dashboard");
feature_flag_enable(&ff);
feature_flag_set_rollout(&ff, 25.0);  // 25% 灰度

// 基于 user_id 的一致性哈希分桶
bool active = feature_flag_is_active_for_user(&ff, 42);

// 紧急关闭
feature_flag_kill_switch(&ff);
```

### 发布检查清单

```c
ReleaseChecklistItem items[] = {
    {"Code freeze", true, "tech-lead", ""},
    {"QA sign-off", false, "qa", "5 tests failing"},
};
double progress = release_checklist_progress(items, 2);
```

### 回滚计划

```c
RollbackPlan rp;
rollback_plan_init(&rp,
    "P99 > 500ms for 5 min",
    "1. Kill switch\n2. Redeploy v1.0.9\n3. Rollback DB",
    20);
bool valid = rollback_plan_validate(&rp);
```

### 发布阶段与公告

阶段：Pre-Alpha → Alpha → Beta → RC → GA

```c
Announcement ann;
announcement_init(&ann, "v2.0 GA Release", "2026-06-15");
announcement_add_highlight(&ann, "New AI features");
announcement_generate(&ann, buffer, sizeof(buffer));
```

## 清理资源

```c
product_lifecycle_reset(&pl);   // 重置生命周期
roadmap_free(&roadmap);          // 释放路线图
spec_free_all(&prd, &ts);       // 释放 PRD 和技术规格
raci_free(&rm);                  // 释放 RACI 矩阵
```

## 构建选项

| 目标 | 命令 |
|---|---|
| 静态库 | `make lib` |
| 示例 | `make examples` |
| 演示 | `make demos` |
| 全部 | `make all` |
| 清理 | `make clean` |
