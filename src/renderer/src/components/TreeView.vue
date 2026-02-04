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

// Build tree structure
const treeData = computed(() => {
  if (!props.data?.items) return null
  
  const items = props.data.items
  const nodes = items.map((item, idx) => ({
    id: idx,
    ...item,
    children: [],
    parent: item.p - 1  // Convert to 0-based index, -1 means root
  }))
  
  // Find root nodes (parent < 0)
  const roots = []
  nodes.forEach(node => {
    if (node.parent < 0) {
      roots.push(node)
    } else if (nodes[node.parent]) {
      nodes[node.parent].children.push(node)
    }
  })
  
  return { roots, nodes }
})

// Calculate node positions (hierarchical layout)
const nodePositions = computed(() => {
  if (!treeData.value) return {}
  
  const positions = {}
  const levelWidth = 120
  const nodeHeight = 80
  
  const calculatePositions = (nodes, level, startX) => {
    let currentX = startX
    nodes.forEach(node => {
      const childWidth = calculateSubtreeWidth(node)
      const nodeX = currentX + childWidth / 2
      positions[node.id] = {
        x: nodeX,
        y: level * nodeHeight + 50
      }
      
      if (node.children.length > 0) {
        calculatePositions(node.children, level + 1, currentX)
      }
      currentX += childWidth
    })
  }
  
  const calculateSubtreeWidth = (node) => {
    if (node.children.length === 0) return levelWidth
    return node.children.reduce((sum, child) => sum + calculateSubtreeWidth(child), 0)
  }
  
  calculatePositions(treeData.value.roots, 0, 50)
  return positions
})

// SVG viewBox size
const svgViewBox = computed(() => {
  const positions = Object.values(nodePositions.value)
  if (positions.length === 0) return '0 0 600 400'
  
  const maxX = Math.max(...positions.map(p => p.x)) + 80
  const maxY = Math.max(...positions.map(p => p.y)) + 80
  return `0 0 ${Math.max(600, maxX)} ${Math.max(400, maxY)}`
})

// Determine node state
const getNodeState = (nodeId) => {
  const step = currentStepInfo.value
  if (!step) return 'idle'
  
  if (step.node === nodeId) return 'current'
  if (step.highlight?.some(h => h.node === nodeId)) return 'highlight'
  if (props.data.path?.some(p => p.node === nodeId)) return 'selected'
  
  return 'idle'
}

// Get edge state
const getEdgeState = (parentId, childId) => {
  const step = currentStepInfo.value
  if (!step) return 'idle'
  
  if (step.node === childId && step.parentNode === parentId) return 'active'
  if (props.data.path?.some(p => p.node === childId)) return 'selected'
  
  return 'idle'
}

// Current processing node DP values
const currentDPValues = computed(() => {
  if (!currentStepInfo.value?.dpValues) return null
  return currentStepInfo.value.dpValues
})
</script>

<template>
  <div class="tree-view" v-if="data">
    <!-- Top information -->
    <div class="info-bar">
      <div class="info-item">
        <span class="label">背包容量：</span>
        <span class="value">{{ data.capacity }}</span>
      </div>
      <div class="info-item" v-if="currentStepInfo">
        <span class="label">当前节点：</span>
        <span class="value highlight">节点 {{ currentStepInfo.node + 1 }}</span>
      </div>
      <div class="info-item" v-if="currentStepInfo">
        <span class="label">当前值：</span>
        <span class="value gold">{{ currentStepInfo.val }}</span>
      </div>
    </div>

    <!-- SVG tree view -->
    <div class="tree-container">
      <svg class="tree-svg" :viewBox="svgViewBox">
        <!-- Edges -->
        <g class="edges">
          <template v-for="node in treeData?.nodes" :key="'edge-' + node.id">
            <line
              v-if="node.parent >= 0 && nodePositions[node.parent] && nodePositions[node.id]"
              :x1="nodePositions[node.parent].x"
              :y1="nodePositions[node.parent].y + 25"
              :x2="nodePositions[node.id].x"
              :y2="nodePositions[node.id].y - 25"
              :class="['edge', getEdgeState(node.parent, node.id)]"
            />
          </template>
        </g>

        <!-- Nodes -->
        <g class="nodes">
          <g 
            v-for="node in treeData?.nodes"
            :key="'node-' + node.id"
            :class="['tree-node', getNodeState(node.id)]"
            :transform="`translate(${nodePositions[node.id]?.x || 0}, ${nodePositions[node.id]?.y || 0})`"
          >
            <!-- Node circle -->
            <circle r="25" class="node-circle" />
            
            <!-- Node ID -->
            <text y="5" class="node-id">{{ node.id + 1 }}</text>
            
            <!-- Node information -->
            <text y="45" class="node-info">w={{ node.w }}, v={{ node.v }}</text>
          </g>
        </g>
      </svg>
    </div>

    <!-- DP values display -->
    <div class="dp-values" v-if="currentDPValues">
      <div class="dp-title">节点 {{ currentStepInfo.node + 1 }} DP值：</div>
      <div class="dp-array">
        <div 
          v-for="(val, idx) in currentDPValues.slice(0, Math.min(currentDPValues.length, 15))"
          :key="idx"
          :class="['dp-cell', { highlight: idx === currentStepInfo.col }]"
        >
          <span class="dp-index">{{ idx }}</span>
          <span class="dp-val">{{ val }}</span>
        </div>
        <div v-if="currentDPValues.length > 15" class="dp-more">...</div>
      </div>
    </div>

    <!-- Decision information -->
    <div class="decision-info" v-if="currentStepInfo">
      <div class="decision-text">
        <span v-if="currentStepInfo.decision === 'take'" class="take">
          ✓ 选择节点 {{ currentStepInfo.node + 1 }}
        </span>
        <span v-else class="skip">
          ✗ 跳过节点 {{ currentStepInfo.node + 1 }}
        </span>
      </div>
      <div class="formula" v-if="currentStepInfo.formula">
        {{ currentStepInfo.formula }}
      </div>
    </div>

    <!-- Legend -->
    <div class="legend">
      <div class="legend-item">
        <span class="legend-dot current"></span>
        <span>当前处理</span>
      </div>
      <div class="legend-item">
        <span class="legend-dot highlight"></span>
        <span>依赖节点</span>
      </div>
      <div class="legend-item">
        <span class="legend-dot selected"></span>
        <span>已选择</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
.tree-view {
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
  flex-wrap: wrap;
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
  color: var(--accent-cyan);
}

.info-item .value.gold {
  color: var(--accent-gold);
}

.tree-container {
  overflow: auto;
  background: var(--bg-secondary);
  border-radius: 8px;
  padding: 16px;
  min-height: 300px;
}

.tree-svg {
  width: 100%;
  min-height: 300px;
}

/* Edge styles */
.edge {
  stroke: var(--border-color);
  stroke-width: 2;
  transition: all 0.3s;
}

.edge.active {
  stroke: var(--accent-cyan);
  stroke-width: 3;
  filter: drop-shadow(0 0 6px var(--accent-cyan));
}

.edge.selected {
  stroke: var(--accent-gold);
  stroke-width: 3;
}

/* Node styles */
.node-circle {
  fill: var(--bg-card);
  stroke: var(--border-color);
  stroke-width: 2;
  transition: all 0.3s;
}

.tree-node.current .node-circle {
  stroke: var(--accent-cyan);
  stroke-width: 3;
  fill: rgba(34, 211, 238, 0.2);
  filter: drop-shadow(0 0 10px var(--accent-cyan));
  animation: pulse 1.5s ease-in-out infinite;
}

.tree-node.highlight .node-circle {
  stroke: var(--accent-orange);
  fill: rgba(249, 115, 22, 0.2);
}

.tree-node.selected .node-circle {
  stroke: var(--accent-gold);
  fill: rgba(251, 191, 36, 0.2);
  filter: drop-shadow(0 0 6px var(--accent-gold));
}

.node-id {
  fill: var(--text-primary);
  font-size: 14px;
  font-weight: 700;
  text-anchor: middle;
}

.node-info {
  fill: var(--text-secondary);
  font-size: 10px;
  text-anchor: middle;
}

/* DP values display */
.dp-values {
  background: var(--bg-secondary);
  border-radius: 8px;
  padding: 12px;
  border: 1px solid var(--border-color);
}

.dp-title {
  font-size: 13px;
  color: var(--text-secondary);
  margin-bottom: 8px;
}

.dp-array {
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
}

.dp-cell {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 4px 8px;
  background: var(--bg-card);
  border-radius: 4px;
  border: 1px solid var(--border-color);
  min-width: 36px;
}

.dp-cell.highlight {
  border-color: var(--accent-cyan);
  background: rgba(34, 211, 238, 0.2);
}

.dp-index {
  font-size: 10px;
  color: var(--text-secondary);
}

.dp-val {
  font-size: 12px;
  font-weight: 600;
  color: var(--text-primary);
}

.dp-more {
  display: flex;
  align-items: center;
  color: var(--text-secondary);
}

/* Decision information */
.decision-info {
  padding: 12px;
  background: var(--bg-secondary);
  border-radius: 8px;
  border: 1px solid var(--border-color);
  text-align: center;
}

.decision-text {
  font-size: 14px;
  font-weight: 600;
  margin-bottom: 4px;
}

.decision-text .take {
  color: var(--accent-green);
}

.decision-text .skip {
  color: var(--accent-orange);
}

.formula {
  font-size: 12px;
  color: var(--text-secondary);
  font-family: 'Consolas', monospace;
}

/* Legend */
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

.legend-dot.current {
  background: var(--accent-cyan);
  box-shadow: 0 0 6px var(--accent-cyan);
}

.legend-dot.highlight {
  background: var(--accent-orange);
}

.legend-dot.selected {
  background: var(--accent-gold);
}

@keyframes pulse {
  0%, 100% { filter: drop-shadow(0 0 10px var(--accent-cyan)); }
  50% { filter: drop-shadow(0 0 20px var(--accent-cyan)); }
}
</style>

