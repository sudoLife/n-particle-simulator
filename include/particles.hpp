#if !defined(PARTICLES_HPP)
#define PARTICLES_HPP

#include "particle.hpp"
#include <fmt/os.h>
#include <fstream>
#include <vector>

class Particles
{
public:
public:
    explicit Particles(int n);

    // TODO: is this where we can optimize it? Because it seems like a waste
    // to not update the neighbor as well
    void ApplyForce(Particle &particle, Particle &neighbor);
    void Move(Particle &particle);
    void Save(fmt::ostream &dump_file);

    std::vector<Particle>::iterator begin() { return particles.begin(); }
    std::vector<Particle>::iterator end() { return particles.end(); }

public:
    std::vector<Particle> particles;
    double size;

private:
private:
};

#endif // PARTICLES_HPP
