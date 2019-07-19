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
		IID_IPolicyConfig2, (LPVOID *) &policyConfig
	);

	if(hr != S_OK)
		hr = CoCreateInstance(
			__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC,
			IID_IPolicyConfig1, (LPVOID *) &policyConfig
		);

	if(hr != S_OK)
		hr = CoCreateInstance(
			__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC,
			IID_IPolicyConfig0, (LPVOID *) &policyConfig
		);

	if(hr != S_OK)
		throw InitException();
}

LoopedBack::~LoopedBack() {
	this->destroyInternal();
}

void LoopedBack::Init(v8::Local<v8::Object> exports) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("LoopedBack").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
	Nan::SetPrototypeMethod(tpl, "getDevices", GetDevices);
	Nan::SetPrototypeMethod(tpl, "getLoopback", GetLoopback);
	Nan::SetPrototypeMethod(tpl, "setLoopback", SetLoopback);

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
	
}

void LoopedBack::GetLoopback(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	LoopedBack* that = ObjectWrap::Unwrap<LoopedBack>(args.Holder());

	if(!that->policyConfig || !that->deviceEnumerator) {
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

	PROPVARIANT monitorOutput;
	PropVariantInit(&monitorOutput);
	hr = propertyStore->GetValue(PKEY_MonitorOutput, &monitorOutput);

	if(hr == S_OK && monitorOutput.pwszVal) {
		args.GetReturnValue().Set(Nan::New((uint16_t*) monitorOutput.pwszVal).ToLocalChecked());
	} else {
		args.GetReturnValue().Set(Nan::Null());
	}

	PropVariantClear(&monitorOutput);

	device->Release();
	device = nullptr;

	propertyStore->Release();
	propertyStore = nullptr;
}

void LoopedBack::SetLoopback(const Nan::FunctionCallbackInfo<v8::Value>& args) {

}

NODE_MODULE(LoopedBack, LoopedBack::Init)

}
