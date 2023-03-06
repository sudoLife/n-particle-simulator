#if !defined(CONSTANTS_HPP)
#define CONSTANTS_HPP

namespace constants
{
    // tuned constants
    constexpr static double density = 0.0005;
    constexpr static double mass = 0.01;
    constexpr static double distanceThreshold = 0.01; // distance beyond which the forces don't interact
    constexpr static double minR = (distanceThreshold / 100.0);
    constexpr static double dt = 0.0005;
} // namespace constants

#endif // CONSTANTS_HPP
