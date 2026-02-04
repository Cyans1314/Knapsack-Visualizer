<script setup>
import { computed } from 'vue'

const props = defineProps({
  data: Object,
  currentStep: Number,
  algorithm: String
})

// Determine if mixed knapsack (row color change)
const isMixedType = computed(() => props.data?.type === '混合背包')

// Determine if Kth optimal solution (display array)
const isKthType = computed(() => props.data?.type === '第K优解')

// Determine if 2D cost (need slider)
const is2DType = computed(() => props.data?.type === '二维费用')

// Determine if scheme counting type (display accumulation formula)
const isCountType = computed(() => props.data?.type === '方案计数')

// Determine if group knapsack (display group info)
const isGroupType = computed(() => props.data?.type === '分组背包')

// Determine if multiple knapsack (has split items)
const isMultipleType = computed(() => props.data?.type === '多重背包')

// Determine if complete knapsack (horizontal dependency)
const isCompleteType = computed(() => props.data?.type === '完全背包')

// Get row count (may differ by algorithm type)
const rowCount = computed(() => {
  if (!props.data) return 0
  if (isMultipleType.value && props.data.splitItems) {
    return props.data.splitItems.length + 1
  }
  if (isGroupType.value && props.data.groups) {
    return props.data.groups.length + 1
  }
  return props.data.items.length + 1
})

// Calculate current DP table state to display
const gridState = computed(() => {
  if (!props.data) return null
  
  const { capacity, steps } = props.data
  const rows = rowCount.value
  const cols = capacity + 1
  
  const grid = Array.from({ length: rows }, () => 
    Array.from({ length: cols }, () => ({ value: 0, filled: false }))
  )
  
  // Initialize first row
  for (let j = 0; j <= capacity; j++) {
    grid[0][j].filled = true
    // Scheme counting: dp[0][0] = 1
    if (isCountType.value && j === 0) {
      grid[0][j].value = 1
    }
  }
  
  // Fill based on current step
  for (let i = 0; i <= props.currentStep && i < steps.length; i++) {
    const step = steps[i]
    if (step.row < rows && step.col < cols) {
      grid[step.row][step.col].value = step.val
      grid[step.row][step.col].filled = true
    }
  }
  
  return grid
})

// Current step information
const currentStepInfo = computed(() => {
  if (!props.data || props.currentStep < 0) return null
  return props.data.steps[props.currentStep]
})

// Determine cell state
const getCellClass = (row, col) => {
  const classes = ['cell']
  const step = currentStepInfo.value
  
  if (!step) return classes
  
  if (step.row === row && step.col === col) {
    classes.push('current')
  }
  
  if (step.highlight) {
    for (const h of step.highlight) {
      if (h.r === row && h.c === col) {
        classes.push('highlight')
        classes.push(`highlight-${h.type}`)
      }
    }
  }
  
  return classes
}

// Check if on backtrack path
const isOnPath = (row, col) => {
  if (!props.data || props.currentStep < props.data.steps.length - 1) return false
  return props.data.path?.some(p => p.r === row && p.c === col)
}

// Get row title information
const getRowInfo = (index) => {
  if (index === 0) return { label: '0', desc: '初始', type: null }
  
  if (isMultipleType.value && props.data.splitItems) {
    const item = props.data.splitItems[index - 1]
    return { 
      label: `${index}`, 
      desc: `Item${item.orig + 1}×${item.cnt}`,
      extra: `w=${item.w}, v=${item.v}`,
      type: null
    }
  }
  
  if (isGroupType.value && props.data.groups) {
    const group = props.data.groups[index - 1]
    return { 
      label: `Group${group.id}`, 
      desc: `${group.items.length} items`,
      type: null
    }
  }
  
  const item = props.data.items[index - 1]
  
  // Mixed knapsack: return type info for color change
  if (isMixedType.value) {
    const typeNames = ['0/1', 'Complete', 'Multiple']
    return { 
      label: `${index}`, 
      desc: `w=${item.w}, v=${item.v}`,
      type: item.t,
      typeName: typeNames[item.t] || 'Unknown'
    }
  }
  
  return { 
    label: `${index}`, 
    desc: `w=${item.w}, v=${item.v}`,
    type: null
  }
}

// Get mixed knapsack row color class
const getRowTypeClass = (index) => {
  if (!isMixedType.value || index === 0) return ''
  const item = props.data.items[index - 1]
  if (item.t === 0) return 'type-01'
  if (item.t === 1) return 'type-complete'
  if (item.t === 2) return 'type-multiple'
  return ''
}

// Get Kth optimal solution cell display array
const getCellValues = (row, col) => {
  if (!isKthType.value) return null
  
  // Find corresponding step's vals array
  for (let i = props.currentStep; i >= 0; i--) {
    const step = props.data.steps[i]
    if (step.row === row && step.col === col && step.vals) {
      return step.vals
    }
  }
  return null
}

// Get decision description
const getDecisionText = (step) => {
  if (!step) return ''
  
  if (isCountType.value) {
    return step.formula || ''
  }
  
  if (isKthType.value) {
    return 'Merge sort'
  }
  
  if (isGroupType.value && step.bestChoice >= 0) {
    return `Select item${step.bestChoice + 1}`
  }
  
  if (isMixedType.value && step.typeStr) {
    return `[${step.typeStr}] ${step.decision === 'take' ? '✓ Select' : '✗ Skip'}`
  }
  
  return step.decision === 'take' ? '✓ Select' : '✗ Skip'
}

// Get decision style
const getDecisionClass = (step) => {
  if (!step) return ''
  if (isCountType.value) return 'formula'
  return step.decision === 'take' ? 'take' : 'skip'
}
</script>

<template>
  <div class="dp-grid-container" v-if="data">
    <!-- Current step description -->
    <div class="step-info" v-if="currentStepInfo">
      <div class="step-badge">Step {{ currentStep + 1 }}</div>
      <div class="step-desc">
        <span v-if="isGroupType">
          Calculate <span class="highlight-text">Group{{ currentStepInfo.groupId }}</span> capacity <span class="highlight-text">{{ currentStepInfo.col }}</span>
        </span>
        <span v-else>
          Calculate <span class="highlight-text">dp[{{ currentStepInfo.row }}][{{ currentStepInfo.col }}]</span>
        </span>
        = <span class="value-text">{{ currentStepInfo.val }}</span>
        <span class="decision-tag" :class="getDecisionClass(currentStepInfo)">
          {{ getDecisionText(currentStepInfo) }}
        </span>
      </div>
    </div>

    <!-- Multiple knapsack: display split information -->
    <div class="split-info" v-if="isMultipleType && data.splitItems">
      <span class="split-label">Binary split:</span>
      <span class="split-count">{{ data.items.length }} items → {{ data.splitItems.length }} sub-items</span>
    </div>

    <!-- Group knapsack: display group attempt -->
    <div class="group-try-info" v-if="isGroupType && currentStepInfo?.tryItems">
      <span class="try-label">Group attempt:</span>
      <div class="try-items">
        <span 
          v-for="(item, idx) in currentStepInfo.tryItems" 
          :key="idx"
          :class="['try-item', { 'best': currentStepInfo.bestChoice === item.itemIdx }]"
        >
          Item{{ item.itemIdx + 1 }}(w={{ item.w }},v={{ item.v }})
          <span v-if="item.canTake">→{{ item.newVal }}</span>
        </span>
      </div>
    </div>

    <!-- Scheme counting: display accumulation formula -->
    <div class="count-formula" v-if="isCountType && currentStepInfo">
      <div class="formula-box">
        <span class="formula-part from-without">
          {{ currentStepInfo.notTake }}
        </span>
        <span class="formula-op">+</span>
        <span class="formula-part from-with">
          {{ currentStepInfo.take }}
        </span>
        <span class="formula-op">=</span>
        <span class="formula-result">
          {{ currentStepInfo.val }}
        </span>
      </div>
      <div class="formula-desc">
        Skip({{ currentStepInfo.notTake }} ways) + Take({{ currentStepInfo.take }} ways) = {{ currentStepInfo.val }} ways
      </div>
    </div>

    <!-- DP grid -->
    <div class="grid-wrapper">
      <div class="grid-header">
        <div class="corner-cell">{{ isGroupType ? 'Group\\Capacity' : 'i \\ j' }}</div>
        <div 
          v-for="j in (data.capacity + 1)" 
          :key="j"
          class="header-cell"
        >
          {{ j - 1 }}
        </div>
      </div>

      <div class="grid-body">
        <div 
          v-for="(row, i) in gridState" 
          :key="i"
          :class="['grid-row', getRowTypeClass(i)]"
        >
          <div class="row-header">
            <div class="row-index">{{ getRowInfo(i).label }}</div>
            <div class="item-info">{{ getRowInfo(i).desc }}</div>
            <div class="item-extra" v-if="getRowInfo(i).extra">{{ getRowInfo(i).extra }}</div>
            <div class="type-badge" v-if="getRowInfo(i).typeName">{{ getRowInfo(i).typeName }}</div>
          </div>

          <div 
            v-for="(cell, j) in row" 
            :key="j"
            :class="[...getCellClass(i, j), { 'on-path': isOnPath(i, j), 'filled': cell.filled }]"
          >
            <!-- Kth optimal solution: display array -->
            <template v-if="isKthType && getCellValues(i, j)">
              <div class="cell-array">
                <span 
                  v-for="(v, idx) in getCellValues(i, j).slice(0, 3)" 
                  :key="idx"
                  class="array-item"
                >{{ v }}</span>
                <span v-if="getCellValues(i, j).length > 3" class="array-more">...</span>
              </div>
            </template>
            <!-- Normal display -->
            <template v-else>
              <Transition name="pop" mode="out-in">
                <span :key="cell.value" class="cell-value">
                  {{ cell.filled ? cell.value : '' }}
                </span>
              </Transition>
            </template>
          </div>
        </div>
      </div>
    </div>

    <!-- Legend -->
    <div class="legend">
      <div class="legend-item">
        <span class="legend-color current"></span>
        <span>Current calculation</span>
      </div>
      <div class="legend-item">
        <span class="legend-color without"></span>
        <span>{{ isCompleteType ? 'Inherit from above' : 'Without dependency' }}</span>
      </div>
      <div class="legend-item">
        <span class="legend-color with"></span>
        <span>{{ isCompleteType ? 'From left' : 'With dependency' }}</span>
      </div>
      <div class="legend-item" v-if="data.path?.length > 0">
        <span class="legend-color path"></span>
        <span>Optimal path</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
.dp-grid-container {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.step-info {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 10px 14px;
  background: var(--bg-secondary);
  border-radius: 8px;
  border: 1px solid var(--border-color);
  flex-wrap: wrap;
}

.step-badge {
  background: linear-gradient(135deg, var(--accent-cyan), var(--accent-blue));
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 12px;
  font-weight: 600;
  white-space: nowrap;
}

.step-desc {
  font-size: 13px;
  color: var(--text-secondary);
}

.highlight-text {
  color: var(--accent-cyan);
  font-weight: 600;
}

.value-text {
  color: var(--accent-gold);
  font-weight: 600;
  font-size: 15px;
}

.decision-tag {
  margin-left: 12px;
  padding: 2px 8px;
  border-radius: 4px;
  font-size: 11px;
}

.decision-tag.take {
  background: rgba(16, 185, 129, 0.2);
  color: var(--accent-green);
}

.decision-tag.skip {
  background: rgba(249, 115, 22, 0.2);
  color: var(--accent-orange);
}

.decision-tag.formula {
  background: rgba(168, 85, 247, 0.2);
  color: var(--accent-purple);
  font-family: 'Consolas', monospace;
}

/* 拆分信息 */
.split-info {
  font-size: 12px;
  color: var(--text-secondary);
  padding: 6px 12px;
  background: rgba(168, 85, 247, 0.1);
  border-radius: 6px;
  border: 1px solid rgba(168, 85, 247, 0.3);
}

.split-label {
  color: var(--accent-purple);
}

/* 分组尝试信息 */
.group-try-info {
  font-size: 12px;
  padding: 8px 12px;
  background: rgba(34, 211, 238, 0.1);
  border-radius: 6px;
  border: 1px solid rgba(34, 211, 238, 0.3);
}

.try-label {
  color: var(--accent-cyan);
  margin-right: 8px;
}

.try-items {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 6px;
}

.try-item {
  padding: 2px 8px;
  background: var(--bg-secondary);
  border-radius: 4px;
  color: var(--text-secondary);
}

.try-item.best {
  background: rgba(16, 185, 129, 0.3);
  color: var(--accent-green);
  font-weight: 600;
}

.grid-wrapper {
  overflow: auto;
}

.grid-header {
  display: flex;
  gap: 2px;
  margin-bottom: 2px;
}

.corner-cell {
  width: 80px;
  min-width: 80px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
  border-radius: 4px;
  font-size: 11px;
  color: var(--text-secondary);
}

.header-cell {
  width: 44px;
  min-width: 44px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
  border-radius: 4px;
  font-size: 11px;
  color: var(--accent-cyan);
  font-weight: 600;
}

.grid-body {
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.grid-row {
  display: flex;
  gap: 2px;
}

.row-header {
  width: 80px;
  min-width: 80px;
  min-height: 44px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
  border-radius: 4px;
  padding: 2px;
}

.row-index {
  font-size: 12px;
  color: var(--accent-purple);
  font-weight: 600;
}

.item-info {
  font-size: 9px;
  color: var(--text-secondary);
  text-align: center;
}

.item-extra {
  font-size: 8px;
  color: var(--text-secondary);
  opacity: 0.7;
}

.type-badge {
  font-size: 9px;
  padding: 1px 4px;
  border-radius: 3px;
  margin-top: 2px;
}

/* 混合背包行变色 */
.grid-row.type-01 .row-header {
  background: rgba(59, 130, 246, 0.3);
  border-left: 3px solid var(--accent-blue);
}
.grid-row.type-01 .type-badge {
  background: var(--accent-blue);
  color: white;
}

.grid-row.type-complete .row-header {
  background: rgba(168, 85, 247, 0.3);
  border-left: 3px solid var(--accent-purple);
}
.grid-row.type-complete .type-badge {
  background: var(--accent-purple);
  color: white;
}

.grid-row.type-multiple .row-header {
  background: rgba(16, 185, 129, 0.3);
  border-left: 3px solid var(--accent-green);
}
.grid-row.type-multiple .type-badge {
  background: var(--accent-green);
  color: white;
}

/* 第K优解数组显示 */
.cell-array {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1px;
  font-size: 9px;
}

.array-item {
  line-height: 1.2;
}

.array-item:first-child {
  font-weight: 700;
  color: var(--accent-gold);
}

.array-more {
  opacity: 0.5;
  font-size: 8px;
}

.cell {
  width: 44px;
  min-width: 44px;
  height: 44px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(17, 24, 39, 0.6);
  border: 1px solid var(--border-color);
  border-radius: 4px;
  font-size: 12px;
  font-weight: 600;
  transition: all 0.3s ease;
}

.cell.filled {
  background: rgba(34, 211, 238, 0.1);
}

.cell.current {
  background: rgba(34, 211, 238, 0.3);
  border-color: var(--accent-cyan);
  box-shadow: 0 0 15px rgba(34, 211, 238, 0.5), inset 0 0 10px rgba(34, 211, 238, 0.2);
  animation: glow 1s ease-in-out infinite;
}

.cell.highlight-without {
  background: rgba(249, 115, 22, 0.3);
  border-color: var(--accent-orange);
  box-shadow: 0 0 10px rgba(249, 115, 22, 0.4);
}

.cell.highlight-with {
  background: rgba(16, 185, 129, 0.3);
  border-color: var(--accent-green);
  box-shadow: 0 0 10px rgba(16, 185, 129, 0.4);
}

.cell.on-path {
  background: rgba(251, 191, 36, 0.3);
  border-color: var(--accent-gold);
  box-shadow: 0 0 15px rgba(251, 191, 36, 0.5);
}

.cell-value {
  display: inline-block;
}

.pop-enter-active {
  animation: numberPop 0.3s ease-out;
}

.pop-leave-active {
  animation: numberPop 0.2s ease-in reverse;
}

.legend {
  display: flex;
  gap: 20px;
  justify-content: center;
  padding-top: 12px;
  border-top: 1px solid var(--border-color);
  flex-wrap: wrap;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 11px;
  color: var(--text-secondary);
}

.legend-color {
  width: 14px;
  height: 14px;
  border-radius: 3px;
}

.legend-color.current {
  background: rgba(34, 211, 238, 0.5);
  border: 1px solid var(--accent-cyan);
}

.legend-color.without {
  background: rgba(249, 115, 22, 0.5);
  border: 1px solid var(--accent-orange);
}

.legend-color.with {
  background: rgba(16, 185, 129, 0.5);
  border: 1px solid var(--accent-green);
}

.legend-color.path {
  background: rgba(251, 191, 36, 0.5);
  border: 1px solid var(--accent-gold);
}

@keyframes glow {
  0%, 100% { box-shadow: 0 0 15px rgba(34, 211, 238, 0.5); }
  50% { box-shadow: 0 0 25px rgba(34, 211, 238, 0.8), 0 0 35px rgba(34, 211, 238, 0.4); }
}

@keyframes numberPop {
  0% { transform: scale(0); opacity: 0; }
  50% { transform: scale(1.3); }
  100% { transform: scale(1); opacity: 1; }
}

/* 方案计数公式样式 */
.count-formula {
  padding: 12px 16px;
  background: rgba(168, 85, 247, 0.1);
  border: 1px solid rgba(168, 85, 247, 0.3);
  border-radius: 8px;
  text-align: center;
}

.formula-box {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12px;
  font-size: 20px;
  font-weight: 700;
  margin-bottom: 8px;
}

.formula-part {
  padding: 4px 12px;
  border-radius: 6px;
  animation: flyIn 0.5s ease-out;
}

.formula-part.from-without {
  background: rgba(249, 115, 22, 0.3);
  color: var(--accent-orange);
}

.formula-part.from-with {
  background: rgba(16, 185, 129, 0.3);
  color: var(--accent-green);
}

.formula-op {
  color: var(--text-secondary);
}

.formula-result {
  background: linear-gradient(135deg, var(--accent-purple), var(--accent-blue));
  padding: 4px 16px;
  border-radius: 6px;
  color: white;
  animation: resultPop 0.5s ease-out 0.3s both;
}

.formula-desc {
  font-size: 12px;
  color: var(--text-secondary);
}

@keyframes flyIn {
  from { transform: translateY(-20px); opacity: 0; }
  to { transform: translateY(0); opacity: 1; }
}

@keyframes resultPop {
  0% { transform: scale(0); }
  50% { transform: scale(1.2); }
  100% { transform: scale(1); }
}
</style>

