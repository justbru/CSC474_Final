#include "ParticleSystem.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

ParticleSystem::ParticleSystem() : posBufID(0),
                                   vaoID(0)
{
}

ParticleSystem::~ParticleSystem()
{
}

/* copy the data from the shape to this object */
void ParticleSystem::createParticleSystem(vec3 emitterLocation, float splashRadius, int totalParts)
{
    totalParticles = totalParts;
    int particleCount;
    for (particleCount = 0; particleCount < totalParticles; particleCount++)
    {
        particles[particleCount].pos = emitterLocation;

        float xVel = (((double)rand() / (RAND_MAX)) * splashRadius) - (splashRadius / 2.f); // value between -0.15 and 0.15
        float yVel = ((double)rand() / (RAND_MAX)) * splashRadius / 1.2f;                   // sets initial y velocity between 0 and 0.15
        particles[particleCount].xVelocity = xVel;
        particles[particleCount].yVelocity = yVel;

        float x = particles[particleCount].pos.x;
        float y = particles[particleCount].pos.y;
        float z = particles[particleCount].pos.z;

        particlePositions[particleCount * 3 + 0] = x;
        particlePositions[particleCount * 3 + 1] = y;
        particlePositions[particleCount * 3 + 2] = z;
    }
}

void ParticleSystem::init()
{
    // Initialize the vertex array object
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    // Send the position array to the GPU
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particlePositions), particlePositions, GL_STATIC_DRAW);

    // Unbind the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// always untextured for intro labs until texture mapping
void ParticleSystem::draw() const
{
    glBindVertexArray(vaoID);
    GLSL::enableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particlePositions), particlePositions, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // glPointSize(2.0f);
    // glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, totalParticles);
    // glDisableClientState(GL_VERTEX_ARRAY);
}

void ParticleSystem::initParticles()
{
}

void ParticleSystem::updateParticles()
{
    int particleCount;
    for (particleCount = 0; particleCount < totalParticles; particleCount++)
    {
        float x = particles[particleCount].pos.x;
        float y = particles[particleCount].pos.y;
        float z = particles[particleCount].pos.z;

        float xVel = particles[particleCount].xVelocity;
        float yVel = particles[particleCount].yVelocity;
        particles[particleCount].pos = vec3(x + xVel, y + yVel, z);
        particles[particleCount].yVelocity = particles[particleCount].yVelocity - 0.001f;
        if (particles[particleCount].xVelocity > 0)
            particles[particleCount].xVelocity = particles[particleCount].xVelocity - 0.0005f;

        if (particles[particleCount].xVelocity < 0)
            particles[particleCount].xVelocity = particles[particleCount].xVelocity + 0.0005f;

        particlePositions[particleCount * 3 + 0] = x + xVel;
        particlePositions[particleCount * 3 + 1] = y + yVel;
        particlePositions[particleCount * 3 + 2] = z;
    }
}
