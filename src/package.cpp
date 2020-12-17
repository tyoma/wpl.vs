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

#include <wpl/vs/package.h>

#include "async.h"

#include <wpl/vs/factory.h>

#include <functional>
#include <logger/log.h>
#include <stdexcept>
#include <wpl/freetype2/font_loader.h>
#include <wpl/win32/cursor_manager.h>
#include <wpl/win32/queue.h>

#define PREAMBLE "Generic VS Package: "

using namespace std;

namespace wpl
{
	namespace vs
	{
		void package::obtain_service(const GUID &service_guid,
			const function<void (const CComPtr<IUnknown> &service)> &on_ready)
		{
			if (_async_service_provider)
			{
				CComPtr<IVsTask> acquisition;

				if (S_OK != _async_service_provider->QueryServiceAsync(service_guid, &acquisition))
				{
					LOGE(PREAMBLE "failed to obtain a service!");
					throw runtime_error("Cannot begin acquistion of a service!");
				}
				async::when_complete(acquisition, VSTC_UITHREAD_NORMAL_PRIORITY,
					[on_ready] (_variant_t r) -> _variant_t {

						r.ChangeType(VT_UNKNOWN);
						on_ready(r.punkVal);
						return _variant_t();
				});
			}
			else
			{
				CComPtr<IUnknown> service;

				_service_provider->QueryService(service_guid, &service);
				on_ready(service);
			}
		}

		CComPtr<IVsUIShell> package::get_shell() const
		{	return _shell_ui;	}

		const factory &package::get_factory() const
		{	return *_factory;	}

		STDMETHODIMP package::Initialize(IAsyncServiceProvider *sp, IProfferAsyncService *, IAsyncProgressCallback *, IVsTask **ppTask)
		try
		{
			LOG(PREAMBLE "initializing (async)...") % A(sp);

			_async_service_provider = sp;
			*ppTask = NULL;
			obtain_root_services();
			return S_OK;
		}
		catch (...)
		{
			return E_FAIL;
		}

		STDMETHODIMP package::SetSite(IServiceProvider *sp)
		try
		{
			CComPtr<IVsShell> shell;
			CComVariant version;

			if (S_OK != sp->QueryService(__uuidof(IVsShell), &shell) || !shell)
				return S_OK;
			if (S_OK == shell->GetProperty(static_cast<VSPROPID>(-9068), &version) && S_OK == version.ChangeType(VT_BSTR))
			{
				wstring sversion = version.bstrVal;
				size_t pos = sversion.find(L'.');
				float v;

				sversion = sversion.substr(0, pos != wstring::npos ? sversion.find(L'.', pos + 1) : pos);
				swscanf(sversion.c_str(), L"%f", &v);
				if (v >= 14.0)
					return S_OK;
			}

			LOG(PREAMBLE "initializing (sync)...") % A(sp);

			_service_provider = sp;
			obtain_root_services();
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
			_fonts_and_colors.Release();
			_shell_ui.Release();
			if (_shell)
				_shell->UnadviseBroadcastMessages(_broadcast_cookie);
			_shell.Release();
			_async_service_provider.Release();
			_service_provider.Release();
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

		void package::obtain_root_services()
		{
			CComPtr<package> self = this;

			obtain_service<IVsUIShell>([self] (CComPtr<IVsUIShell> p) {
				LOG(PREAMBLE "VsUIShell obtained...") % A(p);
				self->_shell_ui = p;
				if (!!self->_fonts_and_colors)
					self->initialize();
			});
			obtain_service<IVsFontAndColorStorage>([self] (CComPtr<IVsFontAndColorStorage> p) {
				LOG(PREAMBLE "FontsAndColors obtained...") % A(p);
				self->_fonts_and_colors = p;
				if (!!self->_shell_ui)
					self->initialize();
			});
		}

		void package::initialize()
		{
			CComPtr<package> self = this;

			obtain_service<IVsShell>([self] (CComPtr<IVsShell> p) {
				LOG(PREAMBLE "VsShell obtained...") % A(p);
				self->_shell = p;
				p->AdviseBroadcastMessages(&*self, &self->_broadcast_cookie);
			});

			LOG(PREAMBLE "initializing wpl support (backbuffer, renderer, text_engine, etc.)...");

			const auto tex_engine = create_text_engine();
			factory_context context = {
				make_shared<gcontext::surface_type>(1, 1, 16),
				make_shared<gcontext::renderer_type>(2),
				tex_engine,
				create_stylesheet(_update_styles, *tex_engine, *_shell_ui, *_fonts_and_colors),
				make_shared<win32::cursor_manager>(),
				win32::clock,
				win32::queue(),
			};

			_factory = make_shared<factory>(context, *_shell_ui);

			LOG(PREAMBLE "entering derived class initialization...");

			initialize(*_factory);
		}
	}
}
