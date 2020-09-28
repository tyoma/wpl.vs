//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include <wpl/win32/font_loader.h>

#include <wpl/vs/factory.h>
#include <wpl/vs/vspackage.h>

#include "async.h"

#include <functional>
#include <logger/log.h>
#include <stdexcept>

#define PREAMBLE "Generic VS Package: "

using namespace std;

namespace wpl
{
	namespace vs
	{
		namespace
		{
			struct text_engine_composite : noncopyable
			{
				text_engine_composite()
					: text_engine(loader, 4)
				{	}

				win32::font_loader loader;
				gcontext::text_engine_type text_engine;
			};

			template <typename T>
			CComPtr<IVsTask> obtain_service(IAsyncServiceProvider *sp, const function<void (const CComPtr<T> &p)> &onready,
				const GUID &serviceid = __uuidof(T))
			{
				CComPtr<IVsTask> acquisition;

				if (S_OK != sp->QueryServiceAsync(serviceid, &acquisition))
				{
					LOGE(PREAMBLE "failed to obtain a service!");
					throw runtime_error("Cannot begin acquistion of a service!");
				}
				return async::when_complete(acquisition, VSTC_UITHREAD_NORMAL_PRIORITY,
					[onready] (_variant_t r) -> _variant_t {

					r.ChangeType(VT_UNKNOWN);
					onready(CComQIPtr<T>(r.punkVal));
					return _variant_t();
				});
			}
		}

		package::package()
			: _broadcast_cookie(0), _initialized(false)
		{	}

		CComPtr<IServiceProvider> package::get_service_provider() const
		{	return _service_provider;	}

		CComPtr<_DTE> package::get_dte() const
		{
			if (!_dte && _service_provider)
			{
				_service_provider->QueryService(__uuidof(_DTE), &_dte);
				LOG(PREAMBLE "DTE obtained on demand.") % A(_dte);
			}
			return _dte;
		}

		CComPtr<IVsUIShell> package::get_shell() const
		{	return _shell_ui;	}

		const factory &package::get_factory() const
		{	return *_factory;	}

		STDMETHODIMP package::Initialize(IAsyncServiceProvider *sp, IProfferAsyncService *, IAsyncProgressCallback *, IVsTask **ppTask)
		try
		{
			CComPtr<package> self = this;

			LOG(PREAMBLE "initializing (async)...");
			obtain_service<_DTE>(sp, [self] (CComPtr<_DTE> p) {
				LOG(PREAMBLE "DTE obtained (async)...") % A(p);
				self->_dte = p;
			});
			obtain_service<IVsShell>(sp, [self] (CComPtr<IVsShell> p) {
				LOG(PREAMBLE "VSShell obtained (async)...") % A(p);
				self->_shell = p;
				self->try_initialize();
			});
			obtain_service<IVsUIShell>(sp, [self] (CComPtr<IVsUIShell> p) {
				LOG(PREAMBLE "VsUIShell obtained (async)...") % A(p);
				self->_shell_ui = p;
				self->try_initialize();
			});
			obtain_service<IVsFontAndColorStorage>(sp, [self] (CComPtr<IVsFontAndColorStorage> p) {
				LOG(PREAMBLE "FontsAndColors obtained (async)...") % A(p);
				self->_fonts_and_colors = p;
				self->try_initialize();
				});
			ppTask = NULL;
			return S_OK;
		}
		catch (...)
		{
			LOGE(PREAMBLE "failed...");
			return E_FAIL;
		}

		STDMETHODIMP package::SetSite(IServiceProvider *sp)
		try
		{
			CComPtr<IVsUIShell> shell;

			LOG(PREAMBLE "initializing (sync)...") % A(sp);
			_service_provider = sp;
			_service_provider->QueryService(__uuidof(IVsShell), &_shell);
			_service_provider->QueryService(__uuidof(IVsUIShell), &_shell_ui);
			_service_provider->QueryService(__uuidof(IVsFontAndColorStorage), &_fonts_and_colors);
			try_initialize();
			return S_OK;
		}
		catch (...)
		{
			LOGE(PREAMBLE "failed...");
			return E_FAIL;
		}

		STDMETHODIMP package::QueryClose(BOOL *can_close)
		{
			*can_close = TRUE;
			return S_OK;
		}

		STDMETHODIMP package::Close()
		{
			LOG(PREAMBLE "shutting down package. Entering derived class termination...");
			terminate();
			LOG(PREAMBLE "... Releasing wpl supporting objects...");
			_factory.reset();
			LOG(PREAMBLE "... Releasing VS objects.");
			_shell->UnadviseBroadcastMessages(_broadcast_cookie);
			_service_provider.Release();
			_dte.Release();
			_shell.Release();
			_shell_ui.Release();
			LOG(PREAMBLE "... Everything released!");
			return S_OK;
		}

		STDMETHODIMP package::GetAutomationObject(LPCOLESTR, IDispatch **)
		{	return E_NOTIMPL;	}

		STDMETHODIMP package::CreateTool(REFGUID)
		{	return E_NOTIMPL;	}

		STDMETHODIMP package::ResetDefaults(VSPKGRESETFLAGS)
		{	return E_NOTIMPL;	}

		STDMETHODIMP package::GetPropertyPage(REFGUID, VSPROPSHEETPAGE *)
		{	return E_NOTIMPL;	}

		STDMETHODIMP package::OnBroadcastMessage(UINT message, WPARAM /*wparam*/, LPARAM /*lparam*/)
		{
			if (WM_SYSCOLORCHANGE == message)
				_update_styles();
			return S_OK;
		}

		void package::try_initialize()
		{
			if (_initialized || !_shell || !_shell_ui || !_fonts_and_colors)
				return;
			_initialized = true;

			_shell->AdviseBroadcastMessages(this, &_broadcast_cookie);

			LOG(PREAMBLE "initializing wpl support (backbuffer, renderer, text_engine, etc.)...");

			shared_ptr<text_engine_composite> tec(new text_engine_composite);
			shared_ptr<gcontext::text_engine_type> te(tec, &tec->text_engine);

			_factory = make_shared<factory>(*_shell_ui,
				make_shared<gcontext::surface_type>(1, 1, 16),
				make_shared<gcontext::renderer_type>(2),
				te,
				create_stylesheet(_update_styles, *te, *_shell_ui, *_fonts_and_colors));

			LOG(PREAMBLE "entering derived class initialization...");

			initialize(*_factory);
		}
	}
}
