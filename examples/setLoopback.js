const LoopedBack = require('..');
const looped = new LoopedBack();

const captureDevice = looped.getDevices(LoopedBack.DEVICE_CAPTURE)
	.find(({name}) => name === 'CABLE Output(VB-Audio Virtual Cable)');

const renderDevice = looped.getDevices(LoopedBack.DEVICE_RENDER)
	.find(({name}) => name === '스피커(High Definition Audio 장치)');

console.log(`${captureDevice.id} => ${renderDevice.id}`);

const successful = looped.setLoopback(captureDevice.id, renderDevice.id);
console.log(`Set loopback ${successful ? 'successfully' : 'unsuccessfully'}`);
