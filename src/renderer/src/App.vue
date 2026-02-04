<!--
  Knapsack Problem Visualization System - Main Application Component
  
  @author Cyans
  @affiliation Chang'an University
  @description Main application interface including algorithm selection, parameter input, visualization display, and result presentation
-->

<script setup>
import { ref, computed, watch } from 'vue'
import InputPanel from './components/InputPanel.vue'
import DPGrid from './components/DPGrid.vue'
import GroupCircleView from './components/GroupCircleView.vue'
import TreeView from './components/TreeView.vue'
import MultipleKnapsackView from './components/MultipleKnapsackView.vue'
import TwoDimensionView from './components/TwoDimensionView.vue'
import DependencyView from './components/DependencyView.vue'
import ControlBar from './components/ControlBar.vue'
import InfoPanel from './components/InfoPanel.vue'

// Current selected algorithm
const currentAlgorithm = ref('knapsack_01')

// Watch algorithm change, clear demo
watch(currentAlgorithm, () => {
  handleClear()
})

// Algorithm list
const algorithms = [
  { value: 'knapsack_01', label: '0/1 背包', badge: '经典', desc: '每个物品只能选一次' },
  { value: 'knapsack_complete', label: '完全背包', badge: '无限', desc: '每个物品可以选无限次' },
  { value: 'knapsack_multiple', label: '多重背包', badge: '限量', desc: '每个物品有数量限制' },
  { value: 'knapsack_mixed', label: '混合背包', badge: '综合', desc: '物品类型混合（0/1、完全、多重）' },
  { value: 'knapsack_2d', label: '二维费用', badge: '双约束', desc: '同时考虑重量和体积两个限制' },
  { value: 'knapsack_group', label: '分组背包', badge: '互斥', desc: '每组物品最多选一个' },
  { value: 'knapsack_depend', label: '依赖背包', badge: '主附件', desc: '附件必须依赖主件才能选' },
  { value: 'knapsack_tree', label: '树形背包', badge: '树形DP', desc: '物品之间有树形依赖关系' },
  { value: 'knapsack_kth', label: '第K优解', badge: '多解', desc: '求第K大的解而非最优解' },
  { value: 'knapsack_count', label: '方案计数', badge: '计数', desc: '统计恰好装满的方案总数' }
]

// Current algorithm description
const currentAlgoInfo = computed(() => {
  return algorithms.find(a => a.value === currentAlgorithm.value)
})

// Determine which view component to use
const viewType = computed(() => {
  if (currentAlgorithm.value === 'knapsack_group') return 'circle'
  if (currentAlgorithm.value === 'knapsack_tree') return 'tree'
  if (currentAlgorithm.value === 'knapsack_multiple') return 'multiple'
  if (currentAlgorithm.value === 'knapsack_2d') return '2d'
  if (currentAlgorithm.value === 'knapsack_depend') return 'depend'
  return 'grid'
})

// Algorithm data
const algorithmData = ref(null)
const isRunning = ref(false)
const currentStep = ref(-1)
const isPlaying = ref(false)
const playSpeed = ref(1)

// Whether demo is complete
const isFinished = computed(() => {
  if (!algorithmData.value) return false
  return currentStep.value >= algorithmData.value.steps.length - 1
})

// Run algorithm
const handleRun = async (params) => {
  isRunning.value = true
  currentStep.value = -1
  algorithmData.value = null
  handlePause() // Stop previous playback first
  
  try {
    const result = await window.api.runAlgorithm(currentAlgorithm.value, params)
    if (result.success) {
      algorithmData.value = result.data
      currentStep.value = 0
      // Auto start playback
      setTimeout(() => {
        handlePlay()
      }, 300)
    } else {
      console.error('Algorithm execution failed:', result.error)
    }
  } catch (error) {
    console.error('Call failed:', error)
  } finally {
    isRunning.value = false
  }
}

// Playback control
let playTimer = null

const handlePlay = () => {
  if (!algorithmData.value) return
  isPlaying.value = true
  playNext()
}

const playNext = () => {
  if (!isPlaying.value || !algorithmData.value) return
  
  if (currentStep.value < algorithmData.value.steps.length - 1) {
    currentStep.value++
    playTimer = setTimeout(playNext, 500 / playSpeed.value)
  } else {
    isPlaying.value = false
  }
}

const handlePause = () => {
  isPlaying.value = false
  if (playTimer) {
    clearTimeout(playTimer)
    playTimer = null
  }
}

const handleStepForward = () => {
  if (!algorithmData.value) return
  if (currentStep.value < algorithmData.value.steps.length - 1) {
    currentStep.value++
  }
}

const handleStepBackward = () => {
  if (currentStep.value > 0) {
    currentStep.value--
  }
}

const handleReset = () => {
  handlePause()
  currentStep.value = 0
}

// Clear demo
const handleClear = () => {
  handlePause()
  algorithmData.value = null
  currentStep.value = -1
}

const handleSpeedChange = (speed) => {
  playSpeed.value = speed
}

const handleStepChange = (step) => {
  currentStep.value = step
}
</script>

<template>
  <div class="app-container">
    <!-- Top title bar -->
    <header class="app-header">
      <div class="header-left">
        <h1 class="title">
          <span class="icon">📦</span>
          背包问题可视化系统
        </h1>
      </div>
      
      <!-- Algorithm selector -->
      <div class="algorithm-selector">
        <span class="selector-label">Select algorithm:</span>
        <el-select v-model="currentAlgorithm" size="large" class="algo-select">
          <el-option
            v-for="algo in algorithms"
            :key="algo.value"
            :label="algo.label"
            :value="algo.value"
          >
            <div class="algo-option">
              <span>{{ algo.label }}</span>
              <span class="algo-badge">{{ algo.badge }}</span>
            </div>
          </el-option>
        </el-select>
        <!-- Algorithm description -->
        <div class="algo-desc" v-if="currentAlgoInfo">
          <span class="desc-icon">💡</span>
          {{ currentAlgoInfo.desc }}
        </div>
      </div>
    </header>

    <!-- Main content area -->
    <main class="app-main">
      <!-- Left: input panel -->
      <aside class="left-panel">
        <InputPanel 
          :is-running="isRunning"
          :algorithm="currentAlgorithm"
          @run="handleRun" 
        />
      </aside>

      <!-- Center: visualization area -->
      <section class="center-panel">
        <!-- Group knapsack: circle view -->
        <GroupCircleView 
          v-if="algorithmData && viewType === 'circle'"
          :data="algorithmData"
          :current-step="currentStep"
        />
        <!-- Tree knapsack: tree view -->
        <TreeView 
          v-else-if="algorithmData && viewType === 'tree'"
          :data="algorithmData"
          :current-step="currentStep"
        />
        <!-- Multiple knapsack: split view -->
        <MultipleKnapsackView 
          v-else-if="algorithmData && viewType === 'multiple'"
          :data="algorithmData"
          :current-step="currentStep"
        />
        <!-- 2D cost: slider view -->
        <TwoDimensionView 
          v-else-if="algorithmData && viewType === '2d'"
          :data="algorithmData"
          :current-step="currentStep"
        />
        <!-- Dependency knapsack: package view -->
        <DependencyView 
          v-else-if="algorithmData && viewType === 'depend'"
          :data="algorithmData"
          :current-step="currentStep"
        />
        <!-- Other algorithms: grid view -->
        <DPGrid 
          v-else-if="algorithmData"
          :data="algorithmData"
          :current-step="currentStep"
          :algorithm="currentAlgorithm"
        />
        <div v-else class="empty-state">
          <div class="empty-icon">🎯</div>
          <p>Enter parameters and click "Start Demo"</p>
        </div>
      </section>

      <!-- Right: result panel (show after demo complete) -->
      <aside class="right-panel" v-if="algorithmData">
        <InfoPanel 
          :data="algorithmData"
          :current-step="currentStep"
          :is-finished="isFinished"
        />
      </aside>
    </main>

    <!-- Bottom control bar -->
    <footer class="app-footer">
      <ControlBar
        :is-playing="isPlaying"
        :current-step="currentStep"
        :total-steps="algorithmData?.steps?.length || 0"
        :speed="playSpeed"
        :disabled="!algorithmData"
        :is-finished="isFinished"
        @play="handlePlay"
        @pause="handlePause"
        @step-forward="handleStepForward"
        @step-backward="handleStepBackward"
        @reset="handleReset"
        @clear="handleClear"
        @speed-change="handleSpeedChange"
        @step-change="handleStepChange"
      />
    </footer>
  </div>
</template>

<style scoped>
.app-container {
  display: flex;
  flex-direction: column;
  height: 100vh;
  padding: 16px;
  gap: 16px;
  position: relative;
  z-index: 1;
}

.app-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding-bottom: 12px;
  border-bottom: 1px solid var(--border-color);
  flex-wrap: wrap;
  gap: 16px;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 16px;
}

.title {
  font-size: 24px;
  font-weight: 600;
  background: linear-gradient(135deg, var(--accent-cyan), var(--accent-purple));
  -webkit-background-clip: text;
  background-clip: text;
  -webkit-text-fill-color: transparent;
  display: flex;
  align-items: center;
  gap: 8px;
}

.icon {
  -webkit-text-fill-color: initial;
}

.subtitle {
  color: var(--text-secondary);
  font-size: 14px;
}

.algorithm-selector {
  display: flex;
  align-items: center;
  gap: 12px;
}

.selector-label {
  color: var(--text-secondary);
  font-size: 14px;
}

.algo-select {
  width: 220px;
}

.algo-option {
  display: flex;
  justify-content: space-between;
  align-items: center;
  width: 100%;
}

.algo-badge {
  font-size: 11px;
  padding: 2px 6px;
  background: linear-gradient(135deg, var(--accent-cyan), var(--accent-blue));
  border-radius: 4px;
  color: white;
}

.algo-desc {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  background: rgba(34, 211, 238, 0.1);
  border: 1px solid rgba(34, 211, 238, 0.3);
  border-radius: 8px;
  font-size: 13px;
  color: var(--text-secondary);
}

.desc-icon {
  font-size: 14px;
}

.app-main {
  flex: 1;
  display: flex;
  gap: 16px;
  overflow: hidden;
  min-height: 0;
}

.left-panel {
  width: 280px;
  min-width: 280px;
  display: flex;
  flex-direction: column;
  overflow-y: auto;
}

.center-panel {
  flex: 1;
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: 12px;
  padding: 20px;
  overflow: auto;
  display: flex;
  align-items: flex-start;
  justify-content: center;
  min-width: 0;
}

.right-panel {
  width: 260px;
  min-width: 260px;
  display: flex;
  flex-direction: column;
  overflow-y: auto;
}

.empty-state {
  text-align: center;
  color: var(--text-secondary);
  align-self: center;
}

.empty-icon {
  font-size: 64px;
  margin-bottom: 16px;
  opacity: 0.5;
}

.app-footer {
  padding-top: 8px;
  border-top: 1px solid var(--border-color);
}

/* 响应式布局 */
@media (max-width: 1200px) {
  .right-panel {
    width: 220px;
    min-width: 220px;
  }
}

@media (max-width: 900px) {
  .app-main {
    flex-direction: column;
  }
  
  .left-panel,
  .right-panel {
    width: 100%;
    min-width: unset;
    max-height: 200px;
  }
  
  .center-panel {
    min-height: 300px;
  }
}
</style>
