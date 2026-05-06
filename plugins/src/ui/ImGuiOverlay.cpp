/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** ImGuiOverlay
*/

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>

#include "rt/interfaces/IUIOverlay.hpp"
#include "rt/core/UIRegistry.hpp"

class ImGuiOverlay : public IUIOverlay {
public:

    // Disable ImGui cursor management at init
    void init(sf::RenderWindow &window) override
    {
        window_ = &window;
        if (!ImGui::SFML::Init(window))
            std::cerr << "ImGuiOverlay: ImGui::SFML::Init failed" << std::endl;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }

    /**
     * Feed the SFML event to imgui-sfml, then report whether ImGui wants to
     * consume it so that ProgressiveDisplay skips camera / hotkey handling.
     */
    bool processEvent(const sf::Event &event) override
    {
        ImGui::SFML::ProcessEvent(*window_, event);
        const ImGuiIO &io = ImGui::GetIO();

        switch (event.type) {
            case sf::Event::MouseButtonPressed:
            case sf::Event::MouseButtonReleased:
            case sf::Event::MouseMoved:
            case sf::Event::MouseWheelScrolled:
                return io.WantCaptureMouse;
            case sf::Event::KeyPressed:
            case sf::Event::KeyReleased:
            case sf::Event::TextEntered:
                return io.WantCaptureKeyboard;
            default:
                return false;
        }
    }

    void update() override
    {
        ImGui::SFML::Update(*window_, deltaClock_.restart());
        buildUI();
    }

    void render(sf::RenderWindow &window) override
    {
        ImGui::SFML::Render(window);
    }

    void shutdown() override
    {
        ImGui::SFML::Shutdown();
        window_ = nullptr;
    }

private:

    /** UI frames creation */
    void buildUI()
    {
        ImGui::SetNextWindowPos(ImVec2(10.f, 40.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300.f, 160.f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene Inspector");

        ImGui::TextDisabled("Raytracer Interactive UI");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::BulletText("Click on a primitive to select it.");
        ImGui::BulletText("Selected object properties will appear");
        ImGui::Spacing();
        ImGui::TextDisabled("(object picking coming next)");

        ImGui::End();
    }

    sf::RenderWindow *window_ = nullptr;
    sf::Clock deltaClock_;
};

extern "C" void rt_plugin_register()
{
    UIRegistry::instance().setOverlay(std::make_shared<ImGuiOverlay>());
}
