#!/bin/bash

echo "===== start build ====="

NOW_PATH=$(cd $(dirname $0); pwd)

WEB_CAPTURE_PATH=$(cd $NOW_PATH/../; pwd)

cd $WEB_CAPTURE_PATH

rm -rf ./dist/
# rm -rf ./tmp/

echo "===== start build js ====="

# echo "wasm path is: $WASM_PATH"

export WASM_PATH

# 对 worker.js 进行打包构建
npm run webpack-worker

echo "===== finish build js ====="

$WEB_CAPTURE_PATH/script/build_wasm.sh

npm run webpack-capture

# rm -rf ./tmp/

echo "===== finish build ====="
