<script setup>
import { computed, ref, watch } from 'vue'

const props = defineProps({
  data: Object,
  currentStep: Number
})

// Whether to show split animation
const showSplitAnimation = ref(true)

// Split items
const splitItems = computed(() => props.data?.splitItems || [])

// Original items
const originalItems = computed(() => props.data?.items || [])

// Current step information
const currentStepInfo = computed(() => {
  if (!props.data || props.currentStep < 0) return null
  return props.data.steps[props.currentStep]
})

// Calculate current DP table state to display
const gridState = computed(() => {
  if (!props.data) return null
  
  const { capacity, steps } = props.data
  const rows = splitItems.value.length + 1
  const cols = capacity + 1
  
  const grid = Array.from({ length: rows }, () => 
    Array.from({ length: cols }, () => ({ value: 0, filled: false }))
  )
  
  // Initialize first row
  for (let j = 0; j <= capacity; j++) {
    grid[0][j].filled = true
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

// Get split item color (same color for same original item)
const getItemColor = (origIndex) => {
  const colors = [
    'var(--accent-cyan)',
    'var(--accent-purple)',
    'var(--accent-green)',
    'var(--accent-orange)',
    'var(--accent-blue)',
    'var(--accent-gold)'
  ]
  return colors[origIndex % colors.length]
}
</script>

<template>
  <div class="multiple-view" v-if="data">
    <!-- Split preview area -->
    <div class="split-preview">
      <div class="split-header">
        <span class="split-title">🔀 Binary Split</span>
        <span class="split-summary">
          {{ originalItems.length }} items → {{ splitItems.length }} sub-items
        </span>
      </div>
      
      <div class="split-container">
        <!-- Original items -->
        <div class="original-items">
          <div class="section-label">Original items</div>
          <div 
            v-for="(item, idx) in originalItems"
            :key="'orig-' + idx"
            class="orig-item"
            :style="{ borderColor: getItemColor(idx) }"
          >
            <span class="item-id">{{ idx + 1 }}</span>
            <span class="item-info">w={{ item.w }}, v={{ item.v }}</span>
            <span class="item-count">×{{ item.c }}</span>
          </div>
        </div>

        <!-- Split arrow -->
        <div class="split-arrow">
          <div class="arrow-line"></div>
          <div class="arrow-head">▶</div>
        </div>

        <!-- Split items -->
        <div class="split-items">
          <div class="section-label">Split result</div>
          <div class="split-groups">
            <div 
              v-for="(item, idx) in splitItems"
              :key="'split-' + idx"
              class="split-item"
              :style="{ 
                backgroundColor: getItemColor(item.orig) + '20',
                borderColor: getItemColor(item.orig)
              }"
            >
              <span class="item-badge" :style="{ background: getItemColor(item.orig) }">
                {{ item.orig + 1 }}-{{ item.cnt }}
              </span>
              <span class="item-detail">w={{ item.w }}, v={{ item.v }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Current step description -->
    <div class="step-info" v-if="currentStepInfo">
      <div class="step-badge">Step {{ currentStep + 1 }}</div>
      <div class="step-desc">
        Process sub-item 
        <span class="highlight-text" :style="{ color: getItemColor(splitItems[currentStepInfo.row - 1]?.orig) }">
          {{ splitItems[currentStepInfo.row - 1]?.orig + 1 }}-{{ splitItems[currentStepInfo.row - 1]?.cnt }}
        </span>
        capacity <span class="highlight-text">{{ currentStepInfo.col }}</span>
        = <span class="value-text">{{ currentStepInfo.val }}</span>
        <span :class="['decision-tag', currentStepInfo.decision]">
          {{ currentStepInfo.decision === 'take' ? '✓ Take' : '✗ Skip' }}
        </span>
      </div>
    </div>

    <!-- DP grid -->
    <div class="grid-wrapper">
      <div class="grid-header">
        <div class="corner-cell">Sub-item \ Capacity</div>
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
          class="grid-row"
        >
          <div 
            class="row-header"
            :style="i > 0 ? { borderLeftColor: getItemColor(splitItems[i-1]?.orig) } : {}"
          >
            <template v-if="i === 0">
              <div class="row-index">0</div>
              <div class="item-info">Initial</div>
            </template>
            <template v-else>
              <div class="row-index" :style="{ color: getItemColor(splitItems[i-1]?.orig) }">
                {{ splitItems[i-1]?.orig + 1 }}-{{ splitItems[i-1]?.cnt }}
              </div>
              <div class="item-info">w={{ splitItems[i-1]?.w }}, v={{ splitItems[i-1]?.v }}</div>
            </template>
          </div>

          <div 
            v-for="(cell, j) in row" 
            :key="j"
            :class="[...getCellClass(i, j), { 'on-path': isOnPath(i, j), 'filled': cell.filled }]"
          >
            <Transition name="pop" mode="out-in">
              <span :key="cell.value" class="cell-value">
                {{ cell.filled ? cell.value : '' }}
              </span>
            </Transition>
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
        <span>Without dependency</span>
      </div>
      <div class="legend-item">
        <span class="legend-color with"></span>
        <span>With dependency</span>
      </div>
      <div class="legend-item" v-if="data.path?.length > 0">
        <span class="legend-color path"></span>
        <span>Optimal path</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
.multiple-view {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

/* Split preview area */
.split-preview {
  background: var(--bg-secondary);
  border-radius: 10px;
  padding: 14px;
  border: 1px solid rgba(168, 85, 247, 0.3);
}

.split-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
}

.split-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--accent-purple);
}

.split-summary {
  font-size: 12px;
  color: var(--text-secondary);
  background: rgba(168, 85, 247, 0.2);
  padding: 3px 10px;
  border-radius: 12px;
}

.split-container {
  display: flex;
  align-items: flex-start;
  gap: 16px;
}

.section-label {
  font-size: 11px;
  color: var(--text-secondary);
  margin-bottom: 8px;
}

.original-items {
  flex: 0 0 auto;
}

.orig-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 10px;
  background: var(--bg-card);
  border-radius: 6px;
  border-left: 3px solid;
  margin-bottom: 6px;
  font-size: 12px;
}

.item-id {
  font-weight: 700;
  min-width: 20px;
}

.item-info {
  color: var(--text-secondary);
}

.item-count {
  font-weight: 600;
  color: var(--accent-orange);
}

.split-arrow {
  display: flex;
  align-items: center;
  padding: 20px 0;
}

.arrow-line {
  width: 30px;
  height: 2px;
  background: linear-gradient(90deg, var(--accent-purple), var(--accent-cyan));
}

.arrow-head {
  color: var(--accent-cyan);
  font-size: 16px;
  animation: pulse 1s ease-in-out infinite;
}

.split-items {
  flex: 1;
}

.split-groups {
  display: flex;
  flex-wrap: wrap;
  gap: 6px;
}

.split-item {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 4px 8px;
  border-radius: 6px;
  border: 1px solid;
  font-size: 11px;
}

.item-badge {
  padding: 1px 6px;
  border-radius: 4px;
  color: white;
  font-weight: 600;
  font-size: 10px;
}

.item-detail {
  color: var(--text-secondary);
}

/* 步骤信息 */
.step-info {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 10px 14px;
  background: var(--bg-secondary);
  border-radius: 8px;
  border: 1px solid var(--border-color);
}

.step-badge {
  background: linear-gradient(135deg, var(--accent-cyan), var(--accent-blue));
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 12px;
  font-weight: 600;
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

/* 网格样式 */
.grid-wrapper {
  overflow: auto;
}

.grid-header {
  display: flex;
  gap: 2px;
  margin-bottom: 2px;
}

.corner-cell {
  width: 90px;
  min-width: 90px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
  border-radius: 4px;
  font-size: 10px;
  color: var(--text-secondary);
}

.header-cell {
  width: 40px;
  min-width: 40px;
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
  width: 90px;
  min-width: 90px;
  min-height: 40px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
  border-radius: 4px;
  padding: 2px;
  border-left: 3px solid transparent;
}

.row-index {
  font-size: 11px;
  font-weight: 600;
}

.row-header .item-info {
  font-size: 9px;
  color: var(--text-secondary);
}

.cell {
  width: 40px;
  min-width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(17, 24, 39, 0.6);
  border: 1px solid var(--border-color);
  border-radius: 4px;
  font-size: 11px;
  font-weight: 600;
  transition: all 0.3s ease;
}

.cell.filled {
  background: rgba(34, 211, 238, 0.1);
}

.cell.current {
  background: rgba(34, 211, 238, 0.3);
  border-color: var(--accent-cyan);
  box-shadow: 0 0 15px rgba(34, 211, 238, 0.5);
  animation: glow 1s ease-in-out infinite;
}

.cell.highlight-without {
  background: rgba(249, 115, 22, 0.3);
  border-color: var(--accent-orange);
}

.cell.highlight-with {
  background: rgba(16, 185, 129, 0.3);
  border-color: var(--accent-green);
}

.cell.on-path {
  background: rgba(251, 191, 36, 0.3);
  border-color: var(--accent-gold);
}

.legend {
  display: flex;
  gap: 20px;
  justify-content: center;
  padding-top: 12px;
  border-top: 1px solid var(--border-color);
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
  50% { box-shadow: 0 0 25px rgba(34, 211, 238, 0.8); }
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

.pop-enter-active {
  animation: numberPop 0.3s ease-out;
}

@keyframes numberPop {
  0% { transform: scale(0); opacity: 0; }
  50% { transform: scale(1.3); }
  100% { transform: scale(1); opacity: 1; }
}
</style>

