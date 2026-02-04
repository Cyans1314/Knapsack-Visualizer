<script setup>
import { computed, ref } from 'vue'

const props = defineProps({
  data: Object,
  currentStep: Number
})

// Current step information
const currentStepInfo = computed(() => {
  if (!props.data || props.currentStep < 0) return null
  return props.data.steps[props.currentStep]
})

// Original items
const originalItems = computed(() => props.data?.items || [])

// Package list
const packages = computed(() => props.data?.packages || [])

// Main item list
const mainItems = computed(() => {
  return originalItems.value
    .map((item, idx) => ({ ...item, idx }))
    .filter(item => item.p === 0)
})

// Get attachments for main item
const getAttachments = (mainIdx) => {
  return originalItems.value
    .map((item, idx) => ({ ...item, idx }))
    .filter(item => item.p === mainIdx + 1)
}

// Calculate current DP table state to display
const gridState = computed(() => {
  if (!props.data) return null
  
  const { capacity, steps } = props.data
  const rows = packages.value.length + 1
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

// Get package color
const getPackageColor = (pkgIdx) => {
  const colors = [
    'var(--accent-cyan)',
    'var(--accent-purple)',
    'var(--accent-green)',
    'var(--accent-orange)',
    'var(--accent-blue)',
    'var(--accent-gold)'
  ]
  // Assign color based on main item in package
  const pkg = packages.value[pkgIdx]
  if (pkg?.items?.length > 0) {
    const mainItem = pkg.items[0]
    return colors[mainItem % colors.length]
  }
  return colors[pkgIdx % colors.length]
}
</script>

<template>
  <div class="dependency-view" v-if="data">
    <!-- Dependency relationship preview -->
    <div class="dependency-preview">
      <div class="preview-header">
        <span class="preview-title">🔗 Main-Attachment Relationship</span>
      </div>
      
      <div class="dependency-groups">
        <div 
          v-for="main in mainItems"
          :key="'main-' + main.idx"
          class="dep-group"
        >
          <!-- Main item -->
          <div class="main-item">
            <span class="item-badge main">Main{{ main.idx + 1 }}</span>
            <span class="item-detail">w={{ main.w }}, v={{ main.v }}</span>
          </div>
          
          <!-- Attachments -->
          <div class="attachments" v-if="getAttachments(main.idx).length > 0">
            <div class="attach-line"></div>
            <div class="attach-items">
              <div 
                v-for="att in getAttachments(main.idx)"
                :key="'att-' + att.idx"
                class="attach-item"
              >
                <span class="item-badge attach">Attach{{ att.idx + 1 }}</span>
                <span class="item-detail">w={{ att.w }}, v={{ att.v }}</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Package combination preview -->
    <div class="package-preview">
      <div class="preview-header">
        <span class="preview-title">📦 Package Combinations</span>
        <span class="preview-summary">Total {{ packages.length }} combinations</span>
      </div>
      
      <div class="package-list">
        <div 
          v-for="(pkg, idx) in packages"
          :key="'pkg-' + idx"
          :class="['package-item', { active: currentStepInfo?.row === idx + 1 }]"
          :style="{ borderColor: getPackageColor(idx) }"
        >
          <span class="pkg-index" :style="{ background: getPackageColor(idx) }">{{ idx + 1 }}</span>
          <span class="pkg-desc">{{ pkg.desc }}</span>
          <span class="pkg-stats">w={{ pkg.w }}, v={{ pkg.v }}</span>
        </div>
      </div>
    </div>

    <!-- Current step description -->
    <div class="step-info" v-if="currentStepInfo">
      <div class="step-badge">Step {{ currentStep + 1 }}</div>
      <div class="step-desc">
        Process package 
        <span class="highlight-text" :style="{ color: getPackageColor(currentStepInfo.row - 1) }">
          {{ currentStepInfo.package }}
        </span>
        capacity <span class="highlight-text">{{ currentStepInfo.col }}</span>
        = <span class="value-text">{{ currentStepInfo.val }}</span>
        <span :class="['decision-tag', currentStepInfo.decision]">
          {{ currentStepInfo.decision === 'take' ? '✓ Select' : '✗ Skip' }}
        </span>
      </div>
    </div>

    <!-- DP grid -->
    <div class="grid-wrapper">
      <div class="grid-header">
        <div class="corner-cell">Package \ Capacity</div>
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
            :style="i > 0 ? { borderLeftColor: getPackageColor(i - 1) } : {}"
          >
            <template v-if="i === 0">
              <div class="row-index">0</div>
              <div class="pkg-info">Initial</div>
            </template>
            <template v-else>
              <div class="row-index" :style="{ color: getPackageColor(i - 1) }">{{ i }}</div>
              <div class="pkg-info">{{ packages[i - 1]?.desc }}</div>
              <div class="pkg-stats-small">w={{ packages[i - 1]?.w }}, v={{ packages[i - 1]?.v }}</div>
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
.dependency-view {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

/* 依赖关系预览 */
.dependency-preview {
  background: var(--bg-secondary);
  border-radius: 10px;
  padding: 14px;
  border: 1px solid rgba(34, 211, 238, 0.3);
}

.preview-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
}

.preview-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--accent-cyan);
}

.preview-summary {
  font-size: 12px;
  color: var(--text-secondary);
  background: rgba(34, 211, 238, 0.2);
  padding: 3px 10px;
  border-radius: 12px;
}

.dependency-groups {
  display: flex;
  flex-wrap: wrap;
  gap: 16px;
}

.dep-group {
  background: var(--bg-card);
  border-radius: 8px;
  padding: 10px;
  min-width: 150px;
}

.main-item {
  display: flex;
  flex-direction: column;
  gap: 4px;
  padding-bottom: 8px;
}

.item-badge {
  padding: 2px 8px;
  border-radius: 4px;
  font-size: 11px;
  font-weight: 600;
}

.item-badge.main {
  background: var(--accent-cyan);
  color: white;
}

.item-badge.attach {
  background: var(--accent-purple);
  color: white;
}

.item-detail {
  font-size: 11px;
  color: var(--text-secondary);
}

.attachments {
  display: flex;
  gap: 8px;
  padding-top: 8px;
  border-top: 1px dashed var(--border-color);
}

.attach-line {
  width: 2px;
  background: linear-gradient(180deg, var(--accent-cyan), var(--accent-purple));
  border-radius: 1px;
}

.attach-items {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.attach-item {
  display: flex;
  flex-direction: column;
  gap: 2px;
}

/* 套餐预览 */
.package-preview {
  background: var(--bg-secondary);
  border-radius: 10px;
  padding: 14px;
  border: 1px solid rgba(168, 85, 247, 0.3);
}

.package-list {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.package-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 10px;
  background: var(--bg-card);
  border-radius: 6px;
  border-left: 3px solid;
  font-size: 12px;
  transition: all 0.3s;
}

.package-item.active {
  background: rgba(34, 211, 238, 0.2);
  box-shadow: 0 0 10px rgba(34, 211, 238, 0.3);
}

.pkg-index {
  width: 20px;
  height: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  color: white;
  font-size: 10px;
  font-weight: 700;
}

.pkg-desc {
  color: var(--text-primary);
  font-weight: 500;
}

.pkg-stats {
  color: var(--text-secondary);
  font-size: 11px;
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
  flex-wrap: wrap;
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
  width: 120px;
  min-width: 120px;
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
  width: 120px;
  min-width: 120px;
  min-height: 44px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
  border-radius: 4px;
  padding: 4px;
  border-left: 3px solid transparent;
}

.row-index {
  font-size: 12px;
  font-weight: 600;
}

.pkg-info {
  font-size: 9px;
  color: var(--text-secondary);
  text-align: center;
  max-width: 110px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.pkg-stats-small {
  font-size: 8px;
  color: var(--text-secondary);
  opacity: 0.7;
}

.cell {
  width: 40px;
  min-width: 40px;
  height: 44px;
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

.pop-enter-active {
  animation: numberPop 0.3s ease-out;
}

@keyframes numberPop {
  0% { transform: scale(0); opacity: 0; }
  50% { transform: scale(1.3); }
  100% { transform: scale(1); opacity: 1; }
}
</style>

