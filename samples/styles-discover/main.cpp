#include <atlbase.h>

namespace
{
	class Module : public CAtlDllModuleT<Module> { } g_module;
}

STDAPI DllCanUnloadNow()
{	return g_module.DllCanUnloadNow();	}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{	return g_module.DllGetClassObject(rclsid, riid, ppv);	}
