# electron-uiohook

[![](https://img.shields.io/npm/v/electron-uiohook/latest?color=CC3534&label=electron-uiohook&logo=npm&labelColor=212121)](https://www.npmjs.com/package/electron-uiohook)

N-API C-bindings for [libuiohook](https://github.com/kwhat/libuiohook).


### Usage example

```typescript
import { uIOhook, UiohookKey } from 'electron-uiohook'
import { clipboard } from 'electron'

uIOhook.on('keydown', (e) => {
  if (e.keycode === UiohookKey.Q) {
    console.log('Hello!')
  }

  if (e.keycode === UiohookKey.Escape) {
    process.exit(0)
  }
})

uIOhook.start()

// clipboard watcher
uIOhook.on('clipboardChanged', () => {
  console.log('clipboard changed', clipboard.readText())
})

uIOhook.startListenClipboard()

```

### API

```typescript
interface UiohookNapi {
  on(event: 'input', listener: (e: UiohookKeyboardEvent | UiohookMouseEvent | UiohookWheelEvent) => void): this

  on(event: 'keydown', listener: (e: UiohookKeyboardEvent) => void): this
  on(event: 'keyup', listener: (e: UiohookKeyboardEvent) => void): this

  on(event: 'mousedown', listener: (e: UiohookMouseEvent) => void): this
  on(event: 'mouseup', listener: (e: UiohookMouseEvent) => void): this
  on(event: 'mousemove', listener: (e: UiohookMouseEvent) => void): this
  on(event: 'click', listener: (e: UiohookMouseEvent) => void): this

  on(event: 'wheel', listener: (e: UiohookWheelEvent) => void): this

  keyTap(key: keycode, modifiers?: keycode[])
  keyToggle(key: keycode, toggle: 'down' | 'up')
}

export interface UiohookKeyboardEvent {
  altKey: boolean
  ctrlKey: boolean
  metaKey: boolean
  shiftKey: boolean
  keycode: number
}

export interface UiohookMouseEvent {
  altKey: boolean
  ctrlKey: boolean
  metaKey: boolean
  shiftKey: boolean
  x: number
  y: number
  button: unknown
  clicks: number
}

export interface UiohookWheelEvent {
  altKey: boolean
  ctrlKey: boolean
  metaKey: boolean
  shiftKey: boolean
  x: number
  y: number
  clicks: number
  amount: number
  direction: WheelDirection
  rotation: number
}
```
