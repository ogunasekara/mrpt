/* +------------------------------------------------------------------------+
|                     Mobile Robot Programming Toolkit (MRPT)            |
|                          https://www.mrpt.org/                         |
|                                                                        |
| Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
| See: https://www.mrpt.org/Authors - All rights reserved.               |
| Released under BSD License. See: https://www.mrpt.org/License          |
+------------------------------------------------------------------------+ */

#include "math-precomp.h"  // Precompiled headers
//
#include <mrpt/math/CVectorDynamic.h>

#include <Eigen/Dense>

using namespace mrpt::math;

template <typename T>
template <typename T2>
CVectorDynamic<T2> CVectorDynamic<T>::cast() const
{
	CVectorDynamic<T2> r(size());
	r.asEigen() = asEigen().template cast<T2>();
	return r;
}

#define DO_VECDYN_INSTANTIATION(T_)                                            \
	template class mrpt::math::CVectorDynamic<T_>;

DO_VECDYN_INSTANTIATION(float)
DO_VECDYN_INSTANTIATION(double)
