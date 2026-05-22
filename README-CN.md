# Mini Software Eng Product（迷你软件工程与产品管理）

**从零开始、零依赖的 C 语言实现**，涵盖软件工程实践、项目管理、架构管理和产品开发。每个模块以教学级精度建模软件工程核心实践 — 从架构决策记录和代码审查到 Git 工作流、敏捷项目管理和软件测试框架。

## 模块总览

| 模块 | 主题 | 参考标准 |
|--------|--------|----------------|
| [mini-arch-management](mini-arch-management/) | 架构决策记录（ADR）、C4 模型、架构评审、技术债务追踪 | C4 Model, ADR GitHub |
| [mini-business-system](mini-business-system/) | 领域建模、工作流引擎、规则引擎、状态机、审批流程 | Camunda, Drools |
| [mini-eng-quality](mini-eng-quality/) | 代码审查、SonarQube 仿真、圈复杂度、技术债务、重构 | SonarQube, Clean Code |
| [mini-git-workflow](mini-git-workflow/) | Git 内部原理（对象/引用）、分支模型、合并/变基、PR 审查、约定式提交 | Pro Git, Conventional Commits |
| [mini-product-eng](mini-product-eng/) | PRD→规格→设计→实现→测试→发布全周期、路线图、干系人管理 | SVPG Product, Shape Up |
| [mini-project-governance](mini-project-governance/) | 敏捷（Scrum/Kanban）、类 Jira 追踪、团队速率、OKR 追踪、回顾会议 | Scrum Guide, SAFe |
| [mini-software-testing](mini-software-testing/) | 单元测试框架、Mock/Stub、TDD、BDD、端到端测试、契约测试、变异测试 | JUnit, Jest, Pact |

## 设计理念

- **零外部依赖** — 纯 C（C99/C11），仅使用 `libc` 和 `libm`
- **模块自包含** — 每个目录自带 `Makefile`、`include/`、`src/`、`examples/`、`demos/`、`tests/`
- **用户态工程仿真** — 对软件工程工具链、项目管理和产品开发流程的教学级建模
- **理论到代码的映射** — 每个模块包含 `docs/` 目录，内有实践标准对齐说明
- **实用演示程序** — Git 内部对象浏览器、工作流引擎、代码质量分析器、测试框架、项目看板等

## 构建方式

每个模块相互独立。进入模块目录后运行：

```bash
cd mini-git-workflow
make all    # 构建全部
make test   # 运行测试
```

需要 **GCC** 和 **GNU Make**。

## 项目结构

```
mini-software-eng-product/
├── mini-arch-management/        # 架构管理
├── mini-business-system/        # 业务系统
├── mini-eng-quality/            # 工程质量
├── mini-git-workflow/           # Git 工作流
├── mini-product-eng/            # 产品工程
├── mini-project-governance/     # 项目治理
└── mini-software-testing/       # 软件测试
```

## 许可证

MIT
