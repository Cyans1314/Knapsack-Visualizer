<script setup>
import { computed } from 'vue'

const props = defineProps({
  data: Object,
  currentStep: Number,
  isFinished: Boolean
})

// Format memory size
const formatBytes = (bytes) => {
  if (bytes < 1024) return bytes + ' B'
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB'
  return (bytes / (1024 * 1024)).toFixed(2) + ' MB'
}

// Selected items
const selectedItems = computed(() => {
  if (!props.data || !props.isFinished) return []
  if (!props.data.path || props.data.path.length === 0) return []
  
  // Handle different algorithm path formats
  return props.data.path.map(p => {
    const itemIdx = p.item !== undefined ? p.item : 0
    if (itemIdx >= props.data.items.length) return null
    
    return {
      index: itemIdx + 1,
      weight: props.data.items[itemIdx]?.w || 0,
      value: props.data.items[itemIdx]?.v || 0,
      package: p.package || null,  // Dependency knapsack package description
      group: p.group || null       // Group knapsack group number
    }
  }).filter(Boolean)
})

// Is scheme counting type
const isCountType = computed(() => props.data?.type === '方案计数')

// Is Kth optimal solution
const isKthType = computed(() => props.data?.type === '第K优解')

// Result label
const resultLabel = computed(() => {
  if (isCountType.value) return 'Total schemes'
  if (isKthType.value) return `${props.data.k}th optimal`
  return 'Max value'
})

// Total weight and value
const totalWeight = computed(() => {
  return selectedItems.value.reduce((sum, item) => sum + item.weight, 0)
})

const totalValue = computed(() => {
  return selectedItems.value.reduce((sum, item) => sum + item.value, 0)
})

// Current progress percentage
const progressPercent = computed(() => {
  if (!props.data) return 0
  return Math.round(((props.currentStep + 1) / props.data.steps.length) * 100)
})
</script>

<template>
  <div class="info-panel" v-if="data">
    <!-- Demo progress -->
    <div class="info-section">
      <h4>📍 Demo Progress</h4>
      <div class="progress-info">
        <div class="progress-bar">
          <div class="progress-fill" :style="{ width: progressPercent + '%' }"></div>
        </div>
        <div class="progress-text">
          <span>Step {{ currentStep + 1 }}</span>
          <span v-if="isFinished">Total {{ data.steps.length }} steps</span>
          <span v-else>In progress...</span>
        </div>
      </div>
    </div>

    <!-- Complexity information (always show) -->
    <div class="info-section">
      <h4>⚡ Complexity Analysis</h4>
      <div class="complexity-grid">
        <div class="complexity-item">
          <span class="label">Time complexity</span>
          <span class="formula">{{ data.complexity?.time }}</span>
        </div>
        <div class="complexity-item">
          <span class="label">Space complexity</span>
          <span class="formula">{{ data.complexity?.space }}</span>
        </div>
        <div class="complexity-item">
          <span class="label">Operations</span>
          <span class="actual">{{ data.complexity?.operations }}</span>
        </div>
        <div class="complexity-item">
          <span class="label">Memory usage</span>
          <span class="actual">{{ formatBytes(data.complexity?.memory_bytes) }}</span>
        </div>
      </div>
    </div>

    <!-- Calculation result (show after demo complete) -->
    <Transition name="fade">
      <div class="info-section result-section" v-if="isFinished">
        <h4>🏆 Calculation Result</h4>
        <div class="result-grid">
          <div class="result-item main-result">
            <span class="label">{{ resultLabel }}</span>
            <span class="value gold">{{ isKthType ? data.kth_value : data.max_value }}</span>
          </div>
          <div class="result-item">
            <span class="label">Execution time</span>
            <span class="value">{{ data.time_ms?.toFixed(2) }} ms</span>
          </div>
        </div>
        
        <!-- Kth optimal solution: show top K values -->
        <div class="topk-list" v-if="isKthType && data.topK">
          <div class="topk-title">Top {{ data.k }} optimal solutions:</div>
          <div class="topk-values">
            <span 
              v-for="(v, idx) in data.topK" 
              :key="idx"
              :class="['topk-item', { 'highlight': idx === data.k - 1 }]"
            >
              #{{ idx + 1 }}: {{ v }}
            </span>
          </div>
        </div>
      </div>
    </Transition>

    <!-- Optimal scheme (show after demo complete, except scheme counting) -->
    <Transition name="fade">
      <div class="info-section" v-if="isFinished && selectedItems.length > 0 && !isCountType">
        <h4>🎒 Optimal Scheme</h4>
        <div class="selected-items">
          <div 
            v-for="(item, idx) in selectedItems" 
            :key="idx"
            class="selected-item"
          >
            <span class="item-badge" v-if="item.package">{{ item.package }}</span>
            <span class="item-badge" v-else-if="item.group">Group{{ item.group }}-Item{{ item.index }}</span>
            <span class="item-badge" v-else>Item {{ item.index }}</span>
            <span class="item-detail">w={{ item.weight }}, v={{ item.value }}</span>
          </div>
        </div>
        <div class="summary">
          <span>Total weight: <strong>{{ totalWeight }}</strong> / {{ data.capacity }}</span>
          <span>Total value: <strong class="gold">{{ totalValue }}</strong></span>
        </div>
      </div>
    </Transition>

    <!-- Demo in progress hint -->
    <div class="info-section waiting" v-if="!isFinished">
      <div class="waiting-icon">⏳</div>
      <p>Demo in progress, results will show after completion...</p>
    </div>
  </div>
</template>

<style scoped>
.info-panel {
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: 12px;
  padding: 16px;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.info-section h4 {
  font-size: 13px;
  font-weight: 600;
  margin-bottom: 10px;
  color: var(--text-primary);
}

/* 进度条 */
.progress-info {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.progress-bar {
  height: 6px;
  background: var(--bg-secondary);
  border-radius: 3px;
  overflow: hidden;
}

.progress-fill {
  height: 100%;
  background: linear-gradient(90deg, var(--accent-cyan), var(--accent-blue));
  border-radius: 3px;
  transition: width 0.3s ease;
}

.progress-text {
  display: flex;
  justify-content: space-between;
  font-size: 12px;
  color: var(--text-secondary);
}

/* 复杂度 */
.complexity-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
}

.complexity-item {
  display: flex;
  flex-direction: column;
  gap: 2px;
  padding: 8px;
  background: var(--bg-secondary);
  border-radius: 6px;
}

.complexity-item .label {
  font-size: 11px;
  color: var(--text-secondary);
}

.complexity-item .formula {
  font-size: 13px;
  color: var(--accent-cyan);
  font-family: 'Consolas', monospace;
}

.complexity-item .actual {
  font-size: 13px;
  color: var(--text-primary);
}

/* 结果 */
.result-section {
  animation: slideIn 0.5s ease;
}

.result-grid {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.result-item {
  background: var(--bg-secondary);
  padding: 10px 12px;
  border-radius: 8px;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.result-item.main-result {
  background: linear-gradient(135deg, rgba(251, 191, 36, 0.2), rgba(249, 115, 22, 0.2));
  border: 1px solid rgba(251, 191, 36, 0.3);
}

.result-item .label {
  font-size: 12px;
  color: var(--text-secondary);
}

.result-item .value {
  font-size: 18px;
  font-weight: 700;
}

.result-item .value.gold {
  color: var(--accent-gold);
  text-shadow: 0 0 10px rgba(251, 191, 36, 0.5);
}

/* 选中物品 */
.selected-items {
  display: flex;
  flex-direction: column;
  gap: 6px;
  margin-bottom: 10px;
  max-height: 150px;
  overflow-y: auto;
}

.selected-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 6px 10px;
  background: rgba(251, 191, 36, 0.1);
  border: 1px solid rgba(251, 191, 36, 0.3);
  border-radius: 6px;
}

.item-badge {
  background: var(--accent-gold);
  color: #000;
  padding: 2px 6px;
  border-radius: 4px;
  font-size: 11px;
  font-weight: 600;
}

.item-detail {
  font-size: 12px;
  color: var(--text-secondary);
}

.summary {
  display: flex;
  justify-content: space-between;
  font-size: 12px;
  color: var(--text-secondary);
  padding-top: 8px;
  border-top: 1px solid var(--border-color);
}

.summary strong {
  color: var(--text-primary);
}

.summary .gold {
  color: var(--accent-gold);
}

/* 第K优解列表 */
.topk-list {
  margin-top: 12px;
  padding-top: 12px;
  border-top: 1px solid var(--border-color);
}

.topk-title {
  font-size: 12px;
  color: var(--text-secondary);
  margin-bottom: 8px;
}

.topk-values {
  display: flex;
  flex-wrap: wrap;
  gap: 6px;
}

.topk-item {
  padding: 4px 8px;
  background: var(--bg-secondary);
  border-radius: 4px;
  font-size: 12px;
  color: var(--text-primary);
}

.topk-item.highlight {
  background: linear-gradient(135deg, var(--accent-gold), var(--accent-orange));
  color: #000;
  font-weight: 600;
}

/* 等待提示 */
.waiting {
  text-align: center;
  padding: 20px;
  color: var(--text-secondary);
}

.waiting-icon {
  font-size: 32px;
  margin-bottom: 8px;
  animation: pulse 1.5s ease-in-out infinite;
}

.waiting p {
  font-size: 12px;
}

/* 动画 */
.fade-enter-active {
  transition: all 0.5s ease;
}

.fade-enter-from {
  opacity: 0;
  transform: translateY(10px);
}

@keyframes slideIn {
  from { transform: translateY(10px); opacity: 0; }
  to { transform: translateY(0); opacity: 1; }
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}
</style>

