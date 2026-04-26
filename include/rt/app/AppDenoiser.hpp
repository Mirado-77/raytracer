/*
** EPITECH PROJECT, 2026
** local-Raytracer
** File description:
** AppDenoiser
*/

#ifndef APPDENOISER_HPP_
    #define APPDENOISER_HPP_

    #include <memory>
    #include <functional>
    #include "rt/interfaces/IDenoiser.hpp"

using DenoiserPtr = std::unique_ptr<IDenoiser, std::function<void(IDenoiser*)>>;

DenoiserPtr makeNullDenoiser();
DenoiserPtr loadDenoiser();


#endif /* !APPDENOISER_HPP_ */
