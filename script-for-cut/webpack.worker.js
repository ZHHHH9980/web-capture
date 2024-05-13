const path = require('path');
const webpack = require('webpack');

//  worker.js -> ./tmp/worker.js -> /tmp/intercept.js -> let workerStr// finally -> worker string + ../src/intercept/index.js -> ../dist/intercept/web-capture.js
// finally -> worker string + ../src/intercept/index.js -> ../dist/intercept/web-capture.js
module.exports = {
    mode: 'production',
    entry: path.join(__dirname, '../src/intercept/worker.js'),
    output: {
        filename: 'worker.js',
        path: path.resolve(__dirname, '../tmp')
    },
    module: {
        rules: [
            {
                test: /\.(js)$/,
                exclude: /node_modules/,
                use: ['babel-loader']
            }
        ]
    },
    plugins: [
        new webpack.DefinePlugin({
            WASM_PATH: JSON.stringify(`${process.env.WASM_PATH}`)
        })
    ]
};
