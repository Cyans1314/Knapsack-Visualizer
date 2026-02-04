<script setup>
import { computed, ref, watch } from 'vue'

const props = defineProps({
  data: Object,
  currentStep: Number
})

// Current step information
const currentStepInfo = computed(() => {
  if (!props.data || props.currentStep < 0) return null
  return props.data.steps[props.currentStep]
})

// Group data
const groups = computed(() => {
  if (!props.data?.groups) return []
  return props.data.groups.map((g, idx) => ({
    ...g,
    items: g.items.map(itemIdx => ({
      idx: itemIdx,
      ...props.data.items[itemIdx]
    })),
    isActive: currentStepInfo.value?.groupId === g.id,
    isCompleted: currentStepInfo.value ? 
      props.data.groups.findIndex(gr => gr.id === currentStepInfo.value.groupId) > idx : false
  }))
})

// Currently selected items
const selectedItems = computed(() => {
  if (!props.data?.path) return new Set()
  return new Set(props.data.path.map(p => p.item))
})

// Calculate circle layout
const getGroupPosition = (index, total) => {
  const centerX = 300
  const centerY = 250
  const radius = 180
  const angle = (index / total) * 2 * Math.PI - Math.PI / 2
  return {
    x: centerX + radius * Math.cos(angle),
    y: centerY + radius * Math.sin(angle)
  }
}

// Calculate item position within group
const getItemPosition = (itemIndex, totalItems, groupX, groupY) => {
  const radius = 45
  const angle = (itemIndex / totalItems) * 2 * Math.PI - Math.PI / 2
  return {
    x: groupX + radius * Math.cos(angle),
    y: groupY + radius * Math.sin(angle)
  }
}

// Determine if item is being tried
const isItemTrying = (itemIdx) => {
  if (!currentStepInfo.value?.tryItems) return false
  return currentStepInfo.value.tryItems.some(t => t.itemIdx === itemIdx)
}

// Determine if item is best choice
const isItemBest = (itemIdx) => {
  return currentStepInfo.value?.bestChoice === itemIdx
}

// Current capacity
const currentCapacity = computed(() => currentStepInfo.value?.col || 0)

// Current optimal value
const currentValue = computed(() => currentStepInfo.value?.val || 0)
</script>

<template>
  <div class="group-circle-view" v-if="data">
    <!-- Top information -->
    <div class="info-bar">
      <div class="info-item">
        <span class="label">Current capacity:</span>
        <span class="value">{{ currentCapacity }}</span>
      </div>
      <div class="info-item">
        <span class="label">Current optimal:</span>
        <span class="value highlight">{{ currentValue }}</span>
      </div>
      <div class="info-item" v-if="currentStepInfo">
        <span class="label">Processing group:</span>
        <span class="value group-badge">Group {{ currentStepInfo.groupId }}</span>
      </div>
    </div>

    <!-- SVG circle view -->
    <svg class="circle-svg" viewBox="0 0 600 500">
      <!-- Center backpack -->
      <g class="backpack-center">
        <circle cx="300" cy="250" r="60" class="backpack-circle" />
        <text x="300" y="240" class="backpack-icon">🎒</text>
        <text x="300" y="270" class="backpack-text">Capacity: {{ data.capacity }}</text>
      </g>

      <!-- Connection lines -->
      <g class="connections">
        <line 
          v-for="(group, gIdx) in groups"
          :key="'line-' + gIdx"
          :x1="300"
          :y1="250"
          :x2="getGroupPosition(gIdx, groups.length).x"
          :y2="getGroupPosition(gIdx, groups.length).y"
          :class="['connect-line', { active: group.isActive, completed: group.isCompleted }]"
        />
      </g>

      <!-- Group circles -->
      <g 
        v-for="(group, gIdx) in groups"
        :key="'group-' + gIdx"
        :class="['group-node', { active: group.isActive, completed: group.isCompleted }]"
      >
        <!-- Group circle -->
        <circle 
          :cx="getGroupPosition(gIdx, groups.length).x"
          :cy="getGroupPosition(gIdx, groups.length).y"
          r="55"
          class="group-circle"
        />
        
        <!-- Group label -->
        <text 
          :x="getGroupPosition(gIdx, groups.length).x"
          :y="getGroupPosition(gIdx, groups.length).y - 65"
          class="group-label"
        >
          Group {{ group.id }}
        </text>

        <!-- Items in group -->
        <g 
          v-for="(item, iIdx) in group.items"
          :key="'item-' + item.idx"
          :class="[
            'item-node',
            { 
              trying: isItemTrying(item.idx),
              best: isItemBest(item.idx),
              selected: selectedItems.has(item.idx)
            }
          ]"
        >
          <circle 
            :cx="getItemPosition(iIdx, group.items.length, getGroupPosition(gIdx, groups.length).x, getGroupPosition(gIdx, groups.length).y).x"
            :cy="getItemPosition(iIdx, group.items.length, getGroupPosition(gIdx, groups.length).x, getGroupPosition(gIdx, groups.length).y).y"
            r="18"
            class="item-circle"
          />
          <text 
            :x="getItemPosition(iIdx, group.items.length, getGroupPosition(gIdx, groups.length).x, getGroupPosition(gIdx, groups.length).y).x"
            :y="getItemPosition(iIdx, group.items.length, getGroupPosition(gIdx, groups.length).x, getGroupPosition(gIdx, groups.length).y).y + 4"
            class="item-text"
          >
            {{ item.idx + 1 }}
          </text>
        </g>
      </g>
    </svg>

    <!-- Current attempt information -->
    <div class="try-info" v-if="currentStepInfo?.tryItems">
      <div class="try-title">Group item attempt:</div>
      <div class="try-list">
        <div 
          v-for="item in currentStepInfo.tryItems"
          :key="item.itemIdx"
          :class="['try-item', { best: currentStepInfo.bestChoice === item.itemIdx }]"
        >
          <span class="item-id">Item{{ item.itemIdx + 1 }}</span>
          <span class="item-detail">w={{ item.w }}, v={{ item.v }}</span>
          <span class="item-result" v-if="item.canTake">
            → {{ item.newVal }}
          </span>
          <span class="item-result cannot" v-else>Overweight</span>
        </div>
      </div>
    </div>

    <!-- Legend -->
    <div class="legend">
      <div class="legend-item">
        <span class="legend-dot active"></span>
        <span>Current processing</span>
      </div>
      <div class="legend-item">
        <span class="legend-dot trying"></span>
        <span>Trying</span>
      </div>
      <div class="legend-item">
        <span class="legend-dot best"></span>
        <span>Best choice</span>
      </div>
      <div class="legend-item">
        <span class="legend-dot selected"></span>
        <span>Selected</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
.group-circle-view {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.info-bar {
  display: flex;
  gap: 24px;
  padding: 12px 16px;
  background: var(--bg-secondary);
  border-radius: 8px;
  border: 1px solid var(--border-color);
}

.info-item {
  display: flex;
  align-items: center;
  gap: 8px;
}

.info-item .label {
  color: var(--text-secondary);
  font-size: 13px;
}

.info-item .value {
  font-weight: 600;
  font-size: 15px;
}

.info-item .value.highlight {
  color: var(--accent-gold);
}

.group-badge {
  background: linear-gradient(135deg, var(--accent-cyan), var(--accent-blue));
  padding: 2px 10px;
  border-radius: 12px;
  font-size: 12px;
}

.circle-svg {
  width: 100%;
  max-width: 600px;
  margin: 0 auto;
}

/* 中心背包 */
.backpack-circle {
  fill: rgba(34, 211, 238, 0.2);
  stroke: var(--accent-cyan);
  stroke-width: 3;
}

.backpack-icon {
  font-size: 32px;
  text-anchor: middle;
}

.backpack-text {
  fill: var(--text-secondary);
  font-size: 12px;
  text-anchor: middle;
}

/* 连接线 */
.connect-line {
  stroke: var(--border-color);
  stroke-width: 2;
  stroke-dasharray: 5, 5;
  transition: all 0.3s;
}

.connect-line.active {
  stroke: var(--accent-cyan);
  stroke-width: 3;
  stroke-dasharray: none;
  filter: drop-shadow(0 0 6px var(--accent-cyan));
}

.connect-line.completed {
  stroke: var(--accent-green);
  stroke-dasharray: none;
}

/* 分组节点 */
.group-circle {
  fill: rgba(17, 24, 39, 0.8);
  stroke: var(--border-color);
  stroke-width: 2;
  transition: all 0.3s;
}

.group-node.active .group-circle {
  stroke: var(--accent-cyan);
  stroke-width: 3;
  filter: drop-shadow(0 0 10px var(--accent-cyan));
  animation: pulse 1.5s ease-in-out infinite;
}

.group-node.completed .group-circle {
  stroke: var(--accent-green);
  fill: rgba(16, 185, 129, 0.1);
}

.group-label {
  fill: var(--text-primary);
  font-size: 14px;
  font-weight: 600;
  text-anchor: middle;
}

/* 物品节点 */
.item-circle {
  fill: var(--bg-secondary);
  stroke: var(--border-color);
  stroke-width: 2;
  transition: all 0.3s;
}

.item-node.trying .item-circle {
  stroke: var(--accent-orange);
  fill: rgba(249, 115, 22, 0.3);
  animation: blink 0.5s ease-in-out infinite;
}

.item-node.best .item-circle {
  stroke: var(--accent-green);
  fill: rgba(16, 185, 129, 0.4);
  filter: drop-shadow(0 0 8px var(--accent-green));
}

.item-node.selected .item-circle {
  stroke: var(--accent-gold);
  fill: rgba(251, 191, 36, 0.4);
  filter: drop-shadow(0 0 6px var(--accent-gold));
}

.item-text {
  fill: var(--text-primary);
  font-size: 12px;
  font-weight: 600;
  text-anchor: middle;
}

/* 尝试信息 */
.try-info {
  background: var(--bg-secondary);
  border-radius: 8px;
  padding: 12px;
  border: 1px solid var(--border-color);
}

.try-title {
  font-size: 13px;
  color: var(--text-secondary);
  margin-bottom: 8px;
}

.try-list {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.try-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 12px;
  background: var(--bg-card);
  border-radius: 6px;
  font-size: 12px;
  border: 1px solid var(--border-color);
}

.try-item.best {
  border-color: var(--accent-green);
  background: rgba(16, 185, 129, 0.2);
}

.item-id {
  font-weight: 600;
  color: var(--accent-cyan);
}

.item-detail {
  color: var(--text-secondary);
}

.item-result {
  color: var(--accent-green);
  font-weight: 600;
}

.item-result.cannot {
  color: var(--accent-orange);
}

/* 图例 */
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

.legend-dot {
  width: 12px;
  height: 12px;
  border-radius: 50%;
}

.legend-dot.active {
  background: var(--accent-cyan);
  box-shadow: 0 0 6px var(--accent-cyan);
}

.legend-dot.trying {
  background: var(--accent-orange);
}

.legend-dot.best {
  background: var(--accent-green);
}

.legend-dot.selected {
  background: var(--accent-gold);
}

@keyframes pulse {
  0%, 100% { filter: drop-shadow(0 0 10px var(--accent-cyan)); }
  50% { filter: drop-shadow(0 0 20px var(--accent-cyan)); }
}

@keyframes blink {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}
</style>

