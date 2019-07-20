#include <iostream>
#include "LoopedBack.h"

namespace LoopedBack {

struct InitException : public std::exception {
	const char * what () const throw () {
		return "Failed to initialize audio instance!";
	}
};

LoopedBack::LoopedBack() {
	CoInitialize(nullptr);
	HRESULT hr;

	hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator), (LPVOID *) &deviceEnumerator
	);

	if(hr != S_OK)
		throw InitException();

	hr = CoCreateInstance(
		__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC,
		IID_IPolicyConfig2, (LPVOID *)&policyConfig
	);

	if(hr != S_OK)
		hr = CoCreateInstance(
			__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC,
			IID_IPolicyConfig1, (LPVOID *)&policyConfig
		);

	if(hr != S_OK)
		hr = CoCreateInstance(
			__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC,
			IID_IPolicyConfig0, (LPVOID *)&policyConfig
		);

	if(hr != S_OK)
		throw InitException();

	initialized = true;
}

LoopedBack::~LoopedBack() {
	this->destroyInternal();
	initialized = false;
}

void LoopedBack::Init(v8::Local<v8::Object> exports) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("LoopedBack").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
	Nan::SetPrototypeMethod(tpl, "getDevices", GetDevices);
	Nan::SetPrototypeMethod(tpl, "getLoopback", GetLoopback);
	Nan::SetPrototypeMethod(tpl, "setLoopback", SetLoopback);
	Nan::SetPrototypeMethod(tpl, "getDefaultEndpoint", GetDefaultEndpoint);
	Nan::SetPrototypeMethod(tpl, "setDefaultEndpoint", SetDefaultEndpoint);
	Nan::SetPrototypeMethod(tpl, "isInitialized", IsInitialized);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(
		exports,
		Nan::New("LoopedBack").ToLocalChecked(),
		Nan::GetFunction(tpl).ToLocalChecked()
	);
}

void LoopedBack::New(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	if (args.IsConstructCall()) {
		try {
			LoopedBack *loopedBack = new LoopedBack();
			loopedBack->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		} catch(const std::exception& e) {
			return Nan::ThrowError(e.what());
		}
	}
}

void LoopedBack::destroyInternal() {
	if(deviceEnumerator) {
		deviceEnumerator->Release();
		deviceEnumerator = nullptr;
	}

	if(policyConfig) {
		policyConfig->Release();
		policyConfig = nullptr;
	}
}

void LoopedBack::Destroy(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = Nan::ObjectWrap::Unwrap<LoopedBack>(args.Holder());
	that->destroyInternal();
}

void LoopedBack::IsInitialized(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());
	args.GetReturnValue().Set(Nan::New<v8::Boolean>(that->initialized));
}

void LoopedBack::GetDevices(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());

	if(!that->deviceEnumerator) {
		return Nan::ThrowError("Failed to initialize audio instance!");
	}

	int type = args[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();
	EDataFlow dataFlowType;
	if(type == 0) {
		dataFlowType = eRender;
	} else if (type == 1) {
		dataFlowType = eCapture;
	} else {
		dataFlowType = eAll;
	}

	IMMDeviceCollection* deviceCollection;
	that->deviceEnumerator->EnumAudioEndpoints(dataFlowType, DEVICE_STATE_ACTIVE, &deviceCollection);

	UINT Count = 0;
	deviceCollection->GetCount(&Count);

	v8::Local<v8::Array> retval = Nan::New<v8::Array>(Count);

	for(UINT i = 0; i < Count; i++) {
		IMMDevice* device;
		deviceCollection->Item(i, &device);

		IPropertyStore* propertyStore;
		device->OpenPropertyStore(STGM_READ, &propertyStore);

		PROPVARIANT deviceName;
		PropVariantInit(&deviceName);
		propertyStore->GetValue(PKEY_Device_FriendlyName, &deviceName);

		LPWSTR deviceId = nullptr;
		device->GetId(&deviceId);

		v8::Local<v8::Object> obj = Nan::New<v8::Object>();
		Nan::Set(
			obj,
			Nan::New("id").ToLocalChecked(),
			Nan::New((uint16_t*) deviceId).ToLocalChecked()
		);
		Nan::Set(
			obj,
			Nan::New("name").ToLocalChecked(),
			Nan::New((uint16_t*) deviceName.pwszVal).ToLocalChecked()
		);

		Nan::Set(retval, i, obj);

		CoTaskMemFree(deviceId);
		deviceId = nullptr;

		PropVariantClear(&deviceName);

		device->Release();
		device = nullptr;

		propertyStore->Release();
		propertyStore = nullptr;
	}

	deviceCollection->Release();
	deviceCollection = nullptr;

	args.GetReturnValue().Set(retval);
}

void LoopedBack::RunWithPropertyStore(
	const Nan::FunctionCallbackInfo<v8::Value>& args,
	std::function<void(IPropertyStore*)> callback
) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());

	if(!that->deviceEnumerator) {
		return Nan::ThrowError("Failed to initialize audio instance!");
	}

	if(args.Length() == 0 || !args[0]->IsString()) {
		return Nan::ThrowTypeError("Device id should be string!");
	}

	HRESULT hr;

	LPCWSTR deviceId = (LPCWSTR) *v8::String::Value(
		args.GetIsolate(),
		args[0]->ToString(Nan::GetCurrentContext()).ToLocalChecked()
	);

	IMMDevice* device;
	hr = that->deviceEnumerator->GetDevice(deviceId, &device);
	if(hr != S_OK) {
		if(hr == E_NOTFOUND) {
			return Nan::ThrowError("No such device!");
		}

		return Nan::ThrowError("Failed to get device!");
	}

	IPropertyStore* propertyStore;
	device->OpenPropertyStore(STGM_READ, &propertyStore);

	callback(propertyStore);

	device->Release();
	device = nullptr;

	propertyStore->Release();
	propertyStore = nullptr;
}

void LoopedBack::GetLoopback(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack::RunWithPropertyStore(args, [&args](IPropertyStore* propertyStore) {
		HRESULT hr;

		PROPVARIANT monitorOutput;
		PropVariantInit(&monitorOutput);
		hr = propertyStore->GetValue(PKEY_MonitorOutput, &monitorOutput);

		if(hr == S_OK && monitorOutput.pwszVal) {
			args.GetReturnValue().Set(Nan::New((uint16_t*) monitorOutput.pwszVal).ToLocalChecked());
		} else {
			args.GetReturnValue().Set(Nan::Null());
		}

		PropVariantClear(&monitorOutput);
	});
}

void LoopedBack::SetLoopback(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());

	if(!that->policyConfig) {
		return Nan::ThrowError("Failed to initialize audio instance!");
	}

	VARIANT_BOOL loopbackEnabled = VARIANT_TRUE;

	if(args.Length() == 0 || !args[0]->IsString()) {
		return Nan::ThrowTypeError("Device id should be string!");
	}

	if(args.Length() <= 1 || !args[1]->IsString()) {
		loopbackEnabled = VARIANT_FALSE;
	}

	LPCWSTR deviceId = (LPCWSTR) *v8::String::Value(
		args.GetIsolate(),
		args[0]->ToString(Nan::GetCurrentContext()).ToLocalChecked()
	);

	LPWSTR targetId = (LPWSTR) *v8::String::Value(
		args.GetIsolate(),
		args[1]->ToString(Nan::GetCurrentContext()).ToLocalChecked()
	);

	HRESULT hr;

	PROPVARIANT setVal;
	PropVariantInit(&setVal);
	InitPropVariantFromBoolean(loopbackEnabled, &setVal);

	hr = that->policyConfig->SetPropertyValue(deviceId, 0, PKEY_MonitorEnabled, &setVal);
	PropVariantClear(&setVal);

	if(hr != S_OK) {
		args.GetReturnValue().Set(Nan::False());
		return;
	}

	InitPropVariantFromString(targetId, &setVal);
	hr = that->policyConfig->SetPropertyValue(deviceId, 0, PKEY_MonitorOutput, &setVal);
	PropVariantClear(&setVal);

	if(hr != S_OK) {
		args.GetReturnValue().Set(Nan::False());
		return;
	}

	args.GetReturnValue().Set(Nan::True());
}

void LoopedBack::GetDefaultEndpoint(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());

	if(!that->deviceEnumerator) {
		return Nan::ThrowError("Failed to initialize audio instance!");
	}

	int type = args[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();
	EDataFlow dataFlowType;
	if(type == 0) {
		dataFlowType = eRender;
	} else {
		dataFlowType = eCapture;
	}

	HRESULT hr;
	v8::Local<v8::Object> retval = Nan::New<v8::Object>();

	int i = 0;
	for(ERole role : {eConsole, eMultimedia, eCommunications}) {
		IMMDevice* device;
		hr = that->deviceEnumerator->GetDefaultAudioEndpoint(dataFlowType, role, &device);

		if(hr == S_OK) {
			LPWSTR deviceId = nullptr;
			device->GetId(&deviceId);

			Nan::Set(
				retval,
				i,
				Nan::New((uint16_t*) deviceId).ToLocalChecked()
			);

			CoTaskMemFree(deviceId);
			deviceId = nullptr;

			device->Release();
			device = nullptr;
		} else {
			Nan::Set(
				retval,
				i,
				Nan::Null()
			);
		}

		i++;
	}

	args.GetReturnValue().Set(retval);
}

void LoopedBack::SetDefaultEndpoint(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());

	if(!that->policyConfig) {
		return Nan::ThrowError("Failed to initialize audio instance!");
	}

	if(args.Length() < 2 || !args[0]->IsString()) {
		return Nan::ThrowTypeError("Wrong argument supplied!");
	}

	LPCWSTR deviceId = (LPCWSTR) *v8::String::Value(
		args.GetIsolate(),
		args[0]->ToString(Nan::GetCurrentContext()).ToLocalChecked()
	);

	int roleInt = args[1]->Uint32Value(Nan::GetCurrentContext()).FromJust();

	ERole role;
	if(roleInt == 0) {
		role = eConsole;
	} else if (roleInt == 1) {
		role = eMultimedia;
	} else {
		role = eCommunications;
	}

	HRESULT hr = that->policyConfig->SetDefaultEndpoint(deviceId, role);
	if(hr != S_OK) {
		args.GetReturnValue().Set(Nan::False());
		return;
	}

	args.GetReturnValue().Set(Nan::True());
}

NODE_MODULE(LoopedBack, LoopedBack::Init)

}
