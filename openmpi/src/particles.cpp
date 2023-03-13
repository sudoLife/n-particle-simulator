#include "particles.hpp"
#include "constants.hpp"
#include <chrono>
#include <cmath>
#include <random>

void Particles::Generate(int n)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    particles.resize(n);
    size = std::sqrt(constants::density * n);

    int sx = (int)std::ceil(std::sqrt((double)n));
    int sy = (n + sx - 1) / sx;

    auto *shuffle = new int[n];

    for (int i = 0; i < n; i++)
        shuffle[i] = i;

    auto real_distribution = std::uniform_real_distribution<double>(0.0, 1.0);

    for (int i = 0; i < n; i++)
    {
        int j = std::uniform_int_distribution<int>(0, n - i)(generator);
        int k = shuffle[j];

        shuffle[j] = shuffle[n - i - 1];

        //
        //  distribute particles evenly to ensure proper spacing
        //
        particles[i].x = size * (1. + (k % sx)) / (1 + sx);
        particles[i].y = size * (1. + (k / sx)) / (1 + sy);

        //
        //  assign random velocities within a bound
        //
        particles[i].vx = real_distribution(generator) * 2 - 1;
        particles[i].vy = real_distribution(generator) * 2 - 1;

        // a new addition, because we will be iterating the grid this time
        particles[i].id = i;
    }

    delete[] shuffle;
}

void Particles::ApplyForce(Particle &particle, Particle &neighbor)
{
    auto dx = neighbor.x - particle.x;
    auto dy = neighbor.y - particle.y;

    auto r2 = dx * dx + dy * dy;

    if (r2 > constants::distanceThreshold * constants::distanceThreshold)
        return;

    r2 = std::max(r2, constants::minR * constants::minR);
    auto r = std::sqrt(r2);

    //
    //  very simple short-range repulsive force
    //
    double coef = (1 - constants::distanceThreshold / r) / r2 / constants::mass;
    particle.ax += coef * dx;
    particle.ay += coef * dy;
}

void Particles::Move(Particle &particle)
{
    //
    //  slightly simplified Velocity Verlet integration
    //  conserves energy better than explicit Euler method
    //
    particle.vx += particle.ax * constants::dt;
    particle.vy += particle.ay * constants::dt;
    particle.x += particle.vx * constants::dt;
    particle.y += particle.vy * constants::dt;

    //
    //  bounce from walls
    //
    // TODO: Another parallelization place?
    while (particle.x < 0 || particle.x > size)
    {
        particle.x = particle.x < 0 ? -particle.x : 2 * size - particle.x;
        particle.vx = -particle.vx;
    }
    while (particle.y < 0 || particle.y > size)
    {
        particle.y = particle.y < 0 ? -particle.y : 2 * size - particle.y;
        particle.vy = -particle.vy;
    }
}

void Particles::Save(fmt::ostream &dump_file)
{
    static bool first = true;

    if (first)
    {
        dump_file.print("{} {}\n", particles.size(), size);
        first = false;
    }

    for (const auto &particle : particles)
    {
        dump_file.print("{} {}\n", particle.x, particle.y);
    }
}
