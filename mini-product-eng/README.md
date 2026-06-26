# mini-product-eng — 产品工程 (C 语言实现)

C99 产品工程库，涵盖产品全生命周期管理：发现→定义→设计→开发→测试→发布→增长→成熟→衰退。

## 模块

| 头文件 | 功能 |
|---|---|
| `product_lifecycle.h` | 产品生命周期：阶段门禁、Go/No-Go、MVP 定义 |
| `roadmap_plan.h` | 路线图：Now/Next/Later、RICE 优先级、OKR 对齐 |
| `spec_design.h` | 规格设计：用户故事、PRD、技术规格、设计评审 |
| `stakeholder_mgmt.h` | 干系人管理：RACI 矩阵、权力×兴趣地图、沟通计划 |
| `release_mgmt.h` | 发布管理：语义版本、发布火车、功能开关、回滚计划 |

## 构建

```sh
make          # 构建静态库 libmini-product-eng.a
make examples # 构建所有示例
make demos    # 构建所有演示
make all      # 构建全部
make clean    # 清理
```

## 使用

```c
#include "product_lifecycle.h"
#include "roadmap_plan.h"
#include "spec_design.h"
#include "stakeholder_mgmt.h"
#include "release_mgmt.h"
```

## 许可证

MIT
