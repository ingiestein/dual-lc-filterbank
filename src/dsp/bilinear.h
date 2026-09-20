#pragma once

#include <array>
#include <algorithm>
#include <cmath>

namespace duallc {

// Bilinear transform of a rational analog transfer function of order 2 or 4.
// s = c * (1 - z^-1) / (1 + z^-1), with analog coefficients b_s/a_s of degree `order`.
// Output b_z/a_z are normalized so a_z[0] == 1.
inline void bilinearTransform (const double* bS, const double* aS, int order, double c,
                               double* bZ, double* aZ) noexcept
{
    const int n = order + 1;
    std::array<double, 5> bz {};
    std::array<double, 5> az {};

    if (order == 2)
    {
        // (1-z)^2 = 1 - 2z + z^2
        // (1-z)(1+z) = 1 - z^2
        // (1+z)^2 = 1 + 2z + z^2
        const double c2 = c * c;
        const double bu2[] = {1.0, -2.0, 1.0};
        const double buv[] = {1.0, 0.0, -1.0};
        const double bv2[] = {1.0, 2.0, 1.0};

        for (int k = 0; k < 3; ++k)
        {
            bz[k] = bS[2] * c2 * bu2[k] + bS[1] * c * buv[k] + bS[0] * bv2[k];
            az[k] = aS[2] * c2 * bu2[k] + aS[1] * c * buv[k] + aS[0] * bv2[k];
        }
    }
    else
    {
        // Degree 4 binomial products of u=(1-z), v=(1+z).
        const double c2 = c * c;
        const double c3 = c2 * c;
        const double c4 = c2 * c2;
        const double u4[]  = {1.0, -4.0, 6.0, -4.0, 1.0};
        const double u3v[] = {1.0, -2.0, 0.0,  2.0, -1.0};
        const double u2v2[]= {1.0,  0.0,-2.0,  0.0, 1.0};
        const double uv3[] = {1.0,  2.0, 0.0, -2.0, -1.0};
        const double v4[]  = {1.0,  4.0, 6.0,  4.0, 1.0};

        for (int k = 0; k < 5; ++k)
        {
            bz[k] = bS[4] * c4 * u4[k] + bS[3] * c3 * u3v[k] + bS[2] * c2 * u2v2[k]
                  + bS[1] * c * uv3[k] + bS[0] * v4[k];
            az[k] = aS[4] * c4 * u4[k] + aS[3] * c3 * u3v[k] + aS[2] * c2 * u2v2[k]
                  + aS[1] * c * uv3[k] + aS[0] * v4[k];
        }
    }

    const double a0 = az[0];
    const double inv = (std::abs (a0) > 1.0e-30) ? 1.0 / a0 : 1.0;
    for (int k = 0; k < n; ++k)
    {
        bZ[k] = bz[k] * inv;
        aZ[k] = az[k] * inv;
    }
    for (int k = n; k < 5; ++k)
    {
        bZ[k] = 0.0;
        aZ[k] = 0.0;
    }
}

inline double prewarpedOmega (double fc, double fs) noexcept
{
    const double nyquist = 0.49 * fs;
    const double clipped = std::min (std::max (fc, 1.0), nyquist);
    return 2.0 * fs * std::tan (3.14159265358979323846 * clipped / fs);
}

} // namespace duallc
