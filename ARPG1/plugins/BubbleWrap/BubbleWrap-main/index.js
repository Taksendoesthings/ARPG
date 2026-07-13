/** custom configuration: */
const file = "menu.html";
const config = {
    height: 600,
    width: 800,
    icon: "images/icon.ico",
    fullscreen: false
}
const debug = false;

/** electron implementation: */
const electron = require('electron');
const { app, BrowserWindow, Menu } = electron;
if (require('electron-squirrel-startup')) app.quit();

const isMac = process.platform === 'darwin'

const template = [
  // { role: 'appMenu' }
  ...(isMac
    ? [{
        label: app.name,
        submenu: [
          { role: 'quit' }
        ]
      }]
    : []),
  // { role: 'fileMenu' }
  {
    label: 'File',
    submenu: [
      isMac ? { role: 'close' } : { role: 'quit' }
    ]
  },
  {
    label: 'View',
    submenu: [
      { role: 'togglefullscreen' }
    ]
  }
]

const menu = Menu.buildFromTemplate(template)
Menu.setApplicationMenu(menu)

let mainWindow;
const createWindow = () => {
    app.commandLine.appendSwitch('autoplay-policy', 'no-user-gesture-required');
    mainWindow = new BrowserWindow({
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
            enableRemoteModule: true,
            additionalArguments: ["volume"]
        },
        ... config
    })
    mainWindow.on('show', () => { 
        mainWindow.focus(); 
    });
    mainWindow.loadFile(file);
    mainWindow.loadFile(file);
    mainWindow.show();
    mainWindow.focus();
    // debugging html page:
    if (debug) {
      mainWindow.webContents.openDevTools();
    }
};

app.whenReady().then(() => {
    createWindow();
})
app.on('ready', function() {
    const ret = electron.globalShortcut.register('Escape', function() {
        if (mainWindow.isFullScreen()) {
            mainWindow.setFullScreen(false);
        }
    });
})
  
app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit()
})
