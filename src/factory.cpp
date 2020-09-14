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

#include <wpl/vs/factory.h>

#include "CComObjectEx.h"
#include "frame.h"
#include "pane.h"

#include <logger/log.h>
#include <vsshell.h>
#include <wpl/win32/form.h>

#define PREAMBLE "WPL/VS Factory: "

using namespace std;

namespace wpl
{
	namespace vs
	{
		namespace
		{
			HWND get_frame_hwnd(IVsUIShell &shell)
			{
				HWND hparent = HWND_DESKTOP;

				return shell.GetDialogOwnerHwnd(&hparent), hparent;
			}
		}

		factory::factory(IVsUIShell &shell, const shared_ptr<gcontext::surface_type> &backbuffer,
				const shared_ptr<gcontext::renderer_type> &renderer,
				const shared_ptr<gcontext::text_engine_type> &text_engine,
				const shared_ptr<stylesheet> &stylesheet_)
			: wpl::factory(backbuffer, renderer, text_engine, stylesheet_), _shell(shell), _backbuffer(backbuffer),
				_renderer(renderer), _text_engine(text_engine), _stylesheet(stylesheet_)
		{
			setup_default(*this);
			register_form([this] (const shared_ptr<gcontext::surface_type> &backbuffer,
				const shared_ptr<gcontext::renderer_type> &renderer,
				const shared_ptr<gcontext::text_engine_type> &text_engine,
				const shared_ptr<stylesheet> &/*stylesheet_*/) {

				return shared_ptr<form>(new win32::form(backbuffer, renderer, text_engine, get_frame_hwnd(_shell)));
			});

			_shell.AddRef();
		}

		factory::~factory()
		{	_shell.Release();	}

		shared_ptr<pane> factory::create_pane(const GUID &menu_group_id, int menu_id) const
		{
			// {BED6EA71-BEE3-4E71-AFED-CFFBA521CF46}
			const GUID c_settingsSlot = { 0xbed6ea71, 0xbee3, 0x4e71, { 0xaf, 0xed, 0xcf, 0xfb, 0xa5, 0x21, 0xcf, 0x46 } };

			CComObjectEx<pane_impl> *pane_object;
			CComObjectEx<pane_impl>::CreateInstance(&pane_object, menu_group_id);
			CComPtr<IVsWindowPane> sp(pane_object);
			CComPtr<IVsWindowFrame> frame_;

			pane_object->backbuffer = _backbuffer;
			pane_object->renderer = _renderer;
			pane_object->text_engine = _text_engine;
			pane_object->stylesheet_ = _stylesheet;
			LOG(PREAMBLE "creating VS pane window...") % A(pane_object) % A(_next_tool_id);
			if (S_OK == _shell.CreateToolWindow(CTW_fMultiInstance | CTW_fToolbarHost, _next_tool_id++, sp, GUID_NULL, c_settingsSlot,
				GUID_NULL, NULL, L"", NULL, &frame_) && frame_)
			{
				CComVariant vtbhost;

				if (S_OK == frame_->GetProperty(VSFPROPID_ToolbarHost, &vtbhost) && vtbhost.punkVal)
				{
					if (CComQIPtr<IVsToolWindowToolbarHost> tbhost = vtbhost.punkVal)
						tbhost->AddToolbar(VSTWT_TOP, &menu_group_id, menu_id);
				}

				LOG(PREAMBLE "frame created.") % A(frame_);
				return make_shared<frame>(frame_, *pane_object);
			}
			LOGE(PREAMBLE "failed to create frame!");
			return shared_ptr<frame>();
		}

		shared_ptr<form> factory::create_modal() const
		{
			const auto new_form = create_form();
			const auto hroot = get_frame_hwnd(_shell);
			const shared_ptr< shared_ptr<form> > composite(new shared_ptr<form>(new_form),
				[hroot] (shared_ptr<form> *p) {
				delete p;
				::EnableWindow(hroot, TRUE);
			});

			::EnableWindow(hroot, FALSE);
			return shared_ptr<form>(composite, composite->get());
		}
	}
}
