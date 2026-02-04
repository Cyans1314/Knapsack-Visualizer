/**
 * Electron Preload Script
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Exposes IPC API to renderer process in a secure context
 */

import { contextBridge, ipcRenderer } from 'electron'
import { electronAPI } from '@electron-toolkit/preload'

// API exposed to renderer process
const api = {
  // Run knapsack algorithm
  runAlgorithm: (algorithm, params) => {
    console.log('runAlgorithm called:', algorithm, params)
    return ipcRenderer.invoke('run-algorithm', { algorithm, params })
  }
}

console.log('Preload script loaded, contextIsolated:', process.contextIsolated)

if (process.contextIsolated) {
  try {
    contextBridge.exposeInMainWorld('electron', electronAPI)
    contextBridge.exposeInMainWorld('api', api)
    console.log('API exposed successfully')
  } catch (error) {
    console.error('Failed to expose API:', error)
  }
} else {
  window.electron = electronAPI
  window.api = api
  console.log('API attached to window directly')
}
