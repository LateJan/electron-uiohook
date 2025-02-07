import { contextBridge, ipcRenderer, IpcRendererEvent } from 'electron';

type IpcRendererListener = (
    event: IpcRendererEvent,
    ...args: unknown[]
) => void;
type EventListener = (...args: unknown[]) => void;

const map = new Map<EventListener, Record<string, IpcRendererListener>>();


contextBridge.exposeInMainWorld('uioHookApi', {
    ready: () => {
        console.log('contextBridge ready');

        ipcRenderer.send('UioHookApi:ready');
    },
    reset: () => {
        console.log('contextBridge reset');

        ipcRenderer.send('UioHookApi:reset');
    },
    save: (arrayBuffer: ArrayBuffer,) => {
        console.log('contextBridge save', arrayBuffer);

        ipcRenderer.send('UioHookApi:save', Buffer.from(arrayBuffer));
    },
    on: (channel: string, fn: EventListener) => {
        console.log('contextBridge on', fn);
    
        const listener = (event: IpcRendererEvent, ...args: unknown[]) => {
          console.log('contextBridge on', channel, fn, ...args);
          fn(...args);
        };
    
        const listeners = map.get(fn) ?? {};
        listeners[channel] = listener;
        map.set(fn, listeners);
    
        ipcRenderer.on(`UioHookApi:${channel}`, listener);
      },
      off: (channel: string, fn: EventListener) => {
        console.log('contextBridge off', fn);
    
        const listeners = map.get(fn) ?? {};
        const listener = listeners[channel];
        delete listeners[channel];
    
        if (!listener) {
          return;
        }
    
        ipcRenderer.off(`UioHookApi:${channel}`, listener);
      },
});