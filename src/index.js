function createWorker() {
    const workerBlob = new Blob([WORKER_STRING], {
        type: 'application/javascript'
    });

    const workerURL = URL.createObjectURL(workerBlob);

    const captureWorker = new Worker(workerURL);

    return captureWorker;
}

const captureWorker = createWorker();

const noop = function () {};

const webCapture = {
    callback: null,

    // 通知 ffmpeg 对 timeStamp 时间戳的文件进行截图
    capture(file, timeStamp, callback = noop) {
        this.callback = callback;

        captureWorker.postMessage({
            type: 'capture',
            data: {
                file,
                timeStamp: timeStamp
            }
        });
    }
};

// 主线程监听 worker 的消息
// 将 dataBuffer 转成 canvas
// 最后由 canvas 转成 base64
captureWorker.onmessage = function (evt) {
    if (evt.data.type == 'capture') {
        const { imageDataBuffer, width, height } = evt.data.data;

        let canvas = document.createElement('canvas');
        let ctx = canvas.getContext('2d');

        canvas.width = width;
        canvas.height = height;

        const imageData = new ImageData(imageDataBuffer, width, height);
        ctx.putImageData(imageData, 0, 0, 0, 0, width, height);

        webCapture.callback(canvas.toDataURL('image/jpeg'), evt.data.data);
    }
};

window.webCapture = webCapture;

export default webCapture;
