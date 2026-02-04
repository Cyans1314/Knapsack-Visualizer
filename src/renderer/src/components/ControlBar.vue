<script setup>
import { computed } from 'vue'
import { 
  VideoPlay, 
  VideoPause, 
  DArrowLeft, 
  DArrowRight,
  RefreshLeft,
  Delete
} from '@element-plus/icons-vue'

const props = defineProps({
  isPlaying: Boolean,
  currentStep: Number,
  totalSteps: Number,
  speed: Number,
  disabled: Boolean,
  isFinished: Boolean
})

const emit = defineEmits([
  'play', 
  'pause', 
  'step-forward', 
  'step-backward', 
  'reset',
  'clear',
  'speed-change',
  'step-change'
])

const speedOptions = [0.5, 1, 2, 4]

const progress = computed(() => {
  if (props.totalSteps === 0) return 0
  return ((props.currentStep + 1) / props.totalSteps) * 100
})

const handleSliderChange = (value) => {
  const step = Math.round((value / 100) * (props.totalSteps - 1))
  emit('step-change', step)
}
</script>

<template>
  <div class="control-bar">
    <!-- Play control buttons -->
    <div class="control-buttons">
      <el-button 
        :icon="RefreshLeft" 
        circle 
        :disabled="disabled"
        @click="$emit('reset')"
        title="Reset"
      />
      <el-button 
        :icon="Delete" 
        circle 
        :disabled="disabled"
        @click="$emit('clear')"
        title="Clear"
      />
      <el-button 
        :icon="DArrowLeft" 
        circle 
        :disabled="disabled || currentStep <= 0"
        @click="$emit('step-backward')"
        title="Previous step"
      />
      <el-button 
        v-if="!isPlaying"
        :icon="VideoPlay" 
        type="primary"
        circle 
        size="large"
        :disabled="disabled || currentStep >= totalSteps - 1"
        @click="$emit('play')"
        title="Play"
      />
      <el-button 
        v-else
        :icon="VideoPause" 
        type="warning"
        circle 
        size="large"
        @click="$emit('pause')"
        title="Pause"
      />
      <el-button 
        :icon="DArrowRight" 
        circle 
        :disabled="disabled || currentStep >= totalSteps - 1"
        @click="$emit('step-forward')"
        title="Next step"
      />
    </div>

    <!-- Progress bar -->
    <div class="progress-section">
      <el-slider 
        :model-value="progress"
        :disabled="disabled"
        :show-tooltip="false"
        @change="handleSliderChange"
      />
      <div class="progress-text">
        {{ currentStep + 1 }} / {{ isFinished ? totalSteps : '??' }}
      </div>
    </div>

    <!-- Speed control -->
    <div class="speed-section">
      <span class="speed-label">Speed:</span>
      <div class="speed-buttons">
        <button 
          v-for="s in speedOptions" 
          :key="s"
          :class="['speed-btn', { active: speed === s }]"
          @click="$emit('speed-change', s)"
        >
          {{ s }}x
        </button>
      </div>
    </div>
  </div>
</template>

<style scoped>
.control-bar {
  display: flex;
  align-items: center;
  gap: 32px;
  padding: 12px 20px;
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: 12px;
}

.control-buttons {
  display: flex;
  align-items: center;
  gap: 8px;
}

.progress-section {
  flex: 1;
  display: flex;
  align-items: center;
  gap: 16px;
}

.progress-section .el-slider {
  flex: 1;
}

.progress-text {
  font-size: 14px;
  color: var(--text-secondary);
  min-width: 80px;
  text-align: center;
}

.speed-section {
  display: flex;
  align-items: center;
  gap: 12px;
}

.speed-label {
  font-size: 14px;
  color: var(--text-secondary);
}

.speed-buttons {
  display: flex;
  gap: 4px;
}

.speed-btn {
  background: var(--bg-secondary);
  border: 1px solid var(--border-color);
  color: var(--text-secondary);
  padding: 4px 12px;
  border-radius: 6px;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s;
}

.speed-btn:hover {
  border-color: var(--accent-cyan);
}

.speed-btn.active {
  background: linear-gradient(135deg, var(--accent-cyan), var(--accent-blue));
  border-color: transparent;
  color: white;
}

/* Element Plus style override */
:deep(.el-slider__runway) {
  background: var(--bg-secondary);
}

:deep(.el-slider__bar) {
  background: linear-gradient(90deg, var(--accent-cyan), var(--accent-blue));
}

:deep(.el-slider__button) {
  border-color: var(--accent-cyan);
}
</style>

