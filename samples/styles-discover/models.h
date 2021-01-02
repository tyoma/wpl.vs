#pragma once

#include <agge/color.h>
#include <atlbase.h>
#include <vsshell80.h>
#include <wpl/models.h>

class styles_columns_model : public wpl::columns_model
{
	virtual short get_count() const throw();
	virtual void get_value(index_type index, short &value) const;
	virtual std::pair<index_type, bool> get_sort_order() const throw();
	virtual void update_column(index_type index, short width);
	virtual void get_caption(index_type index, agge::richtext_t &caption) const;
	virtual void activate_column(index_type column);
};

class styles_model : public wpl::table_model
{
public:
	styles_model(const CComPtr<IVsUIShell2> &vsshell);

	virtual index_type get_count() const throw();
	virtual void get_text(index_type row, index_type column, std::wstring &text) const;
	virtual void set_order(index_type column, bool ascending);

	agge::color get_color(index_type row) const;

private:
	CComPtr<IVsUIShell2> _vsshell;
};
