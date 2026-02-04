<script setup>
import { computed, ref, watch } from 'vue'

const props = defineProps({
  data: Object,
  currentStep: Number
})

// Current selected volume slice
const currentVolume = ref(0)

// Initialize volume to maximum value
watch(() => props.data, (newData) => {
  if (newData?.capacity2) {
    currentVolume.value = newData.capacity2
  }
}, { immediate: true })

// Current step information
const currentStepInfo = computed(() => {
  if (!props.data || props.currentStep < 0) return null
  return props.data.steps[props.currentStep]
})

// Auto-follow current step volume
watch(() => currentStepInfo.value, (step) => {
  if (step?.vol !== undefined) {
    currentVolume.value = step.vol
  }
})

// Calculate current volume slice DP table state
const gridState = computed(() => {
  if (!props.data) return null
  
  const { capacity, capacity2, steps, items } = props.data
  const rows = items.length + 1
  const cols = capacity + 1
  const vol = currentVolume.value
  
  const grid = Array.from({ length: rows }, () => 
    Array.from({ length: cols }, () => ({ value: 0, filled: false }))
  )
  
  // Initialize first row
  for (let j = 0; j <= capacity; j++) {
    grid[0][j].filled = true
  }
  
  // Fill based on current step (only show current volume slice values)
  for (let i = 0; i <= props.currentStep && i < steps.length; i++) {
    const step = steps[i]
    if (step.vol === vol && step.row < rows && step.col < cols) {
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
  
  if (!step || step.vol !== currentVolume.value) return classes
  
  if (step.row === row && step.col === col) {
    classes.push('current')
  }
  
  if (step.highlight) {
    for (const h of step.highlight) {
      if (h.r === row && h.c === col && (h.v === undefined || h.v === currentVolume.value)) {
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
  return props.data.path?.some(p => p.r === row && p.c === col && p.v === currentVolume.value)
}

// Get item information
const getItemInfo = (index) => {
  if (index === 0) return { label: '0', desc: 'Initial' }
  const item = props.data.items[index - 1]
  return { 
    label: `${index}`, 
    desc: `w=${item.w}, m=${item.m}, v=${item.v}`
  }
}
</script>

<template>
  <div class="two-dimension-view" v-if="data">
    <!-- Dimension information -->
    <div class="dimension-info">
      <div class="dim-item">
        <span class="dim-icon">📦</span>
        <span class="dim-label">Weight capacity:</span>
        <span class="dim-value">{{ data.capacity }}</span>
      </div>
      <div class="dim-item">
        <span class="dim-icon">📐</span>
        <span class="dim-label">Volume capacity:</span>
        <span class="dim-value">{{ data.capacity2 }}</span>
      </div>
    </div>

    <!-- Current volume display -->
    <div class="volume-display">
      <span class="vol-label">Current volume slice:</span>
      <span class="vol-value">{{ currentVolume }}</span>
      <span class="vol-hint">(Auto-follow animation)</span>
    </div>

    <!-- Current step description -->
    <div class="step-info" v-if="currentStepInfo">
      <div class="step-badge">Step {{ currentStep + 1 }}</div>
      <div class="step-desc">
        Calculate <span class="highlight-text">dp[{{ currentStepInfo.row }}][{{ currentStepInfo.col }}][{{ currentStepInfo.vol }}]</span>
        = <span class="value-text">{{ currentStepInfo.val }}</span>
        <span :class="['decision-tag', currentStepInfo.decision]">
          {{ currentStepInfo.decision === 'take' ? '✓ Take' : '✗ Skip' }}
        </span>
      </div>
      <div class="step-hint" v-if="currentStepInfo.vol !== currentVolume">
        <span class="hint-icon">💡</span>
        Current step is at volume {{ currentStepInfo.vol }}, click to switch view
        <el-button size="small" @click="currentVolume = currentStepInfo.vol">
          Switch to {{ currentStepInfo.vol }}
        </el-button>
      </div>
    </div>

    <!-- 3D visual hint -->
    <div class="cube-hint">
      <div class="cube-visual">
        <div class="cube-layer" v-for="v in Math.min(data.capacity2 + 1, 5)" :key="v"
          :class="{ active: currentVolume === v - 1 }"
          :style="{ transform: `translateZ(${(v - 1) * 8}px) translateY(${(v - 1) * -3}px)` }"
        ></div>
      </div>
      <span class="cube-label">Volume dimension</span>
    </div>

    <!-- DP grid (current volume slice) -->
    <div class="grid-wrapper">
      <div class="grid-title">
        dp[i][j][<span class="vol-highlight">{{ currentVolume }}</span>] slice
      </div>
      
      <div class="grid-header">
        <div class="corner-cell">Item \ Weight</div>
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
          <div class="row-header">
            <div class="row-index">{{ getItemInfo(i).label }}</div>
            <div class="item-info">{{ getItemInfo(i).desc }}</div>
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
    </div>
  </div>
</template>

<style scoped>
.two-dimension-view {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

/* Dimension information */
.dimension-info {
  display: flex;
  gap: 24px;
  padding: 12px 16px;
  background: var(--bg-secondary);
  border-radius: 8px;
  border: 1px solid var(--border-color);
}

.dim-item {
  display: flex;
  align-items: center;
  gap: 8px;
}

.dim-icon {
  font-size: 18px;
}

.dim-label {
  color: var(--text-secondary);
  font-size: 13px;
}

.dim-value {
  font-weight: 700;
  font-size: 16px;
  color: var(--accent-cyan);
}

/* Volume slider */
.volume-slider {
  background: linear-gradient(135deg, rgba(168, 85, 247, 0.1), rgba(34, 211, 238, 0.1));
  border: 1px solid rgba(168, 85, 247, 0.3);
  border-radius: 10px;
  padding: 14px;
}

.slider-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}

.slider-label {
  font-size: 13px;
  color: var(--text-secondary);
}

.slider-value {
  font-size: 20px;
  font-weight: 700;
  color: var(--accent-purple);
  background: rgba(168, 85, 247, 0.2);
  padding: 2px 12px;
  border-radius: 8px;
}

.slider-container {
  display: flex;
  align-items: center;
  gap: 12px;
}

.slider-min, .slider-max {
  font-size: 12px;
  color: var(--text-secondary);
  min-width: 20px;
  text-align: center;
}

.volume-range {
  flex: 1;
}

.volume-marks {
  display: flex;
  justify-content: space-between;
  margin-top: 8px;
  padding: 0 20px;
}

.mark {
  font-size: 10px;
  color: var(--text-secondary);
  cursor: pointer;
  padding: 2px 6px;
  border-radius: 4px;
  transition: all 0.2s;
}

.mark:hover {
  background: rgba(168, 85, 247, 0.2);
}

.mark.active {
  background: var(--accent-purple);
  color: white;
  font-weight: 600;
}

/* 3D visual hint */
.cube-hint {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 8px 16px;
  background: var(--bg-secondary);
  border-radius: 8px;
}

.cube-visual {
  position: relative;
  width: 60px;
  height: 40px;
  perspective: 200px;
  transform-style: preserve-3d;
}

.cube-layer {
  position: absolute;
  width: 50px;
  height: 30px;
  background: rgba(168, 85, 247, 0.2);
  border: 1px solid rgba(168, 85, 247, 0.4);
  border-radius: 4px;
  transition: all 0.3s;
}

.cube-layer.active {
  background: rgba(168, 85, 247, 0.5);
  border-color: var(--accent-purple);
  box-shadow: 0 0 10px rgba(168, 85, 247, 0.5);
}

.cube-label {
  font-size: 12px;
  color: var(--text-secondary);
}

/* Step information */
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

.step-hint {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 12px;
  color: var(--accent-orange);
  margin-top: 4px;
}

.hint-icon {
  font-size: 14px;
}

/* Grid styles */
.grid-wrapper {
  overflow: auto;
}

.grid-title {
  font-size: 13px;
  color: var(--text-secondary);
  margin-bottom: 8px;
  text-align: center;
}

.vol-highlight {
  color: var(--accent-purple);
  font-weight: 700;
  font-size: 15px;
}

.grid-header {
  display: flex;
  gap: 2px;
  margin-bottom: 2px;
}

.corner-cell {
  width: 100px;
  min-width: 100px;
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
  width: 100px;
  min-width: 100px;
  min-height: 40px;
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
  background: rgba(168, 85, 247, 0.1);
}

.cell.current {
  background: rgba(168, 85, 247, 0.3);
  border-color: var(--accent-purple);
  box-shadow: 0 0 15px rgba(168, 85, 247, 0.5);
  animation: glow-purple 1s ease-in-out infinite;
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
  background: rgba(168, 85, 247, 0.5);
  border: 1px solid var(--accent-purple);
}

.legend-color.without {
  background: rgba(249, 115, 22, 0.5);
  border: 1px solid var(--accent-orange);
}

.legend-color.with {
  background: rgba(16, 185, 129, 0.5);
  border: 1px solid var(--accent-green);
}

@keyframes glow-purple {
  0%, 100% { box-shadow: 0 0 15px rgba(168, 85, 247, 0.5); }
  50% { box-shadow: 0 0 25px rgba(168, 85, 247, 0.8); }
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

