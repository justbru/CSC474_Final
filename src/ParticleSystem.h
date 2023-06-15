#ifndef _PARTICLESYSTEM_H_
#define _PARTICLESYSTEM_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_PARTICLES 1000

using namespace glm;

class Program;

class ParticleSystem
{
private:
    // Might not need
    struct Particle
    {
        vec3 pos;
        float xVelocity, yVelocity;
    };

    std::vector<unsigned int> eleBuf;
    std::vector<float> posBuf;
    std::vector<float> texBuf;
    Particle particles[MAX_PARTICLES] = {Particle()};
    float particlePositions[MAX_PARTICLES * 3] = {-1000.0f};
    unsigned eleBufID;
    unsigned posBufID;
    unsigned norBufID;
    unsigned texBufID;
    unsigned vaoID;
    bool texOff;
    int totalParticles = MAX_PARTICLES;
    unsigned int lastUsedParticle;

public:
    ParticleSystem();
    virtual ~ParticleSystem();
    void createParticleSystem(vec3 emitterLocation, float splashRadius, int totalParticles);
    void init();
    void draw() const;
    void updateParticles();
    void initParticles();
    unsigned int FirstUnusedParticle();
};

#endif
