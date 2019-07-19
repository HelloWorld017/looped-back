const LoopedBack = require('../');

const looped = new LoopedBack;
const captureDevices = looped.getDevices(LoopedBack.DEVICE_CAPTURE);

captureDevices.forEach(({id, name}) => {
	try {
		const loopbackDevice = looped.getLoopback(id);
		console.log(`${name} => ${loopbackDevice}`);
	} catch(e) {
		console.error(`${name} ${e}`);
	}
});
