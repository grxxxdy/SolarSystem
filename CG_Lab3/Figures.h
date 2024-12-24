#ifndef FIGURES_H
#define FIGURES_H

#include <vector>
#include <cmath>

#define PI 3.14159265358979323846

class Sphere
{
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Sphere(float radius, int sectorCount, int stackCount)
    {
        float x, y, z, xy;
        float nx, ny, nz;
        float s, t;

        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;

        // Vertices
        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle); // r * cos(u)
            z = radius * sinf(stackAngle);  // r * sin(u)

            // add (sectorCount+1) vertices per stack
            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep; // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // normal vector (nx, ny, nz)
                nx = x / radius;
                ny = y / radius;
                nz = z / radius;
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                vertices.push_back(s);
                vertices.push_back(t);
            }
        }

        // Indices
        int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1); // beginning of current stack
            k2 = k1 + sectorCount + 1; // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding the first and last stacks
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stackCount - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
    }
};

class Torus
{
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Torus(float outerRadius, float innerRadius, int numSides, int numRings)
    {
        float ringFactor = 2.0f * PI / numRings;
        float sideFactor = 2.0f * PI / numSides;
        float x, y, z;
        float nx, ny, nz;

        // Vertices
        for (int ring = 0; ring <= numRings; ++ring)
        {
            float u = ring * ringFactor;
            float cosU = cos(u);
            float sinU = sin(u);

            for (int side = 0; side <= numSides; ++side)
            {
                float v = side * sideFactor;
                float cosV = cos(v);
                float sinV = sin(v);

                x = (outerRadius + innerRadius * cosV) * cosU;
                y = (outerRadius + innerRadius * cosV) * sinU;
                z = innerRadius * sinV;

                nx = x / innerRadius;
                ny = y / innerRadius;
                nz = z / innerRadius;

                // Push back vertex position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Normals
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
            }
        }

        // Indices
        for (int ring = 0; ring < numRings; ++ring)
        {
            int ringStart = ring * (numSides + 1);
            int nextRingStart = (ring + 1) * (numSides + 1);

            for (int side = 0; side < numSides; ++side)
            {
                indices.push_back(ringStart + side);
                indices.push_back(nextRingStart + side);
                indices.push_back(ringStart + side + 1);

                indices.push_back(ringStart + side + 1);
                indices.push_back(nextRingStart + side);
                indices.push_back(nextRingStart + side + 1);
            }
        }
    }
};

#endif
