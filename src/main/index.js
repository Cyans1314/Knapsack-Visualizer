/**
 * Electron Main Process
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Handles application window management, IPC communication, and C++ algorithm invocation
 */

import { app, shell, BrowserWindow, ipcMain } from 'electron'
import { join } from 'path'
import { spawn } from 'child_process'
import { electronApp, optimizer, is } from '@electron-toolkit/utils'
import icon from '../../resources/icon.png?asset'

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    show: false,
    autoHideMenuBar: true,
    ...(process.platform === 'linux' ? { icon } : {}),
    webPreferences: {
      preload: join(__dirname, '../preload/index.js'),
      sandbox: false
    }
  })

  mainWindow.on('ready-to-show', () => {
    mainWindow.show()
  })

  mainWindow.webContents.setWindowOpenHandler((details) => {
    shell.openExternal(details.url)
    return { action: 'deny' }
  })

  if (is.dev && process.env['ELECTRON_RENDERER_URL']) {
    mainWindow.loadURL(process.env['ELECTRON_RENDERER_URL'])
  } else {
    mainWindow.loadFile(join(__dirname, '../renderer/index.html'))
  }
}

// Get C++ executable file path
function getCppPath(algorithmName) {
  if (is.dev) {
    return join(__dirname, '../../cpp', `${algorithmName}.exe`)
  } else {
    return join(process.resourcesPath, 'cpp', `${algorithmName}.exe`)
  }
}

// Run knapsack algorithm
function runKnapsackAlgorithm(algorithm, params) {
  return new Promise((resolve, reject) => {
    const exePath = getCppPath(algorithm)
    
    // Build command line arguments based on algorithm type
    let args = [params.capacity.toString(), params.items.length.toString()]
    
    params.items.forEach(item => {
      let itemStr = `${item.weight},${item.value}`
      
      // Multiple knapsack: add quantity
      if (item.count !== undefined) {
        itemStr += `,${item.count}`
      }
      // Mixed knapsack: add type
      if (item.type !== undefined) {
        itemStr += `,${item.type}`
      }
      // Two-dimensional cost: add volume
      if (item.volume !== undefined) {
        itemStr = `${item.weight},${item.volume},${item.value}`
      }
      // Group knapsack: add group ID
      if (item.group !== undefined) {
        itemStr += `,${item.group}`
      }
      // Dependency knapsack: add parent item ID
      if (item.parent !== undefined) {
        itemStr += `,${item.parent}`
      }
      
      args.push(itemStr)
    })
    
    // Two-dimensional cost: add second capacity
    if (params.capacity2 !== undefined) {
      args.splice(1, 0, params.capacity2.toString())
    }
    
    // Kth optimal solution: add K value
    if (params.k !== undefined) {
      args.splice(1, 0, params.k.toString())
    }
    
    console.log('Running:', exePath, args)
    
    const child = spawn(exePath, args)
    let stdout = ''
    let stderr = ''
    
    child.stdout.on('data', (data) => {
      stdout += data.toString()
    })
    
    child.stderr.on('data', (data) => {
      stderr += data.toString()
    })
    
    child.on('close', (code) => {
      if (code === 0) {
        try {
          const result = JSON.parse(stdout)
          resolve(result)
        } catch (e) {
          reject(new Error(`JSON parsing failed: ${e.message}\nOutput: ${stdout}`))
        }
      } else {
        reject(new Error(`Process exit code: ${code}\nError: ${stderr}\nOutput: ${stdout}`))
      }
    })
    
    child.on('error', (err) => {
      reject(new Error(`Failed to start: ${err.message}\nPlease ensure compiled: ${exePath}`))
    })
  })
}

app.whenReady().then(() => {
  electronApp.setAppUserModelId('com.electron')

  app.on('browser-window-created', (_, window) => {
    optimizer.watchWindowShortcuts(window)
  })

  // IPC: Run algorithm
  ipcMain.handle('run-algorithm', async (event, { algorithm, params }) => {
    try {
      const result = await runKnapsackAlgorithm(algorithm, params)
      return { success: true, data: result }
    } catch (error) {
      return { success: false, error: error.message }
    }
  })

  createWindow()

  app.on('activate', function () {
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})
