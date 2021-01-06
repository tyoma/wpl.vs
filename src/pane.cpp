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

#include "pane.h"

#include <logger/log.h>
#include <wpl/stylesheet.h>
#include <wpl/win32/view_host.h>

#define PREAMBLE "Generic VS Pane: "

namespace wpl
{
	namespace vs
	{
		pane_impl::pane_impl(const GUID &menu_group_id)
			: ole_command_target(menu_group_id)
		{	LOG(PREAMBLE "constructed...") % A(this);	}

		pane_impl::~pane_impl()
		{	LOG(PREAMBLE "destroyed...") % A(this);	}

		STDMETHODIMP pane_impl::SetSite(IServiceProvider * /*site*/)
		{	return S_OK;	}

		STDMETHODIMP pane_impl::CreatePaneWindow(HWND hparent, int, int, int, int, HWND *)
		{
			LOG(PREAMBLE "CreatePaneWindow called. Constructing view_host...") % A(this);
			host.reset(new win32::view_host(hparent, context));
			::SetClassLongPtr(hparent, GCL_STYLE, CS_DBLCLKS | ::GetClassLongPtr(hparent, GCL_STYLE));
			LOG(PREAMBLE "...view_host constructed.") % A(host.get());
			return S_OK;
		}

		STDMETHODIMP pane_impl::GetDefaultSize(SIZE *)
		{	return S_FALSE;	}

		STDMETHODIMP pane_impl::ClosePane()
		{
			LOG(PREAMBLE "ClosePane called. Releasing...") % A(this);
			return S_OK;
		}

		STDMETHODIMP pane_impl::LoadViewState(IStream *)
		{	return E_NOTIMPL;	}

		STDMETHODIMP pane_impl::SaveViewState(IStream *)
		{	return E_NOTIMPL;	}

		STDMETHODIMP pane_impl::TranslateAccelerator(MSG *)
		{	return E_NOTIMPL;	}
	}
}
