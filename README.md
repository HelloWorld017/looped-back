# LoopedBack
A node module to change loopback device (a.k.a "Listen to this device") in Windows.  
Codes imported from [mrbindraw/DemoStereoMix](https://github.com/mrbindraw/DemoStereoMix)

## Building
To compile the extension for the first time, run

```
$ npm i
```

All subsequent builds only need `npm run build`

## Usage

```node
$ node
> const LoopedBack = require('looped-back');
undefined

> const looped = new LoopedBack();
undefined

> looped.isInitialized();
true

> looped.getDevices(LoopedBack.DEVICE_ALL);
[
	{
		id: '{0.0.0.00000000}.{01234567-89ab-cdef-0123-456789abcdef}',
		name: 'Speaker(High Definition Audio Device)'
	},

	{
		id: '{0.0.0.00000000}.{12345678-90ab-cdef-1234-567890abcdef}',
		name: 'Microphone(High Definition Audio Device)'
	},
	...
]

> const microphone = looped.getDevices(LoopedBack.DEVICE_CAPTURE)[0];
undefined

> const speaker = looped.getDevices(LoopedBack.DEVICE_RENDER)[0];
undefined

> looped.getLoopback(microphone.id); //returns null if loopback is disabled
null

> looped.setLoopback(microphone.id, speaker.id); //returns false if failed
true

> looped.getLoopback(microphone.id);
'{0.0.0.00000000}.{01234567-89ab-cdef-0123-456789abcdef}'

> looped.setLoopback(microphone.id); //disables loopback
true

> looped.setDefaultEndpoint(microphone.id, LoopedBack.ROLE_COMMUNICATION)
true

> looped.getDefaultEndpoint(LoopedBack.DEVICE_CAPTURE)[LoopedBack.ROLE_COMMUNICATION];
'{0.0.0.00000000}.{12345678-90ab-cdef-1234-567890abcdef}'

> looped.destroy();
undefined
```
