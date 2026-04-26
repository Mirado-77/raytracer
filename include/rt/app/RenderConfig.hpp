/*
** EPITECH PROJECT, 2026
** local-Raytracer
** File description:
** RenderConfig
*/

#ifndef RENDERCONFIG_HPP_
    #define RENDERCONFIG_HPP_

    #include <string>

struct RenderConfig
{
    std::string sceneFile;
    std::string outputFile = "output.ppm";
    bool singleThread = false;
    bool noDisplay = false;
    bool usePathTracing = false;
    bool useDenoise = false;
    int samplesPerPixel = 100;
};

bool parseArgs(int ac, char *av[], RenderConfig &cfg);


#endif /* !RENDERCONFIG_HPP_ */
