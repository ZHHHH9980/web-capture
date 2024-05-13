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

const videoCapture = {
    callback: null,

    // 通知 worker 对视频文件进行截取
    capture(file, duration, outputName, callback = noop) {
        this.callback = callback;

        captureWorker.postMessage({
            type: 'capture',
            data: {
                file,
                duration,
                outputName
            }
        });
    }
};

// 主线程监听 worker 的消息
captureWorker.onmessage = function (evt) {
    if (evt.data.type === 'capture') {
        const { outputBlob } = evt.data.data;

        videoCapture.callback(outputBlob);
    } else if (evt.data.type === 'error') {
        console.error('Video capture failed:', evt.data.data.message);
    }
};

window.videoCapture = videoCapture;

export default videoCapture;
