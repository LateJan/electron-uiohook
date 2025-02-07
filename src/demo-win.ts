import { uIOhook, UiohookKey } from '.'

import { app, BrowserWindow } from 'electron';

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



    uIOhook.on('keydown', (e) => {
        // console.log(
        //   `${prettyModifier('ctrl', e.ctrlKey)}${prettyModifier('shift', e.shiftKey)}${prettyModifier('alt', e.altKey)}`,
        //   e.keycode,
        //   keycodeMap.get(e.keycode as any)
        // )
    
        // if (e.keycode === UiohookKey.Escape) {
        //   process.exit(0)
        // }
      })
    
      uIOhook.start()

      uIOhook.startListenClipboard();

    win.on('close', () => {
        uIOhook.stop();
        uIOhook.stopListenClipboard();
        win = null;
    });
});