class WebCapture {
    constructor() {
        this.isMKDIR = false;

        this.cCapture = null;
    }

    capture({ file, timeStamp }) {
        const MOUNT_DIR = '/working';

        if (!this.isMKDIR) {
            FS.mkdir(MOUNT_DIR);
            this.isMKDIR = true;
        }

        FS.mount(WORKERFS, { files: [file] }, MOUNT_DIR);

        if (!this.cCapture) {
            this.cCapture = Module.cwrap('capture', 'number', ['number', 'string']);
        }

        let imgDataPtr = this.cCapture(timeStamp, `${MOUNT_DIR}/${file.name}`);

        FS.unmount(MOUNT_DIR);

        const evt = {
            type: 'capture',
            data: this._getImageInfo(imgDataPtr)
        };

        // worker 读取到以后会推送到主线程
        self.postMessage(evt, [evt.data.imageDataBuffer.buffer]);
    }
}

const webCapture = new WebCapture();

let isInit = false;

self.onmessage = function (evt) {
    const evtData = evt.data;

    if (isInit && webCapture[evtData.type]) {
        webCapture[evtData.type](evtData.data);
    }
};

self.Module = {
    // wasm 加载完成后的回调
    instantiateWasm: (info, receiveInstance) => {
        var binary_string = self.atob(WASM_STRING);
        var len = binary_string.length;
        var bytes = new Uint8Array(len);
        for (var i = 0; i < len; i++) {
            bytes[i] = binary_string.charCodeAt(i);
        }

        WebAssembly.instantiate(bytes, info).then(result => {
            receiveInstance(result.instance);
        });
    },
    onRuntimeInitialized: () => {
        isInit = true;

        self.postMessage({
            type: 'init',
            data: {}
        });
    }
};
