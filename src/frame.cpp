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

#include "frame.h"

#include "pane.h"

#include <logger/log.h>
#include <vsshell80.h>

#define PREAMBLE "Generic VS Frame/Form: "

using namespace std;

namespace wpl
{
	namespace vs
	{
		namespace
		{
			class frame_events_listener : public CComObjectRootEx<CComSingleThreadModel>, public IVsWindowFrameNotify
			{
			public:
				frame_events_listener()
					: _pane(nullptr)
				{	}

				void set_sink(pane &pane_)
				{	_pane = &pane_;	}

			private:
				BEGIN_COM_MAP(frame_events_listener)
					COM_INTERFACE_ENTRY(IVsWindowFrameNotify)
				END_COM_MAP()

			private:
				virtual STDMETHODIMP OnShow(FRAMESHOW fShow)
				{
					switch (fShow)
					{
					case FRAMESHOW_TabActivated: _pane->activated(); break;
					case FRAMESHOW_WinClosed: _pane->close(); break;
					}
					return S_OK;
				}

				virtual STDMETHODIMP OnMove()
				{	return S_OK;	}

				virtual STDMETHODIMP OnSize()
				{	return S_OK;	}

				virtual STDMETHODIMP OnDockableChange(BOOL /*fDockable*/)
				{	return S_OK;	}

			private:
				pane *_pane;
			};
		}

		frame::frame(const CComPtr<IVsWindowFrame> &underlying, pane_impl &pane_)
			: _underlying(underlying), _underlying2(CComQIPtr<IVsWindowFrame2>(underlying)), _pane(pane_)
		{
			_underlying->SetProperty(VSFPROPID_FrameMode, CComVariant(VSFM_MdiChild));
			if (_underlying2)
			{
				CComObject<frame_events_listener> *plistener;
				CComObject<frame_events_listener>::CreateInstance(&plistener);
				CComPtr< CComObject<frame_events_listener> > listener(plistener);

				listener->set_sink(*this);
				_underlying2->Advise(listener, &_advise_cookie);
			}
			LOG(PREAMBLE "constructed...") % A(this);
		}

		frame::~frame()
		{
			LOG(PREAMBLE "destroying...") % A(this);
			_underlying->CloseFrame(FRAMECLOSE_NoSave);
			if (_underlying2)
				_underlying2->Unadvise(_advise_cookie);
		}

		void frame::set_view(shared_ptr<wpl::view> v)
		{	_pane.host->set_view(v);	}

		view_location frame::get_location() const
		{	throw 0;	}

		void frame::set_location(const view_location &)
		{	throw 0;	}

		void frame::set_visible(bool value)
		{	value ? _underlying->Show() : _underlying->Hide();	}

		void frame::set_caption(const wstring &caption)
		{	_underlying->SetProperty(VSFPROPID_Caption, CComVariant(caption.c_str()));	}

		void frame::set_caption_icon(const gcontext::surface_type &)
		{	throw 0;	}

		void frame::set_task_icon(const gcontext::surface_type &)
		{	throw 0;	}

		shared_ptr<form> frame::create_child()
		{	throw 0;	}

		void frame::set_features(unsigned /*styles*/)
		{	throw 0;	}

		void frame::add_command(int id, const execute_fn &execute, bool is_group, const query_state_fn &query_state,
			const get_name_fn &get_name)
		{	_pane.add_command(id, execute, is_group, query_state, get_name);	}

		void frame::activate()
		{
			_underlying->Show();
			LOG(PREAMBLE "made active...") % A(this);
		}
	}
}
