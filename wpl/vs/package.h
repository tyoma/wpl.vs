//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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

#pragma once

#include "com.h"

#include <functional>
#include <vsshell.h>
#include <vsshell140.h>
#include <wpl/queue.h>
#include <wpl/visual.h>

namespace wpl
{
	struct form;
	struct stylesheet;

	namespace vs
	{
		class factory;

		extern const GUID c_guidGlobalCmdSet;

		class package : public freethreaded< CComObjectRootEx<CComMultiThreadModel> >,
			public IAsyncLoadablePackageInitialize,
			public IVsPackage,
			public IVsBroadcastMessageEvents
		{
		public:
			BEGIN_COM_MAP(package)
				COM_INTERFACE_ENTRY(IAsyncLoadablePackageInitialize)
				COM_INTERFACE_ENTRY(IVsPackage)
				COM_INTERFACE_ENTRY(IVsBroadcastMessageEvents)
				COM_INTERFACE_ENTRY_CHAIN(freethreaded_base)
			END_COM_MAP()

		protected:
			void obtain_service(const GUID &service_guid,
				const std::function<void (const CComPtr<IUnknown> &service)> &on_ready);

			template <typename I>
			void obtain_service(const std::function<void (const CComPtr<I> &service)> &on_ready);

			CComPtr<IVsUIShell> get_shell() const;
			const factory &get_factory() const;

		private:
			virtual wpl::clock get_clock() const = 0;
			virtual wpl::queue initialize_queue() = 0;
			virtual std::shared_ptr<wpl::stylesheet> create_stylesheet(signal<void ()> &update,
				gcontext::text_engine_type &text_services, IVsUIShell &shell,
				IVsFontAndColorStorage &font_and_color) const = 0;
			virtual void initialize(factory &factory_) = 0;
			virtual void terminate() throw() = 0;

		private:
			// IAsyncLoadablePackageInitialize methods
			STDMETHODIMP Initialize(IAsyncServiceProvider *sp, IProfferAsyncService *, IAsyncProgressCallback *, IVsTask **ppTask);

			// IVsPackage methods
			STDMETHODIMP SetSite(IServiceProvider *sp);
			STDMETHODIMP QueryClose(BOOL *can_close);
			STDMETHODIMP Close();
			STDMETHODIMP GetAutomationObject(LPCOLESTR pszPropName, IDispatch **ppDisp);
			STDMETHODIMP CreateTool(REFGUID rguidPersistenceSlot);
			STDMETHODIMP ResetDefaults(VSPKGRESETFLAGS grfFlags);
			STDMETHODIMP GetPropertyPage(REFGUID rguidPage, VSPROPSHEETPAGE *ppage);

			// IVsBroadcastMessageEvents methods
			STDMETHODIMP OnBroadcastMessage(UINT message, WPARAM wparam, LPARAM lparam);

		private:
			void obtain_root_services();
			void initialize();

		private:
			CComPtr<IServiceProvider> _service_provider;
			CComPtr<IAsyncServiceProvider> _async_service_provider;
			CComPtr<IVsShell> _shell;
			CComPtr<IVsUIShell> _shell_ui;
			CComPtr<IVsFontAndColorStorage> _fonts_and_colors;
			signal<void ()> _update_styles;
			std::shared_ptr<factory> _factory;
			VSCOOKIE _broadcast_cookie;
		};



		template <typename I>
		inline void package::obtain_service(const std::function<void (const CComPtr<I> &service)> &on_ready)
		{	obtain_service(__uuidof(I), [on_ready] (CComPtr<IUnknown> p) {	on_ready(CComQIPtr<I>(p));	});	}
	}
}
