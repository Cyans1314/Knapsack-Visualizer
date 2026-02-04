# 背包问题可视化系统

一个基于 Electron + Vue 3 + C++ 的动态规划算法可视化演示系统，支持 10 种经典背包问题算法。

## 功能特性

- **10 种背包算法可视化**：0/1 背包、完全背包、多重背包、混合背包、二维费用、分组背包、依赖背包、树形背包、第K优解、方案计数
- **实时动画演示**：逐步展示 DP 状态转移过程
- **交互式界面**：支持自定义输入、预设案例、随机生成
- **性能分析**：显示算法复杂度和执行时间

## 环境要求

- **Node.js** >= 16.0
- **npm** >= 8.0
- **g++** (用于编译 C++ 算法)
  - Windows: 安装 [MinGW](https://www.mingw-w64.org/) 或 [TDM-GCC](https://jmeubank.github.io/tdm-gcc/)
  - macOS: `xcode-select --install`
  - Linux: `sudo apt-get install build-essential`

## 快速开始

### 1. 安装依赖

```bash
npm install
```

### 2. 编译 C++ 算法

```bash
cd cpp
# Windows
.\build.bat

# macOS/Linux
./build.sh
```

### 3. 开发模式运行

```bash
npm run dev
```

### 4. 打包发布

```bash
# Windows
npm run build:win

# macOS
npm run build:mac

# Linux
npm run build:linux
```

打包完成后，安装程序在 `dist/` 目录下。

## 推荐开发工具

- [VSCode](https://code.visualstudio.com/)
- [ESLint](https://marketplace.visualstudio.com/items?itemName=dbaeumer.vscode-eslint)
- [Prettier](https://marketplace.visualstudio.com/items?itemName=esbenp.prettier-vscode)
- [Volar](https://marketplace.visualstudio.com/items?itemName=Vue.volar)

## 项目结构

```
├── src/
│   ├── main/
│   │   └── index.js                    # Electron 主进程（IPC通信、C++调用）
│   ├── preload/
│   │   └── index.js                    # 预加载脚本（安全上下文）
│   └── renderer/
│       ├── index.html                  # 应用入口 HTML
│       └── src/
│           ├── App.vue                 # 主应用组件
│           ├── main.js                 # Vue 应用初始化
│           ├── assets/                 # 静态资源
│           └── components/             # Vue 组件
│               ├── InputPanel.vue      # 输入参数面板
│               ├── ControlBar.vue      # 播放控制栏
│               ├── InfoPanel.vue       # 结果信息面板
│               ├── DPGrid.vue          # DP 表格视图（0/1、完全、混合、方案计数）
│               ├── TreeView.vue        # 树形背包可视化
│               ├── GroupCircleView.vue # 分组背包圆环视图
│               ├── DependencyView.vue  # 依赖背包套餐视图
│               ├── MultipleKnapsackView.vue  # 多重背包拆分视图
│               └── TwoDimensionView.vue      # 二维费用切片视图
├── cpp/
│   ├── build.bat                       # Windows 编译脚本
│   ├── knapsack_01.cpp                 # 0/1 背包算法
│   ├── knapsack_complete.cpp           # 完全背包算法
│   ├── knapsack_multiple.cpp           # 多重背包算法（二进制拆分）
│   ├── knapsack_mixed.cpp              # 混合背包算法
│   ├── knapsack_2d.cpp                 # 二维费用背包算法
│   ├── knapsack_group.cpp              # 分组背包算法
│   ├── knapsack_depend.cpp             # 依赖背包算法
│   ├── knapsack_tree.cpp               # 树形背包算法
│   ├── knapsack_kth.cpp                # 第K优解算法
│   └── knapsack_count.cpp              # 方案计数算法
├── build/                              # 构建资源
│   ├── entitlements.mac.plist          # macOS 权限配置
│   ├── icon.icns                       # macOS 图标
│   ├── icon.ico                        # Windows 图标
│   └── icon.png                        # 通用图标
├── resources/
│   └── icon.png                        # 应用图标
├── package.json                        # 项目依赖配置
├── electron-builder.yml                # Electron 打包配置
├── electron.vite.config.mjs            # Vite 构建配置
├── eslint.config.mjs                   # ESLint 配置
└── README.md                           # 项目说明
```

## 技术栈

- **前端**：Vue 3 + Vite + Element Plus
- **桌面**：Electron 39
- **算法**：C++ 17
- **构建**：electron-vite + electron-builder

## 许可证

MIT
