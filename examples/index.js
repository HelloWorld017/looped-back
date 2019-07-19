const LoopedBack = require('../');
const looped = new LoopedBack;

const captureDevices = looped.getDevices(LoopedBack.DEVICE_CAPTURE);
const renderDevices = looped.getDevices(LoopedBack.DEVICE_RENDER);

captureDevices.forEach(({id, name}) => {
	const loopbackDeviceId = looped.getLoopback(id);
	const loopbackDevice = renderDevices.find(({id: renderDeviceId}) => renderDeviceId === loopbackDeviceId);
	console.log(`${name} => ${loopbackDevice ? loopbackDevice.name : 'None'}`);
});
