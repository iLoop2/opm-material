// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  Copyright (C) 2008-2013 by Andreas Lauser

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
/*!
 * \file
 * \copydoc Opm::RegularizedBrooksCoreyParams
 */
#ifndef OPM_REGULARIZED_BROOKS_COREY_PARAMS_HPP
#define OPM_REGULARIZED_BROOKS_COREY_PARAMS_HPP

#include "BrooksCorey.hpp"
#include "BrooksCoreyParams.hpp"

#include <dune/common/deprecated.hh>

#include <cassert>

namespace Opm {
/*!
 * \ingroup FluidMatrixInteractions
 *
 * \brief   Parameters that are necessary for the \em regularization of
 *          the Brooks-Corey capillary pressure model.
 */
template <class TraitsT>
class RegularizedBrooksCoreyParams : public Opm::BrooksCoreyParams<TraitsT>
{
    typedef Opm::BrooksCoreyParams<TraitsT> BrooksCoreyParams;
    typedef Opm::BrooksCorey<TraitsT, RegularizedBrooksCoreyParams> BrooksCorey;
    typedef typename TraitsT::Scalar Scalar;

public:
    typedef TraitsT Traits;

    RegularizedBrooksCoreyParams()
        : BrooksCoreyParams()
        , pcnwLowSw_(0.01)
    {
#ifndef NDEBUG
        finalized_ = false;
#endif
    }

    RegularizedBrooksCoreyParams(Scalar entryPressure, Scalar lambda)
        : BrooksCoreyParams(entryPressure, lambda)
        , pcnwLowSw_(0.01)
    { finalize(); }

    /*!
     * \brief Calculate all dependent quantities once the independent
     *        quantities of the parameter object have been set.
     */
    void finalize()
    {
        BrooksCoreyParams::finalize();

        pcnwLow_ = BrooksCorey::twoPhaseSatPcnw(*this, pcnwLowSw_);
        pcnwSlopeLow_ = dPcnw_dSw_(pcnwLowSw_);
        pcnwHigh_ = BrooksCorey::twoPhaseSatPcnw(*this, 1.0);
        pcnwSlopeHigh_ = dPcnw_dSw_(1.0);

#ifndef NDEBUG
        finalized_ = true;
#endif
    }

    /*!
     * \brief Return the threshold saturation below which the
     *        capillary pressure is regularized.
     */
    Scalar pcnwLowSw() const
    { assertFinalized_(); return pcnwLowSw_; }

    /*!
     * \brief Return the capillary pressure at the low threshold
     *        saturation of the wetting phase.
     */
    Scalar pcnwLow() const
    { assertFinalized_(); return pcnwLow_; }

    /*!
     * \brief Return the slope capillary pressure curve if Sw is
     *        smaller or equal to the low threshold saturation.
     *
     * For this case, we extrapolate the curve using a straight line.
     */
    Scalar pcnwSlopeLow() const
    { assertFinalized_(); return pcnwSlopeLow_; }

    /*!
     * \brief Set the threshold saturation below which the capillary
     *        pressure is regularized.
     */
    void setPcLowSw(Scalar value)
    { pcnwLowSw_ = value; }

    void setThresholdSw(Scalar value) DUNE_DEPRECATED_MSG("this method has been renamed to setPcLowSw()")
    { pcnwLowSw_ = value; }

    /*!
     * \brief Return the capillary pressure at the high threshold
     *        saturation of the wetting phase.
     */
    Scalar pcnwHigh() const
    { assertFinalized_(); return pcnwHigh_; }

    /*!
     * \brief Return the slope capillary pressure curve if Sw is
     *        larger or equal to 1.
     *
     * For this case, we extrapolate the curve using a straight line.
     */
    Scalar pcnwSlopeHigh() const
    { assertFinalized_(); return pcnwSlopeHigh_; }

private:
#ifndef NDEBUG
    void assertFinalized_() const
    { assert(finalized_); }

    bool finalized_;
#else
    void assertFinalized_() const
    { }
#endif

    Scalar dPcnw_dSw_(Scalar Sw) const
    {
        // use finite differences to calculate the derivative w.r.t. Sw of the
        // unregularized curve's capillary pressure.
        const Scalar eps = 1e-7;
        Scalar delta = 0.0;
        Scalar pc1;
        Scalar pc2;
        if (Sw + eps < 1.0) {
            pc2 = BrooksCorey::twoPhaseSatPcnw(*this, Sw + eps);
            delta += eps;
        }
        else
            pc2 = BrooksCorey::twoPhaseSatPcnw(*this, Sw);

        if (Sw - eps > 0.0) {
            pc1 = BrooksCorey::twoPhaseSatPcnw(*this, Sw - eps);
            delta += eps;
        }
        else
            pc1 = BrooksCorey::twoPhaseSatPcnw(*this, Sw);

        return (pc2 - pc1)/delta;
    }

    Scalar pcnwLowSw_;
    Scalar pcnwLow_;
    Scalar pcnwSlopeLow_;
    Scalar pcnwHigh_;
    Scalar pcnwSlopeHigh_;
};
} // namespace Opm

#endif
