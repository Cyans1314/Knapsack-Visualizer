<script setup>
import { ref, computed, watch } from 'vue'
import { Plus, Delete, VideoPlay } from '@element-plus/icons-vue'

const props = defineProps({
  isRunning: Boolean,
  algorithm: String
})

const emit = defineEmits(['run'])

// 算法配置：定义每种算法需要的输入字段
const algorithmConfig = {
  knapsack_01: {
    name: '0/1 背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:6},{w:2,v:3},{w:6,v:5},{w:5,v:4},{w:4,v:6}] },
      { name: '简单', capacity: 5, items: [{w:1,v:2},{w:2,v:4},{w:3,v:4}] }
    ]
  },
  knapsack_complete: {
    name: '完全背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:3},{w:3,v:4},{w:4,v:5}] }
    ]
  },
  knapsack_multiple: {
    name: '多重背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value', 'count'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:3,c:3},{w:3,v:4,c:2},{w:4,v:5,c:1}] }
    ]
  },
  knapsack_mixed: {
    name: '混合背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value', 'type'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:3,t:0},{w:3,v:4,t:1},{w:4,v:5,t:2}] }
    ]
  },
  knapsack_2d: {
    name: '二维费用',
    fields: ['capacity', 'capacity2', 'items'],
    itemFields: ['weight', 'volume', 'value'],
    presets: [
      { name: '经典', capacity: 10, capacity2: 8, items: [{w:2,m:3,v:4},{w:3,m:2,v:5},{w:4,m:4,v:6}] }
    ]
  },
  knapsack_group: {
    name: '分组背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value', 'group'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:3,g:1},{w:3,v:4,g:1},{w:4,v:5,g:2},{w:5,v:6,g:2}] }
    ]
  },
  knapsack_depend: {
    name: '依赖背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value', 'parent'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:3,v:5,p:0},{w:2,v:3,p:1},{w:1,v:2,p:1},{w:4,v:6,p:0}] }
    ]
  },
  knapsack_tree: {
    name: '树形背包',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value', 'parent'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:3,p:0},{w:3,v:4,p:1},{w:2,v:2,p:1},{w:4,v:5,p:0},{w:2,v:3,p:4}] },
      { name: '简单', capacity: 8, items: [{w:2,v:4,p:0},{w:3,v:5,p:1},{w:2,v:3,p:1}] }
    ]
  },
  knapsack_kth: {
    name: '第K优解',
    fields: ['capacity', 'k', 'items'],
    itemFields: ['weight', 'value'],
    presets: [
      { name: '经典', capacity: 10, k: 3, items: [{w:2,v:3},{w:3,v:4},{w:4,v:5}] }
    ]
  },
  knapsack_count: {
    name: '方案计数',
    fields: ['capacity', 'items'],
    itemFields: ['weight', 'value'],
    presets: [
      { name: '经典', capacity: 10, items: [{w:2,v:3},{w:3,v:4},{w:4,v:5}] }
    ]
  }
}

// Current algorithm configuration
const currentConfig = computed(() => algorithmConfig[props.algorithm] || algorithmConfig.knapsack_01)

// Input data
const capacity = ref(10)
const capacity2 = ref(8)
const k = ref(3)
const items = ref([])

// Item type options (for mixed knapsack)
const typeOptions = [
  { value: 0, label: '0/1背包' },
  { value: 1, label: '完全背包' },
  { value: 2, label: '多重背包' }
]

// Initialize items list
const initItems = () => {
  const config = currentConfig.value
  const preset = config.presets[0]
  capacity.value = preset.capacity
  if (preset.capacity2) capacity2.value = preset.capacity2
  if (preset.k) k.value = preset.k
  
  items.value = preset.items.map(item => {
    const newItem = { weight: item.w, value: item.v }
    if (item.c !== undefined) newItem.count = item.c
    if (item.t !== undefined) newItem.type = item.t
    if (item.m !== undefined) newItem.volume = item.m
    if (item.g !== undefined) newItem.group = item.g
    if (item.p !== undefined) newItem.parent = item.p
    return newItem
  })
}

// Watch algorithm change, reinitialize
watch(() => props.algorithm, () => {
  initItems()
}, { immediate: true })

// Add item
const addItem = () => {
  const config = currentConfig.value
  const newItem = { weight: 1, value: 1 }
  if (config.itemFields.includes('count')) newItem.count = 1
  if (config.itemFields.includes('type')) newItem.type = 0
  if (config.itemFields.includes('volume')) newItem.volume = 1
  if (config.itemFields.includes('group')) newItem.group = 1
  if (config.itemFields.includes('parent')) newItem.parent = 0
  items.value.push(newItem)
}

// Delete item
const removeItem = (index) => {
  if (items.value.length > 1) {
    items.value.splice(index, 1)
  }
}

// Load preset
const loadPreset = (preset) => {
  capacity.value = preset.capacity
  if (preset.capacity2) capacity2.value = preset.capacity2
  if (preset.k) k.value = preset.k
  
  items.value = preset.items.map(item => {
    const newItem = { weight: item.w, value: item.v }
    if (item.c !== undefined) newItem.count = item.c
    if (item.t !== undefined) newItem.type = item.t
    if (item.m !== undefined) newItem.volume = item.m
    if (item.g !== undefined) newItem.group = item.g
    if (item.p !== undefined) newItem.parent = item.p
    return newItem
  })
}

// Random generation
const randomGenerate = () => {
  const config = currentConfig.value
  const n = Math.floor(Math.random() * 18) + 3  // 3-20 items
  capacity.value = Math.floor(Math.random() * 10) + 8
  if (config.fields.includes('capacity2')) {
    capacity2.value = Math.floor(Math.random() * 8) + 5
  }
  
  items.value = Array.from({ length: n }, (_, i) => {
    const item = {
      weight: Math.floor(Math.random() * 5) + 1,
      value: Math.floor(Math.random() * 8) + 1
    }
    if (config.itemFields.includes('count')) item.count = Math.floor(Math.random() * 3) + 1
    if (config.itemFields.includes('type')) item.type = Math.floor(Math.random() * 3)
    if (config.itemFields.includes('volume')) item.volume = Math.floor(Math.random() * 4) + 1
    if (config.itemFields.includes('group')) item.group = Math.floor(i / 2) + 1
    if (config.itemFields.includes('parent')) {
      // Tree/dependency knapsack: generate reasonable tree structure
      if (i === 0) {
        item.parent = 0  // First is root node
      } else {
        // Randomly select a previous node as parent, or become new root
        item.parent = Math.random() > 0.3 ? Math.floor(Math.random() * i) + 1 : 0
      }
    }
    return item
  })
}

// Start running
const handleRun = () => {
  const params = {
    capacity: capacity.value,
    items: JSON.parse(JSON.stringify(items.value))
  }
  if (currentConfig.value.fields.includes('capacity2')) {
    params.capacity2 = capacity2.value
  }
  if (currentConfig.value.fields.includes('k')) {
    params.k = k.value
  }
  emit('run', params)
}

// Validation
const isValid = computed(() => {
  return capacity.value > 0 && items.value.length > 0 &&
    items.value.every(i => i.weight > 0 && i.value > 0)
})

// Get field label
const getFieldLabel = (field) => {
  const labels = {
    weight: 'w',
    value: 'v',
    count: 'n',
    type: '类型',
    volume: 'm',
    group: '组',
    parent: '主件'
  }
  return labels[field] || field
}
</script>

<template>
  <div class="input-panel">
    <div class="panel-header">
      <h3>📝 {{ currentConfig.name }}</h3>
    </div>

    <!-- 预设案例 -->
    <div class="preset-section">
      <div class="preset-buttons">
        <button 
          v-for="preset in currentConfig.presets" 
          :key="preset.name"
          class="preset-btn"
          @click="loadPreset(preset)"
        >
          {{ preset.name }}
        </button>
        <button class="preset-btn random" @click="randomGenerate">🎲 随机</button>
      </div>
    </div>

    <!-- 背包容量 -->
    <div class="form-item">
      <label>背包容量 (C)</label>
      <el-input-number v-model="capacity" :min="1" :max="50" size="default" />
    </div>

    <!-- 二维费用：第二个容量 -->
    <div class="form-item" v-if="currentConfig.fields.includes('capacity2')">
      <label>体积限制 (M)</label>
      <el-input-number v-model="capacity2" :min="1" :max="50" size="default" />
    </div>

    <!-- 第K优解：K值 -->
    <div class="form-item" v-if="currentConfig.fields.includes('k')">
      <label>求第 K 优解</label>
      <el-input-number v-model="k" :min="1" :max="10" size="default" />
    </div>

    <!-- 物品列表 -->
    <div class="form-item">
      <div class="item-header">
        <label>物品列表</label>
        <el-button type="primary" :icon="Plus" circle size="small" @click="addItem" />
      </div>
      
      <div class="item-list">
        <div class="item-row header">
          <span class="col-index">#</span>
          <span 
            v-for="field in currentConfig.itemFields" 
            :key="field"
            class="col-field"
          >
            {{ getFieldLabel(field) }}
          </span>
          <span class="col-action"></span>
        </div>
        
        <TransitionGroup name="list">
          <div 
            v-for="(item, index) in items" 
            :key="index"
            class="item-row"
          >
            <span class="col-index">{{ index + 1 }}</span>
            
            <!-- 重量 -->
            <el-input 
              v-if="currentConfig.itemFields.includes('weight')"
              v-model.number="item.weight" 
              type="number"
              size="small"
              class="col-field"
            />
            
            <!-- 体积（二维费用） -->
            <el-input 
              v-if="currentConfig.itemFields.includes('volume')"
              v-model.number="item.volume" 
              type="number"
              size="small"
              class="col-field"
            />
            
            <!-- 价值 -->
            <el-input 
              v-if="currentConfig.itemFields.includes('value')"
              v-model.number="item.value" 
              type="number"
              size="small"
              class="col-field"
            />
            
            <!-- 数量（多重背包） -->
            <el-input 
              v-if="currentConfig.itemFields.includes('count')"
              v-model.number="item.count" 
              type="number"
              size="small"
              class="col-field"
            />
            
            <!-- 类型（混合背包） -->
            <el-select 
              v-if="currentConfig.itemFields.includes('type')"
              v-model="item.type" 
              size="small"
              class="col-field"
            >
              <el-option 
                v-for="opt in typeOptions" 
                :key="opt.value" 
                :label="opt.label" 
                :value="opt.value" 
              />
            </el-select>
            
            <!-- 组号（分组背包） -->
            <el-input 
              v-if="currentConfig.itemFields.includes('group')"
              v-model.number="item.group" 
              type="number"
              size="small"
              class="col-field"
            />
            
            <!-- 主件ID（依赖背包） -->
            <el-input 
              v-if="currentConfig.itemFields.includes('parent')"
              v-model.number="item.parent" 
              type="number"
              size="small"
              class="col-field"
            />
            
            <el-button 
              :icon="Delete" 
              circle 
              size="small"
              class="col-action"
              :disabled="items.value?.length <= 1"
              @click="removeItem(index)"
            />
          </div>
        </TransitionGroup>
      </div>
    </div>

    <!-- 运行按钮 -->
    <el-button 
      type="primary" 
      size="large" 
      :icon="VideoPlay"
      :loading="isRunning"
      :disabled="!isValid"
      class="run-button"
      @click="handleRun"
    >
      {{ isRunning ? '计算中...' : '开始演示' }}
    </el-button>
  </div>
</template>

<style scoped>
.input-panel {
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: 12px;
  padding: 16px;
}

.panel-header {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 12px;
}

.panel-header h3 {
  font-size: 15px;
  font-weight: 600;
}

.preset-section {
  margin-bottom: 12px;
}

.preset-buttons {
  display: flex;
  flex-wrap: wrap;
  gap: 6px;
}

.preset-btn {
  background: var(--bg-secondary);
  border: 1px solid var(--border-color);
  color: var(--text-primary);
  padding: 4px 10px;
  border-radius: 6px;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s;
}

.preset-btn:hover {
  border-color: var(--accent-cyan);
}

.preset-btn.random {
  background: linear-gradient(135deg, var(--accent-purple), var(--accent-blue));
  border: none;
}

.form-item {
  margin-bottom: 12px;
}

.form-item > label {
  display: block;
  font-size: 12px;
  color: var(--text-secondary);
  margin-bottom: 6px;
}

.item-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 6px;
}

.item-header label {
  font-size: 12px;
  color: var(--text-secondary);
}

.item-list {
  background: var(--bg-secondary);
  border-radius: 8px;
  padding: 6px;
  max-height: 180px;
  overflow-y: auto;
}

.item-row {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 4px;
  border-radius: 4px;
}

.item-row.header {
  font-size: 11px;
  color: var(--text-secondary);
  border-bottom: 1px solid var(--border-color);
  margin-bottom: 4px;
  padding-bottom: 6px;
}

.item-row:not(.header):hover {
  background: rgba(34, 211, 238, 0.1);
}

.col-index {
  width: 18px;
  text-align: center;
  color: var(--accent-cyan);
  font-weight: 600;
  font-size: 11px;
  flex-shrink: 0;
}

.col-field {
  flex: 1;
  min-width: 0;
  text-align: center;
}

.col-field :deep(.el-input),
.col-field :deep(.el-select) {
  width: 100%;
}

.col-field :deep(.el-input__wrapper),
.col-field :deep(.el-select__wrapper) {
  padding: 2px 6px !important;
}

.col-field :deep(.el-input__inner) {
  text-align: center;
  font-size: 12px;
}

.col-action {
  width: 24px;
  flex-shrink: 0;
}

.col-action .el-button {
  width: 22px !important;
  height: 22px !important;
}

.run-button {
  width: 100%;
  height: 42px;
  font-size: 14px;
  font-weight: 600;
  margin-top: 8px;
}

/* 列表动画 */
.list-enter-active,
.list-leave-active {
  transition: all 0.3s ease;
}

.list-enter-from,
.list-leave-to {
  opacity: 0;
  transform: translateX(-20px);
}
</style>

