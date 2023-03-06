#if !defined(CONSTANTS_HPP)
#define CONSTANTS_HPP

namespace constants
{
    // tuned constants
    constexpr double density = 0.0005;
    constexpr double mass = 0.01;
    constexpr double distanceThreshold = 0.01; // distance beyond which the forces don't interact
    constexpr double minR = (distanceThreshold / 100.0);
    constexpr double dt = 0.0005;
} // namespace constants

#endif // CONSTANTS_HPP
