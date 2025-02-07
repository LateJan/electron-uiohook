import { uIOhook, UiohookKey } from '.'

import { app, BrowserWindow, clipboard } from 'electron';

const keycodeMap = new Map(Object.entries(UiohookKey).map(_ => [_[1], _[0]]))

let win: BrowserWindow | null;

console.log('start app');

function prettyModifier (name: string, state: boolean) {
    return state ? `[${name}]` : ` ${name} `
  }
  

app.whenReady().then(() => {
    win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: { nodeIntegration: true }
    });

    win.loadURL("https://www.baidu.com");

    uIOhook.on('clipboardChanged', () => {
      console.log('clipboard changed:', clipboard.readText())
    })
    
      uIOhook.start()

      uIOhook.startListenClipboard();

    win.on('close', () => {
        uIOhook.stop();
        uIOhook.stopListenClipboard();
        win = null;
    });
});