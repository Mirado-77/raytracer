/*
** EPITECH PROJECT, 2026
** local-Raytracer
** File description:
** InteractiveMode
*/

#ifndef INTERACTIVEMODE_HPP_
    #define INTERACTIVEMODE_HPP_

    #include <memory>
    #include <vector>
    #include "rt/scene/Scene.hpp"
    #include "rt/math/Color.hpp"
    #include "rt/scene/LibconfigLoader.hpp"
    #include "AppDenoiser.hpp"
    #include "RenderConfig.hpp"

int runInteractiveMode(const RenderConfig &cfg, std::unique_ptr<Scene> &scene,
    std::vector<Color> &pixels, DenoiserPtr &denoiser, LibconfigLoader &loader);

#endif /* !INTERACTIVEMODE_HPP_ */
