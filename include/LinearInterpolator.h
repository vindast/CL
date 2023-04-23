#pragma once 
#include <vector>

namespace CL
{
	template<typename agrType, typename valueType> struct LinearInterpolatorData
	{
		agrType argument;
		valueType value;
	};

	template<typename agrType, typename valueType> class LinearInterpolator
	{
	public:
		void add(valueType value, agrType argument)
		{
			LinearInterpolatorData<agrType, valueType> x;

			x.argument = argument;
			x.value = value;

			data.push_back(x);
		}

		void clear()
		{
			data.clear();
		}

		valueType getValue(agrType argument) const
		{
			valueType result(0);

			if (argument >= data[data.size() - 1].argument)
			{
				result = data[data.size() - 1].value;
			}
			else if (argument <= data[0].argument)
			{
				result = data[0].value;
			}
			else
			{
				size_t a = 0;
				size_t b = data.size() - 1;

				while (b - a > 1)
				{
					size_t centr = a + (b - a) / 2;

					if (data[centr].argument > argument)
					{
						b = centr;
					}
					else
					{
						a = centr;
					}
				}

				agrType arg = (argument - data[a].argument) / (data[b].argument - data[a].argument);

				result = data[a].value * (1.0f - arg) + data[b].value * arg;
			}

			return result;
		}

	private:
		std::vector<LinearInterpolatorData<agrType, valueType> > data;

	};
};