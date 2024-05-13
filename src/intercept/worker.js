class VideoCapture {
    constructor() {
        this.isMKDIR = false;
        this.cCapture = null;
    }

    capture({ file, startTime, endTime }) {
        const MOUNT_DIR = '/working';
    
        if (!this.isMKDIR) {
            FS.mkdir(MOUNT_DIR);
            this.isMKDIR = true;
        }
    
        FS.mount(WORKERFS, { files: [file] }, MOUNT_DIR);
    
        if (!this.cCapture) {
            this.cCapture = Module.cwrap('cut_video', 'number', ['number', 'number', 'string']);
        }
    
        let outputPtr = this.cCapture(startTime, endTime, `${MOUNT_DIR}/${file.name}`);
    
        FS.unmount(MOUNT_DIR);
    
        const outputData = this._getVideoData(outputPtr);
    
        const evt = {
            type: 'capture',
            data: outputData
        };
    
        self.postMessage(evt, [outputData.buffer]);
    }
    
    _getVideoData(ptr) {
        // 假设C函数返回的是一个结构体,包含数据指针和数据长度
        let dataPtr = Module.HEAPU8.subarray(ptr, ptr + 4);
        let dataSize = Module.HEAPU8.subarray(ptr + 4, ptr + 8);
    
        dataPtr = Module.HEAPU32[dataPtr >> 2];
        dataSize = Module.HEAPU32[dataSize >> 2];
    
        let data = Module.HEAPU8.slice(dataPtr, dataPtr + dataSize);
    
        // 释放C函数分配的内存
        Module._free(dataPtr);
        Module._free(ptr);
    
        return data;
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
