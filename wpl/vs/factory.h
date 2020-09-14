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

#pragma once

#include <wpl/factory.h>

typedef struct _GUID GUID;
typedef struct IVsUIShell IVsUIShell;

namespace wpl
{
	namespace vs
	{
		struct pane;

		class factory : public wpl::factory
		{
		public:
			factory(IVsUIShell &shell, const std::shared_ptr<gcontext::surface_type> &backbuffer,
				const std::shared_ptr<gcontext::renderer_type> &renderer,
				const std::shared_ptr<gcontext::text_engine_type> &text_engine,
				const std::shared_ptr<stylesheet> &stylesheet_);
			~factory();

			std::shared_ptr<pane> create_pane(const GUID &menu_group_id, int menu_id) const;
			std::shared_ptr<form> create_modal() const;

		private:
			IVsUIShell &_shell;
			const std::shared_ptr<gcontext::surface_type> _backbuffer;
			const std::shared_ptr<gcontext::renderer_type> _renderer;
			const std::shared_ptr<gcontext::text_engine_type> _text_engine;
			const std::shared_ptr<stylesheet> _stylesheet;
			mutable unsigned _next_tool_id;
		};
	}
}
