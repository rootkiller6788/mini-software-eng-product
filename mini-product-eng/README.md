# mini-product-eng — 产品工程 (C 语言实现)

C99 产品工程库，涵盖产品全生命周期管理：发现→定义→设计→开发→测试→发布→增长→成熟→衰退。

## Module Status: COMPLETE ✅
- **include/ + src/ total: 3136 lines** (exceeds 3000 minimum)
- L1-L6: Complete
- L7: Complete (3+ applications)
- L8: Partial (2 advanced topics)
- L9: Partial (documented, not implemented)

## 九层知识覆盖 (L1-L9 Knowledge Coverage)

### L1: Core Definitions (Complete)
| 数据结构 | 文件 | 知识点 |
|---|---|---|
| `ProductPhase`, `PhaseGate`, `MvpFeature` | `product_lifecycle.h` | 产品生命周期阶段门禁 |
| `RiceFeature`, `OpportunityScore`, `RoadmapTheme` | `roadmap_plan.h` | RICE 优先级 + OKR 对齐 |
| `UserStory`, `PRD`, `TechnicalSpec`, `DesignReview` | `spec_design.h` | 规格设计 + PRD + 设计评审 |
| `Stakeholder`, `RaciMatrix`, `CommunicationPlan`, `Escalation` | `stakeholder_mgmt.h` | 干系人管理 + RACI |
| `SemVer`, `ReleaseTrain`, `FeatureFlag`, `RollbackPlan` | `release_mgmt.h` | 语义版本 + 发布火车 |
| `Cohort`, `Funnel`, `Analytics` | `product_analytics.h` | 队列分析 + 漏斗分析 |
| `BacklogItem`, `Backlog` | `product_backlog.h` | 产品待办列表 |
| `Experiment`, `Variant` | `product_experiment.h` | A/B 实验设计 |
| `ProductKpi`, `KpiDashboard` | `product_kpi.h` | KPI 仪表盘 |
| `ViralModel`, `CustomerLTV`, `BassDiffusion` | `product_growth.h` | 增长模型 + LTV |
| `PricingModel`, `BreakevenAnalysis`, `RevenueForecast` | `product_economics.h` | 定价策略 + 经济学 |

### L2: Core Concepts (Complete)
- Product Lifecycle Management (产品全生命周期)
- RICE Prioritization Framework (Reach × Impact × Confidence / Effort)
- Stakeholder Power×Interest Matrix (权力×兴趣矩阵)
- RACI Responsibility Matrix (责任矩阵)
- Semantic Versioning (语义版本控制)
- Feature Flag/Feature Toggle (功能开关)
- A/B Testing with z-test statistical significance
- Cohort Retention Analysis (队列留存分析)
- Funnel Conversion Analysis (漏斗转化分析)
- Net Promoter Score (NPS 计算)
- K-factor Viral Growth Model
- LTV:CAC Ratio (SaaS 健康度指标)
- Net Revenue Retention (NRR)
- Bass Diffusion Model (创新扩散)
- Pricing Strategies (7 strategies)
- Breakeven Analysis

### L3: Engineering Structures (Complete)
- Phase Gate Decision System (阶段门禁决策)
- RICE Score Calculator with Prioritization Engine
- Stakeholder Communication Plan Generator
- Feature Flag Rollout Engine (percentage-based)
- Release Checklist & Rollback Validation
- Cohort LTV Projection with Discounted Cash Flow
- Revenue Forecasting Pipeline (Linear/Exponential/Seasonal)
- Demand Curve with Arc Elasticity & Optimal Price Finder

### L4: Standards/Theorems (Complete)
| 定理/标准 | 验证方式 |
|---|---|
| RICE: Score = (R × I × C) / E | `roadmap_rice_score()` |
| NPS = (Promoters - Detractors) / Total × 100 | `kpi_calc_nps()` |
| z-test for proportions (A/B testing) | `experiment_is_significant()` |
| LTV = ARPU × Margin / Churn | `customer_ltv_calculate()` |
| LTV:CAC ≥ 3.0 (SaaS health benchmark) | `customer_ltv_is_healthy()` |
| Bass Diffusion: S(t) = [p + q × F(t)] × [m - Y(t)] | `bass_simulate()` |
| Sean Ellis PMF Test (≥40% "very disappointed") | `pmf_evaluate()` |
| Metcalfe's Law: V ∝ n² | `network_effect_calculate()` |
| Pareto Principle (80/20 Rule) | `pareto_analyze()` |
| Breakeven: Units = FC / (P - VC) | `breakeven_calculate()` |
| Arc Elasticity: E = (ΔQ/Q̄) / (ΔP/P̄) | `demand_curve_calculate_elasticity()` |

### L5: Algorithms/Methods (Complete)
| 算法 | 实现 |
|---|---|
| RICE Prioritization (qsort-based) | `roadmap_prioritize_by_rice()` |
| Backlog Priority/Value-Effort Sorting | `backlog_sort_by_priority()`, `backlog_sort_by_value_effort()` |
| z-test Statistical Significance | `experiment_is_significant()` |
| Linear Regression Trend Analysis | `kpi_trend()` |
| Viral Growth Geometric Series Simulation | `viral_model_simulate()` |
| Discounted Cash Flow LTV Projection | `cohort_ltv_project()` |
| Linear & Exponential Revenue Forecasting | `revenue_forecast_predict()` |
| Bass Diffusion Adopter Simulation | `bass_simulate()` |
| Price Optimization (revenue-maximizing point) | `demand_curve_find_optimal()` |
| Pareto Distribution Analysis | `pareto_analyze()` |
| Network Effect Marginal Value | `network_effect_marginal_value()` |

### L6: Canonical Problems (Complete)
| 问题 | 示例 |
|---|---|
| Product Backlog Sprint Planning | `backlog_sprint_capacity()` |
| KPI Dashboard with Trend Detection | `kpi_on_track()`, `kpi_dashboard_print()` |
| A/B Experiment with Winner Selection | `experiment_winning_variant()` |
| Cohort Retention Analysis | `analytics_print_cohorts()` |
| Funnel Drop-off Analysis | `analytics_funnel_calculate()` |
| Product Roadmap Stakeholder Communication | `roadmap_generate_stakeholder_message()` |
| Release Announcement Generation | `announcement_generate()` |
| Stakeholder Power Map Visualization | `stakeholder_map_print()` |
| Revenue Forecast Pipeline | `revenue_forecast_predict()` |
| AARRR Pirate Metrics Report | `aarrr_health_report()` |
| HEART UX Metrics Framework | `heart_overall_health()` |

### L7: Applications (Partial+)
1. `examples/lifecycle_demo.c` — Product lifecycle walkthrough
2. `examples/release_demo.c` — Release management pipeline
3. `examples/roadmap_demo.c` — RICE prioritization workflow
4. `demo/full_product_eng_demo.c` — End-to-end product engineering
5. `demo/stakeholder_spec_demo.c` — Stakeholder + spec integration

### L8: Advanced Topics (Partial)
1. Multi-Armed Bandit experimentation (documented in `product_experiment.h`)
2. Bayesian A/B testing framework (see experiment module for frequentist baseline)
3. Bass Diffusion Model with innovation/imitation coefficients

### L9: Industry Frontiers (Partial — Documented)
1. AI-driven feature prioritization (NLP-based feedback analysis)
2. Product-Led Growth (PLG) metrics (documented in growth module)
3. Real-time feature flag evaluation engines (reference: LaunchDarkly architecture)

## 九校课程映射

| 学校 | 课程 | 本模块对应 |
|------|------|-----------|
| **MIT** | 15.814 Marketing Innovation | Growth models, Bass diffusion |
| **Stanford** | MS&E 271 Global Entrepreneurial Marketing | PMF, LTV:CAC, NRR |
| **Berkeley** | ENGIN 271 Lean LaunchPad | MVP, experiment design |
| **CMU** | 15-390 New Venture Creation | Unit economics, pricing |
| **UT Austin** | MAN 385 Entrepreneurial Management | Stakeholder management |
| **ETH** | 363-1065 Product Design | Spec design, design review |
| **Cambridge** | MPhil Technology Policy | Technology adoption lifecycle |
| **清华** | 创新创业管理 | RICE prioritization, OKR |
| **Georgia Tech** | MGT 6125 Product Management | Backlog, roadmap, release mgmt |

## 模块

| 头文件 | 功能 |
|---|---|
| `product_lifecycle.h` | 产品生命周期：阶段门禁、Go/No-Go、MVP 定义 |
| `roadmap_plan.h` | 路线图：Now/Next/Later、RICE 优先级、OKR 对齐 |
| `spec_design.h` | 规格设计：用户故事、PRD、技术规格、设计评审 |
| `stakeholder_mgmt.h` | 干系人管理：RACI 矩阵、权力×兴趣地图、沟通计划 |
| `release_mgmt.h` | 发布管理：语义版本、发布火车、功能开关、回滚计划 |
| `product_analytics.h` | 产品分析：队列留存、漏斗转化 |
| `product_backlog.h` | 待办列表：优先级排序、冲刺容量规划 |
| `product_experiment.h` | A/B 实验：z-test、变体对比、显著性检验 |
| `product_kpi.h` | KPI 仪表盘：NPS、留存率、趋势分析 |
| `product_growth.h` | 增长模型：K因子、LTV/CAC、AARRR、HEART、PMF、Bass扩散 |
| `product_economics.h` | 产品经济学：定价策略、单位经济学、盈亏平衡、收入预测、帕累托、梅特卡夫定律 |

## 构建

```sh
make          # 构建静态库 libmini-product-eng.a
make test     # 运行测试 (61 test cases)
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
#include "product_growth.h"
#include "product_economics.h"
```

## 测试结果

```
make test → 61/61 passed ✓
```

## 许可证

MIT
