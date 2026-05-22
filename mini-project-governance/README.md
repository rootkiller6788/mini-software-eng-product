# mini-project-governance — 项目治理 (C 语言实现)

轻量级 C99 项目治理工具库，覆盖 Scrum、Kanban、Issue Tracking、OKR、Retrospective 五大模块。

## 模块概览

| 模块 | 头文件 | 功能 |
|------|--------|------|
| Agile Scrum | `agile_scrum.h` | Sprint 管理、站会、回顾、速率追踪、燃尽/燃起图 |
| Kanban Board | `kanban_board.h` | 看板列、WIP 限制、CFD、流动指标、瓶颈检测 |
| Project Tracker | `project_tracker.h` | Issue 追踪、自定义工作流、标签/组件、JQL 过滤器 |
| OKR Tracker | `okr_tracker.h` | 目标与关键结果、评分、周检视、对齐、CFR |
| Retrospective | `retrospective.h` | 回顾会议、行动项、健康检查、士气调查、心理安全评估 |

## 编译与运行

```sh
make all          # 编译库及所有示例
make demo_scrum   # 运行 Scrum 演示
make demo_kr      # 运行 Kanban + Retro 演示
make clean        # 清理构建产物
```

## API 文档

详见 [API.md](API.md) 和 [DESIGN.md](DESIGN.md)。

## 许可

MIT License
