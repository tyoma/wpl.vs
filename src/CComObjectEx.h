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

namespace ATL
{
	template <class Base>
	class CComObjectEx : 
		public Base
	{
	public:
		typedef Base _BaseClass;

		template <typename ArgT>
		CComObjectEx(const ArgT &arg1) throw()
			: _BaseClass(arg1)
		{	_pAtlModule->Lock();	}

		virtual ~CComObjectEx() throw()
		{
			FinalRelease();
			_pAtlModule->Unlock();
		}

		STDMETHODIMP_(ULONG) AddRef() 
		{	return InternalAddRef();	}

		STDMETHODIMP_(ULONG) Release()
		{
			ULONG l = InternalRelease();
			if (l == 0)
				delete this;
			return l;
		}

		STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) throw()
		{	return _InternalQueryInterface(iid, ppvObject);	}

		//template <class Q>
		//HRESULT STDMETHODCALLTYPE QueryInterface(
		//	_Deref_out_ Q** pp) throw()
		//{
		//	return QueryInterface(__uuidof(Q), (void**)pp);
		//}

		template <typename ArgT>
		static HRESULT CreateInstance(CComObjectEx<Base>** pp, const ArgT &arg1) throw();
	};

	template <class Base>
	template <typename ArgT>
	inline HRESULT CComObjectEx<Base>::CreateInstance(CComObjectEx<Base>** pp, const ArgT &arg1) throw()
	{
		ATLASSERT(pp != NULL);
		if (pp == NULL)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		CComObjectEx<Base>* p = NULL;
		ATLTRY(p = new CComObjectEx<Base>(arg1))
		if (p != NULL)
		{
			p->SetVoid(NULL);
			p->InternalFinalConstructAddRef();
			hRes = p->_AtlInitialConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->FinalConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->_AtlFinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes != S_OK)
			{
				delete p;
				p = NULL;
			}
		}
		*pp = p;	
		return hRes;
	}
}
