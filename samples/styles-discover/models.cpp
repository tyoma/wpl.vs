#include "models.h"

using namespace std;

namespace
{
	agge::color convert(COLORREF c)
	{	return agge::color::make(GetRValue(c), GetGValue(c), GetBValue(c));	}
}

short styles_columns_model::get_count() const throw()
{	return 5;	}

void styles_columns_model::get_value(index_type index, short &value) const
{
	switch (index)
	{
	case 0: value = 60; break;
	case 1: value = 80; break;
	case 2: value = 50; break;
	case 3: value = 50; break;
	case 4: value = 50; break;
	}
}

pair<styles_columns_model::index_type, bool> styles_columns_model::get_sort_order() const throw()
{	return make_pair((short)-1, false);	}

void styles_columns_model::update_column(index_type /*index*/, short /*width*/)
{	}

void styles_columns_model::get_caption(index_type index, agge::richtext_t &caption) const
{
	switch (index)
	{
	case 0: caption.append(L"Code"); break;
	case 1: caption.append(L"Color"); break;
	case 2: caption.append(L"Red"); break;
	case 3: caption.append(L"Green"); break;
	case 4: caption.append(L"Blue"); break;
	}
}

void styles_columns_model::activate_column(index_type /*column*/)
{	}


styles_model::styles_model(const CComPtr<IVsUIShell2> &vsshell)
	: _vsshell(vsshell)
{	}

styles_model::index_type styles_model::get_count() const throw()
{	return 250u;	}

void styles_model::get_text(index_type row, index_type column, wstring &text) const
{
	const long long color_index = 40 - (int)row;

	switch (column)
	{
	case 0:
		text = std::to_wstring(color_index);
		break;

	case 2:
	case 3:
	case 4:
		DWORD clr;
		if (S_OK == _vsshell->GetVSSysColorEx(static_cast<VSSYSCOLOREX>(color_index), &clr))
		{
			auto c = convert(clr);

			switch (column)
			{
			case 2:	text = std::to_wstring((long long)c.r); break;
			case 3:	text = std::to_wstring((long long)c.g); break;
			case 4:	text = std::to_wstring((long long)c.b); break;
			}
		}
		else
		{
			text = L"N/A";
		}
		break;
	}
}

void styles_model::set_order(index_type /*column*/, bool /*ascending*/)
{	}

agge::color styles_model::get_color(index_type row) const
{
	DWORD clr;

	return S_OK == _vsshell->GetVSSysColorEx(static_cast<VSSYSCOLOREX>(40 - (int)row), &clr)
		? convert(clr) : agge::color::make(0, 0, 0, 0);
}
