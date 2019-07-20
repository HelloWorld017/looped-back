const {LoopedBack} = require('bindings')('LoopedBack');

LoopedBack.DEVICE_RENDER = 0;
LoopedBack.DEVICE_CAPTURE = 1;
LoopedBack.DEVICE_ALL = 2;

LoopedBack.ROLE_CONSOLE = 0;
LoopedBack.ROLE_MULTIMEDIA = 1;
LoopedBack.ROLE_COMMUNICATION = 2;

module.exports = LoopedBack;
