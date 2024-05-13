class VideoCapture {
    constructor() {
        this.isMKDIR = false;
        this.cCapture = null;
    }

    capture({ file, duration, outputName }) {
        const MOUNT_DIR = '/working';

        if (!this.isMKDIR) {
            FS.mkdir(MOUNT_DIR);
            this.isMKDIR = true;
        }

        FS.mount(WORKERFS, { files: [file] }, MOUNT_DIR);

        if (!this.cCapture) {
            this.cCapture = Module.cwrap('cut_video', 'number', ['number', 'string', 'string']);
        }

        let outputPath = `${MOUNT_DIR}/${outputName}`;
        let outputCtx = this.cCapture(duration, `${MOUNT_DIR}/${file.name}`, outputPath);

        FS.unmount(MOUNT_DIR);

        if (outputCtx !== 0) {
            const outputFile = FS.readFile(outputPath);
            const outputBlob = new Blob([outputFile], { type: 'video/mp4' });

            const evt = {
                type: 'capture',
                data: {
                    outputBlob: outputBlob
                }
            };

            self.postMessage(evt, [outputBlob]);
        } else {
            const evt = {
                type: 'error',
                data: {
                    message: 'Failed to capture video'
                }
            };

            self.postMessage(evt);
        }
    }
}

const videoCapture = new VideoCapture();

let isInit = false;

self.onmessage = function (evt) {
    const evtData = evt.data;

    if (isInit && videoCapture[evtData.type]) {
        videoCapture[evtData.type](evtData.data);
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
