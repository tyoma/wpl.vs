#include <crtdbg.h>

#include <atlbase.h>

namespace
{
	class Module : public CAtlDllModuleT<Module>
	{
	public:
		Module()
		{	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	}
	} g_module;
}

STDAPI DllCanUnloadNow()
{	return g_module.DllCanUnloadNow();	}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{	return g_module.DllGetClassObject(rclsid, riid, ppv);	}
