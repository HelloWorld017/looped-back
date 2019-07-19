const LoopedBack = require('../');

const looped = new LoopedBack;

const captureDevices = looped.getDevices(LoopedBack.DEVICE_CAPTURE);

const captureDevice = captureDevices
	.find(device => device.name === 'CABLE Output(VB-Audio Virtual Cable)');

const renderDevice = looped.getDevices(LoopedBack.DEVICE_RENDER)
	.find(device => device.name === '스피커(High Definition Audio 장치)');

looped.setLoopback(captureDevice.id, renderDevice.id);

console.log("Done setting loopback");

captureDevices.forEach(({id, name}) => {
	try {
		const loopbackDevice = looped.getLoopback(id);
		console.log(`${name} => ${loopbackDevice}`);
	} catch(e) {
		console.error(`${name} ${e}`);
	}
});
