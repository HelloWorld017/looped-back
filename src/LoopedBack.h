#include <functional>

#include <winsock2.h>
#include <Windows.h>
#include <Initguid.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <Objbase.h>
#include <Shlwapi.h>
#include <Propvarutil.h>
#include <nan.h>

#include "PolicyConfig.h"

typedef short VARIANT_BOOL;
#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

DEFINE_PROPERTYKEY(PKEY_MonitorOutput, 0x24dbb0fc, 0x9311, 0x4b3d, 0x9c, 0xf0, 0x18, 0xff, 0x15, 0x56, 0x39, 0xd4, 0);
DEFINE_PROPERTYKEY(PKEY_MonitorEnabled, 0x24dbb0fc, 0x9311, 0x4b3d, 0x9c, 0xf0, 0x18, 0xff, 0x15, 0x56, 0x39, 0xd4, 1);
DEFINE_PROPERTYKEY(
	PKEY_MonitorPauseOnBattery, 0x24dbb0fc, 0x9311, 0x4b3d, 0x9c, 0xf0, 0x18, 0xff, 0x15, 0x56, 0x39, 0xd4, 2
);

namespace LoopedBack {

class LoopedBack : public Nan::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:
		LoopedBack();
		~LoopedBack();

		bool initialized;
		IMMDeviceEnumerator* deviceEnumerator;
		IPolicyConfig* policyConfig;

		void destroyInternal();
		static void RunWithPropertyStore(
			const Nan::FunctionCallbackInfo<v8::Value>& args,
			std::function<void(IPropertyStore*)> callback
		);

		static void New(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void Destroy(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void GetDevices(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void GetLoopback(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void SetLoopback(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void GetDefaultEndpoint(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void SetDefaultEndpoint(const Nan::FunctionCallbackInfo<v8::Value>& args);
		static void IsInitialized(const Nan::FunctionCallbackInfo<v8::Value>& args);

		static inline Nan::Persistent<v8::Function> & constructor() {
			static Nan::Persistent<v8::Function> construct;
			return construct;
		}
};

}
