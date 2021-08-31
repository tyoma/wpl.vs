#pragma once

#include <agge/color.h>
#include <atlbase.h>
#include <vsshell80.h>
#include <wpl/models.h>

class styles_columns_model : public wpl::headers_model
{
	virtual index_type get_count() const throw() override;
	virtual void get_value(index_type index, short &value) const override;
	virtual void get_caption(index_type index, agge::richtext_t &caption) const override;
};

class styles_model : public wpl::richtext_table_model
{
public:
	styles_model(const CComPtr<IVsUIShell2> &vsshell);

	virtual index_type get_count() const throw() override;
	virtual void get_text(index_type row, index_type column, agge::richtext_t &text) const override;

	agge::color get_color(index_type row) const;

private:
	CComPtr<IVsUIShell2> _vsshell;
};
