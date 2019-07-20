const LoopedBack = require('..');
const looped = new LoopedBack();

const captureDevices = looped.getDevices(LoopedBack.DEVICE_CAPTURE);
const captureDevice = captureDevices
	.find(({name}) => name === 'CABLE Output(VB-Audio Virtual Cable)');

const successful = looped.setDefaultEndpoint(captureDevice.id, LoopedBack.ROLE_COMMUNICATION);
console.log(`Set default communication device ${successful ? 'successfully' : 'unsuccessfully'}`);

const findCaptureById = id => captureDevices.find(({id: deviceId}) => deviceId === id);
const captureEndpoints = looped.getDefaultEndpoint(LoopedBack.DEVICE_CAPTURE);

console.log(`Default device: ${findCaptureById(captureEndpoints[LoopedBack.ROLE_CONSOLE]).name}`);
console.log(`Communication device: ${findCaptureById(captureEndpoints[LoopedBack.ROLE_COMMUNICATION]).name}`);
