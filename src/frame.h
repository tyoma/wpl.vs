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

#include <atlbase.h>
#include <atlcom.h>

#include <wpl/concepts.h>
#include <wpl/vs/pane.h>

typedef struct IVsWindowFrame IVsWindowFrame;
typedef struct IVsWindowFrame2 IVsWindowFrame2;

namespace wpl
{
	namespace vs
	{
		class pane_impl;

		class frame : public pane, noncopyable
		{
		public:
			frame(const CComPtr<IVsWindowFrame> &underlying, pane_impl &pane_);
			~frame();

		private:
			virtual void set_root(std::shared_ptr<wpl::control> root);

			virtual rect_i get_location() const override;
			virtual void set_location(const rect_i &location) override;
			virtual void center_parent() override;
			virtual void set_visible(bool value) override;
			virtual void set_caption(const std::string &caption) override;
			virtual void set_caption_icon(const gcontext::surface_type &icon) override;
			virtual void set_task_icon(const gcontext::surface_type &icon) override;
			virtual std::shared_ptr<form> create_child() override;
			virtual void set_features(unsigned /*styles*/ features) override;

			virtual void add_command(int id, const execute_fn &execute, bool is_group, const query_state_fn &query_state,
				const get_name_fn &get_name) override;

			virtual void activate() override;

		private:
			const CComPtr<IVsWindowFrame> _underlying;
			const CComPtr<IVsWindowFrame2> _underlying2;
			pane_impl &_pane;
			DWORD_PTR _advise_cookie;
		};
	}
}
