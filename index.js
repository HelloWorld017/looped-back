const {LoopedBack} = require('bindings')('LoopedBack');
LoopedBack.DEVICE_RENDER = 0;
LoopedBack.DEVICE_CAPTURE = 1;
LoopedBack.DEVICE_ALL = 2;

module.exports = LoopedBack;
