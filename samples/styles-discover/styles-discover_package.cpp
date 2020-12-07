#include <agge.text/text_engine.h>

#include "models.h"
#include "resources/command-ids.h"

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <atlbase.h>
#include <stdexcept>
#include <wpl/controls/listview_basic.h>
#include <wpl/controls/listview_composite.h>
#include <wpl/layout.h>
#include <wpl/stylesheet_db.h>
#include <wpl/stylesheet_helpers.h>
#include <wpl/vs/factory.h>
#include <wpl/vs/ole-command-target.h>
#include <wpl/vs/pane.h>
#include <wpl/vs/package.h>

using namespace agge;
using namespace std;
using namespace wpl;

extern const GUID c_guidStylesDiscoverPkg = guidStylesDiscoverPkg;
extern const GUID c_guidGlobalCmdSet = guidGlobalCmdSet;

namespace
{
	typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender;

	void inflate(rect_r &r, real_t dx, real_t dy)
	{	r.x1 -= dx, r.x2 += dx, r.y1 -= dy, r.y2 += dy;	}

	shared_ptr<agge::font> create(gcontext::text_engine_type &text_engine, const LOGFONTW &native_font)
	{
		return text_engine.create_font(native_font.lfFaceName, native_font.lfHeight, native_font.lfWeight > FW_NORMAL,
			!!native_font.lfItalic, agge::font::key::gf_vertical);
	}

	shared_ptr<agge::font> get_system_font(gcontext::text_engine_type &text_engine)
	{
		NONCLIENTMETRICSW m = {};

		m.cbSize = sizeof(m);
		if (::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &m, 0))
			return create(text_engine, m.lfMenuFont);
		throw runtime_error("Cannot retrieve system font!");
	}

	color get_system_color(int color_kind)
	{
		auto c = ::GetSysColor(color_kind);
		return color::make(GetRValue(c), GetGValue(c), GetBValue(c));
	}

	color invert(color original)
	{	return color::make(~original.r, ~original.g, ~original.b, original.a);	}

	color semi(color original, double opacity)
	{	return color::make(original.r, original.g, original.b, static_cast<uint8_t>(opacity * 255));	}



	class styleview : public controls::listview_basic
	{
	public:
		styleview()
		{	}

		void apply_styles(const stylesheet &ss)
		{
			listview_basic::apply_styles(ss);
			_padding = ss.get_value("padding.listview");
			_border = ss.get_value("border");
		}

		void set_model(shared_ptr<styles_model> m)
		{
			controls::listview_basic::set_model(m);
			_model = m;
		}

		virtual void draw_subitem(wpl::gcontext &ctx, wpl::gcontext::rasterizer_ptr &rasterizer_,
			const rect_r &box, index_type item, unsigned state, wpl::columns_model::index_type subitem,
			const std::wstring &text) const
		{
			if (subitem == 1)
			{
				if (_model)
				{
					auto b = box;

					inflate(b, -_padding, -_padding);
					add_path(*rasterizer_, rectangle(b.x1, b.y1 + _border, b.x2, b.y2));
					ctx(rasterizer_, blender(_model->get_color(item)), winding<>());
				}
			}
			else
			{
				controls::listview_basic::draw_subitem(ctx, rasterizer_, box, item, state, subitem, text);
			}
		}

	private:
		real_t _padding, _border;
		shared_ptr<styles_model> _model;
	};
}

class styles_discover_package : public vs::package,
	public CComCoClass<styles_discover_package, &c_guidStylesDiscoverPkg>,
	public vs::ole_command_target
{
public:
	styles_discover_package()
		: vs::ole_command_target(c_guidGlobalCmdSet)
	{	}

public:
	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(styles_discover_package)
		COM_INTERFACE_ENTRY(IOleCommandTarget)
		COM_INTERFACE_ENTRY_CHAIN(vs::package)
	END_COM_MAP()

private:
	virtual std::shared_ptr<stylesheet> create_stylesheet(signal<void ()> &, gcontext::text_engine_type &text_engine,
		IVsUIShell &/*shell*/, IVsFontAndColorStorage &/*font_and_color*/) const
	{
		const auto ss = make_shared<stylesheet_db>();
		const auto background = get_system_color(COLOR_BTNFACE);

		ss->set_color("background", background);
		ss->set_color("text", get_system_color(COLOR_BTNTEXT));
		ss->set_color("text.selected", get_system_color(COLOR_HIGHLIGHTTEXT));
		ss->set_color("border", semi(get_system_color(COLOR_3DSHADOW), 0.3));

		ss->set_color("background.listview.even", color::make(0, 0, 0, 0));
		ss->set_color("background.listview.odd", semi(invert(background), 0.02));
		ss->set_color("background.selected", get_system_color(COLOR_HIGHLIGHT));

		ss->set_color("background.header.sorted", semi(invert(background), 0.05));

		ss->set_value("border", 1.0f);
		ss->set_value("padding", 3.0f);

		ss->set_font("text", text_engine.create_font(L"Segoe UI", 20, false, false, agge::font::key::gf_vertical));
		return ss;
	}

	virtual void initialize(vs::factory &factory_)
	{
		factory_.register_control("styleview", [] (const factory &f, const control_context &context) {
			return apply_stylesheet(make_shared< wpl::controls::listview_composite<styleview> >(f, context), *context.stylesheet_);
		});

		add_command(cmdidShowStyles, [this, &factory_] (unsigned) {
			_active_pane = factory_.create_pane(GUID_NULL, 0);
			auto &active_pane = _active_pane;

			_active_pane->set_caption(L"VS Styles");

			const auto root = make_shared<overlay>();
				root->add(factory_.create_control<control>("background"));

				const auto sv = factory_.create_control<styleview>("styleview");
				root->add(sv);
					sv->set_model(make_shared<styles_model>(CComQIPtr<IVsUIShell2>(get_shell())));
					sv->set_columns_model(make_shared<styles_columns_model>());

			_active_pane->set_root(root);
			_active_pane->set_visible(true);
			_close_conn = _active_pane->close += [&active_pane] { active_pane.reset(); };
		}, false, [this] (unsigned, unsigned &state) -> bool {
			state = supported | visible | (!_active_pane ? enabled : 0);
			return true;
		});
	}

	virtual void terminate() throw()
	{
		_active_pane.reset();
	}

private:
	slot_connection _close_conn;
	shared_ptr<vs::pane> _active_pane;
};

OBJECT_ENTRY_AUTO(c_guidStylesDiscoverPkg, styles_discover_package);
