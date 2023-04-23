#pragma once

namespace CL
{ 
	template<typename T>
	void QSort(T* l, T* r)
	{
		if (r <= l)
		{
			return;
		}

		T centr = *(l + ((r - l) >> 1));
		T* pL = l;
		T* pR = r;

		while (pL <= pR)
		{
			while (*pL < centr)
			{
				++pL;
			}

			while (*pR > centr)
			{
				--pR;
			}

			if (pL <= pR)
			{
				CL_SWAP(*pL, *pR);
				++pL;
				--pR;
			}
		}

		if (l < pR)
		{
			QSort<T>(l, pR);
		}

		if (pL < r)
		{
			QSort<T>(pL, r);
		}
	}

	template<typename T>
	void QSort(T* l, T* r, int(*comporator)(const T*, const T*))
	{
		if (r <= l)
		{
			return;
		}

		T* pCentr = (l + ((r - l) >> 1));
		T* pL = l;
		T* pR = r;

		while (pL <= pR)
		{
			while (comporator(pL, pCentr) < 0)
			{
				++pL;
			}

			while (comporator(pR, pCentr) > 0)
			{
				--pR;
			}

			if (pL <= pR)
			{
				CL_SWAP(*pL, *pR);

				if (pL == pCentr)
				{
					pCentr = pR;
				}
				else if (pR == pCentr)
				{
					pCentr = pL;
				}

				++pL;
				--pR;
			}
		}

		if (l < pR)
		{
			QSort<T>(l, pR, comporator);
		}

		if (pL < r)
		{
			QSort<T>(pL, r, comporator);
		}
	}

	template<typename T>
	void Sort(T* l, T* r)
	{
		auto Delta = (r - l);

		if (Delta < 30)
		{
			Delta++;
			int i, key, j;
			for (i = 1; i < Delta; i++)
			{
				key = l[i];
				j = i - 1;

				while (j >= 0 && l[j] > key)
				{
					l[j + 1] = l[j];
					j--;
				}

				l[j + 1] = key;
			}
		}
		else
		{
			T centr = *(l + (Delta >> 1));
			T* pL = l;
			T* pR = r;

			while (pL <= pR)
			{
				while (*pL < centr)
				{
					++pL;
				}

				while (*pR > centr)
				{
					--pR;
				}

				if (pL <= pR)
				{
					CL_SWAP(*pL, *pR);
					++pL;
					--pR;
				}
			}

			if (l < pR)
			{
				Sort<T>(l, pR);
			}

			if (pL < r)
			{
				Sort<T>(pL, r);
			}
		}
	}
}