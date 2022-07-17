#include "FractalRemake.h"

// The code for combining everything together into an interacive application

int main(int argc, char* argv[]) {

    if (argc > 1) {

        handleConsoleParams(argc, argv);

        return 0;

    }

    sf::RenderWindow window(sf::VideoMode(1280, 960), "Fractal Remake");
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    sf::CircleShape shape(window.getSize().y / 2);
    shape.setFillColor(sf::Color::Green);

    FractalImage mainFractalImage(window.getSize().x, window.getSize().y);
    mainFractalImage.generatePalette();
    FractalImage renderingFractalImage = FractalImage();

    sf::Texture renderTexture;
    renderTexture.create(mainFractalImage.width, mainFractalImage.height);
    sf::Sprite renderSprite(renderTexture);

    sf::Texture fractalTexture;
    fractalTexture.create(mainFractalImage.width, mainFractalImage.height);
    sf::Sprite fractalSprite(fractalTexture);
    fractalSprite.setOrigin((sf::Vector2f)window.getSize() / 2.0f);

    sf::Texture windowTexture;
    windowTexture.create(window.getSize().x, window.getSize().y);
    int middleSpriteSize = 50;
    sf::Sprite middleSprite(windowTexture, sf::IntRect(sf::Vector2i(windowTexture.getSize().x / 2 - middleSpriteSize / 2, windowTexture.getSize().y / 2 - middleSpriteSize / 2), sf::Vector2i(middleSpriteSize, middleSpriteSize)));
    middleSprite.setScale(sf::Vector2f(1, -1));

    sf::Mouse mouse;
    sf::Keyboard keyboard;
    bool isAnyKeyPressed = false;
    int SampleDistanceOrMultisampling = -1;

    int currentPaletteIndex = 0;
    int paletteComboIndex = 0;
    bool displayPaletteEditor = false;
    char* paletteNameBuffer = FractalImage::currentPalette.name.data();

    float colorOffsetFloat = 0.0f;
    int temp = 1;

    int renderWidth = 1920;
    int renderHeight = 1080;
    int renderms = 1;
    int keyFrameCounter = 1;
    double startingScale;


    sf::Clock deltaClock;

    std::future<void> drawThread;
    std::future_status drawThreadStatus = std::future_status::ready;

    std::vector<const char*> algorithmComboItems;
    for (std::map<const char*, fractalAlgorithmFunction>::iterator it = fa::fractalAlgorithms.begin(); it != fa::fractalAlgorithms.end(); it++)
    {
        algorithmComboItems.push_back(it->first);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                mainFractalImage.cancelUpdate();
                mainFractalImage.savePaletteList();
                window.close();
            } else if (event.type == sf::Event::Resized) {
                mainFractalImage.cancelUpdate();
                sf::FloatRect visibleArea(sf::Vector2f(0, 0), sf::Vector2f(event.size.width, event.size.height));
                window.setView(sf::View(visibleArea));
                mainFractalImage.width = event.size.width;
                mainFractalImage.height = event.size.height; 
                mainFractalImage.scaledHeight = mainFractalImage.height / std::max(mainFractalImage.sampleDistance, 1);
                mainFractalImage.scaledWidth = mainFractalImage.width / std::max(mainFractalImage.sampleDistance, 1);
                mainFractalImage.pixelArray.resize(mainFractalImage.scaledWidth * mainFractalImage.scaledHeight * 4);
                mainFractalImage.renderingStatus = NeedUpdate;
                fractalTexture.create(mainFractalImage.scaledWidth, mainFractalImage.scaledHeight);
                fractalSprite.setTexture(fractalTexture, true);
                fractalSprite.setOrigin((sf::Vector2f)fractalSprite.getTextureRect().getSize() / 2.0f);
                renderTexture.create(event.size.width, event.size.height);
                renderSprite.setTexture(renderTexture, true);
                windowTexture.create(window.getSize().x, window.getSize().y);
                middleSprite.setTexture(windowTexture);
                middleSprite.setTextureRect(sf::IntRect(sf::Vector2i(windowTexture.getSize().x / 2 - middleSpriteSize / 2, windowTexture.getSize().y / 2 - middleSpriteSize / 2), sf::Vector2i(middleSpriteSize, middleSpriteSize)));
            } else if (event.type == sf::Event::KeyPressed) {
                isAnyKeyPressed = true;
            } else if (event.type == sf::Event::KeyReleased) {
                isAnyKeyPressed = false || keyboard.isKeyPressed(keyboard.LShift);
            }
        }

        sf::Time t = deltaClock.getElapsedTime();
        ImGui::SFML::Update(window, deltaClock.restart());

#pragma region ImGui_Windows

#pragma region Main_settings_window

        ImGui::Begin("Main", 0, 0 | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("MainTabBar", 0)) {

            // ALGORITHM TAB
            if (ImGui::BeginTabItem("Algorithm")) {

                if (ImGui::InputInt("Iterations", &mainFractalImage.iterationMax, pow(10, floor(log10(mainFractalImage.iterationMax)) - 1), 0, 0 | ImGuiInputTextFlags_CharsDecimal)) {
                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                static int algorithmComboIndex = 0;
                if (ImGui::Combo("Algorithm##algorithmcombo", &algorithmComboIndex, algorithmComboItems.data(), algorithmComboItems.size())) {
                    mainFractalImage.currentAlgorithm = fa::fractalAlgorithms[algorithmComboItems[algorithmComboIndex]];

                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                if (ImGui::Button("Refresh")) {
                    mainFractalImage.renderingStatus = NeedUpdate;
                    fractalSprite.setOrigin((sf::Vector2f)fractalSprite.getTextureRect().getSize() / 2.0f);
                }

                if (ImGui::CollapsingHeader("Iteration flags")) {
                    static bool disableTests = false;
                    static bool disableSmooth = false;
                    static bool disableShadow = false;

                    int flags = 0;
                    if (disableTests) flags |= fa::DisableTests;
                    if (disableSmooth) flags |= fa::DisableSmoothing;
                    if (disableShadow) flags |= fa::DisableShadowCalculation;

                    if(ImGui::Checkbox("Disable tests", &disableTests)) mainFractalImage.renderingStatus = NeedUpdate;
                    ImGui::SameLine(); HelpMarker("Disables some checks that test if a given point is inside the set.\nFaster when left unchecked in most cases except when there are very few points belonging to the set visible");
                    if(ImGui::Checkbox("Disable smoothing", &disableSmooth)) mainFractalImage.renderingStatus = NeedUpdate;
                    if(ImGui::Checkbox("Disable shadow value calculation", &disableShadow)) mainFractalImage.renderingStatus = NeedUpdate;

                    mainFractalImage.flags = flags;
                }


                ImGui::EndTabItem();

            }

            // RENDERING TAB
            if (ImGui::BeginTabItem("Rendering")) {


                bool anyOfTheseWasChanged = false;
                if (ImGui::RadioButton("Sample Distance", &SampleDistanceOrMultisampling, 1)) anyOfTheseWasChanged = true;
                ImGui::SameLine();
                if (ImGui::RadioButton("Multisampling", &SampleDistanceOrMultisampling, -1)) anyOfTheseWasChanged = true;
                if (ImGui::InputInt("", &temp, 1, 1, 0 | ImGuiInputTextFlags_CharsDecimal)) anyOfTheseWasChanged = true;

                if (anyOfTheseWasChanged) {
                    mainFractalImage.cancelUpdate();
                    if (temp == 0) temp = 1;
                    if (temp * SampleDistanceOrMultisampling < -4) {
                        temp = 4;
                    }
                    mainFractalImage.sampleDistance = temp * SampleDistanceOrMultisampling;
                    mainFractalImage.renderingStatus = NeedUpdate;
                    fractalTexture.create(mainFractalImage.width / std::max(mainFractalImage.sampleDistance, 1), mainFractalImage.height / std::max(mainFractalImage.sampleDistance, 1));
                    fractalSprite.setTexture(fractalTexture, true);
                    fractalSprite.setOrigin((sf::Vector2f)fractalSprite.getTextureRect().getSize() / 2.0f);
                }

                if (ImGui::Button("Refresh")) {
                    mainFractalImage.renderingStatus = NeedUpdate;
                    fractalSprite.setOrigin((sf::Vector2f)fractalSprite.getTextureRect().getSize() / 2.0f);
                }

                if (ImGui::Button("Quick render")) {
                    mainFractalImage.saveToImage("images/");
                    mainFractalImage.renderingStatus = Ready;
                }
                ImGui::SameLine();
                if (ImGui::Button("Advanced")) {
                    renderingFractalImage = FractalImage(mainFractalImage);
                    keyFrameCounter = 1;
                    startingScale = renderingFractalImage.scale;
                    ImGui::OpenPopup("Render");
                }

                ImGui::SameLine(); 
                HelpMarker("\"Quick render\" Saves the already rendered image on screen to a file.\n\"Advanced\" allows you to specify more image parameters.");

                // Always center this window when appearing
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal("Render", 0, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if (renderingFractalImage.renderingStatus != Ready && renderingFractalImage.renderingStatus != Empty) {
                        ImGui::BeginDisabled(true);
                    }

                    ImGui::BeginGroup();
                    ImGui::Text("Image resolution");

                    static int resolutionComboIndex = 0;
                    if (ImGui::Combo("Presets", &resolutionComboIndex, "480p\000720p\0001080p\0001440p\0004k\0008k\000\000")) {
                        switch (resolutionComboIndex)
                        {
                        case 0:
                            renderingFractalImage.width = 640;
                            renderingFractalImage.height = 480;
                            break;
                        case 1:
                            renderingFractalImage.width = 1280;
                            renderingFractalImage.height = 720;
                            break;
                        case 2:
                            renderingFractalImage.width = 1920;
                            renderingFractalImage.height = 1080;
                            break;
                        case 3:
                            renderingFractalImage.width = 2560;
                            renderingFractalImage.height = 1440;
                            break;
                        case 4:
                            renderingFractalImage.width = 3840;
                            renderingFractalImage.height = 2160;
                            break;
                        case 5:
                            renderingFractalImage.width = 7680;
                            renderingFractalImage.height = 4320;
                            break;
                        default:
                            break;
                        }
                    }

                    ImGui::DragInt("##renderwidth", &renderingFractalImage.width, 1, 0, 38400);
                    ImGui::SameLine();
                    ImGui::Text(" x ");
                    ImGui::SameLine();
                    ImGui::DragInt("##renderheight", &renderingFractalImage.height, 1, 0, 21600);
                    ImGui::EndGroup();

                    ImGui::DragInt("Multisampling", &renderms, 0.1f, 1, 4);

                    static int renderType = 0;
                    ImGui::RadioButton("Image", &renderType, 0);
                    ImGui::SameLine();
                    ImGui::RadioButton("Video", &renderType, 1);

                    if (renderType != 0) {
                        // Different video settings here
                    }


                    int keyFrameAmount = floor(log(startingScale / 3.0) / log(0.5));

                    if (ImGui::Button("Render")) {
                        renderingFractalImage.sampleDistance = -renderms;

                        if (renderType != 0) {

                            std::thread t([&renderingFractalImage, &keyFrameCounter] {
                                renderingFractalImage.generateZoomVideo(keyFrameCounter);
                                });
                            t.detach();

                        }
                        else {
                            std::thread t([&renderingFractalImage] {
                                renderingFractalImage.saveToImage("images/");
                                });
                            t.detach();
                        }
                    }


                    if (renderingFractalImage.renderingStatus != Ready && renderingFractalImage.renderingStatus != Empty) {
                        ImGui::EndDisabled();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        renderingFractalImage.cancelUpdate();
                        renderingFractalImage.pixelArray.clear();
                        renderingFractalImage.dataArray.clear();
                        keyFrameCounter = 1;
                        ImGui::CloseCurrentPopup();
                    }

                    if (renderingFractalImage.renderingStatus == Rendering) {
                        if (renderType != 0) {
                            char buf[32];
                            sprintf_s(buf, "keyframe %d/%d: %.0f%%", keyFrameCounter, keyFrameAmount, renderingFractalImage.renderProgress * 100 + 0.01f);
                            ImGui::ProgressBar(renderingFractalImage.renderProgress, ImVec2(-FLT_MIN, 0), buf);
                        }
                        else {
                            ImGui::ProgressBar(renderingFractalImage.renderProgress);
                        }

                    }
                    else if (renderingFractalImage.renderingStatus == Ready && renderType == 0) {
                        ImGui::Text("Saving image to file...");
                    }
                    else if ((  renderingFractalImage.renderingStatus == ImageSaved && renderType == 0) || 
                                renderingFractalImage.renderingStatus == VideoSaved) {
                        renderingFractalImage.pixelArray.clear();
                        renderingFractalImage.dataArray.clear();
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::InputInt("Threads", &mainFractalImage.threadAmount, 1, 1, 0 | ImGuiInputTextFlags_CharsDecimal)) {
                    mainFractalImage.threadAmount = std::clamp(mainFractalImage.threadAmount, 1, (int)std::thread::hardware_concurrency());
                }


                ImGui::EndTabItem();
            }

            // VISUALS TAB
            if (ImGui::BeginTabItem("Visuals")) {

                if (ImGui::Checkbox("##shadowcheckbox", &FractalImage::shadowFx)) {
                    mainFractalImage.refreshVisuals();
                }

                ImGui::SameLine();

                if (ImGui::DragFloat("Shadow", &FractalImage::shadowStrength, 0.01f, 0.0f, 1.0f, "%.2f", 0 | ImGuiSliderFlags_AlwaysClamp) && mainFractalImage.shadowFx) {
                    mainFractalImage.refreshVisuals();
                }

                if (ImGui::DragInt("Shadow angle", &fa::shadowAngle, 1.0f, 0, 360, "%d", 0 | ImGuiSliderFlags_AlwaysClamp) && mainFractalImage.shadowFx) {
                    fa::updateShadowVars();
                    mainFractalImage.renderingStatus = NeedUpdate;

                } ImGui::SameLine(); HelpMarker("Warning! Requires re-rendering");

                if (ImGui::DragFloat("Shadow height", &fa::h2, 0.01f, 0.0f, 10.0f, "%.2f", 0 | ImGuiSliderFlags_AlwaysClamp) && mainFractalImage.shadowFx) {
                    mainFractalImage.renderingStatus = NeedUpdate;

                } ImGui::SameLine(); HelpMarker("Warning! Requires re-rendering");

                if (ImGui::Button("<<")) {
                    mainFractalImage.iterDiv >>= 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine(); 
                if (ImGui::Button("-1")) {
                    mainFractalImage.iterDiv -= 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                if (ImGui::DragInt("##iterdivslider", &mainFractalImage.iterDiv)) {
                    mainFractalImage.refreshVisuals();
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button("+1")) {
                    mainFractalImage.iterDiv += 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                if (ImGui::Button(">>")) {
                    mainFractalImage.iterDiv <<= 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                ImGui::Text("Iter div");


                const char* combo_preview_value = FractalImage::paletteList[paletteComboIndex].name.c_str();
                ImGui::PushItemWidth(100);
                if (ImGui::BeginCombo("##palettepickercombo", combo_preview_value, 0))
                {
                    for (int n = 0; n < FractalImage::paletteList.size(); n++)
                    {
                        const bool is_selected = (paletteComboIndex == n);
                        if (ImGui::Selectable(FractalImage::paletteList[n].name.c_str(), is_selected)) {
                            paletteComboIndex = n;

                            FractalImage::currentPalette = FractalImage::paletteList[paletteComboIndex];
                            currentPaletteIndex = paletteComboIndex;

                            mainFractalImage.generatePalette();

                            mainFractalImage.refreshVisuals();
                        }


                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();
                ImGui::SameLine();

                if (ImGui::Button("Edit##editpalette")) {
                    displayPaletteEditor = !displayPaletteEditor;
                }

                ImGui::SameLine(); ImGui::Text("Color palette");

                if (ImGui::DragFloat("Color offset", &colorOffsetFloat, 0.005f, 0.0f, 1.0f, "%.3f", 0 | ImGuiSliderFlags_AlwaysClamp)) {

                    mainFractalImage.ColorOffset = mainFractalImage.ColorAmount * colorOffsetFloat;

                    mainFractalImage.refreshVisuals();
                }

                ImGui::EndTabItem();
            }

            // MISC TAB
            if (ImGui::BeginTabItem("Misc")) {

                if (ImGui::Button("Save", ImVec2(80, 25))) {

                    mainFractalImage.savePaletteList();

                    std::ofstream ofs("save.txt");

                    ofs << mainFractalImage << '\n';
                    ofs << currentPaletteIndex;

                    ofs.close();
                }

                ImGui::SameLine(0.0f, 10.0f);

                if (ImGui::Button("Load", ImVec2(80, 25))) {

                    mainFractalImage.loadPaletteList();

                    std::ifstream ifs("save.txt");

                    ifs >> mainFractalImage;
                    ifs >> currentPaletteIndex;

                    ifs.close();

                    FractalImage::currentPalette = FractalImage::paletteList[currentPaletteIndex];

                    mainFractalImage.generatePalette();

                    mainFractalImage.renderingStatus = NeedUpdate;

                }

                if (ImGui::Button("Generate dataset")) {
                    generateDataset(1000000, 1000000);
                }

                if (ImGui::Button("Print dataset")) {
                    printDataset(100);
                }

                if (ImGui::Button("Benchmark")) {
                    BenchmarkMandelbrot();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

#pragma endregion 

#pragma region Debug_info_window

        ImGui::Begin("Debug info", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text((std::to_string(1000000 / t.asMicroseconds()) + " FPS").c_str());
        ImGui::Text(("Last render took " + std::to_string(mainFractalImage.lastRenderTime) + " ms").c_str());
        ImGui::Text(mainFractalImage.debugInfo().c_str());

        static bool showPixelInfoWindow = false;

        ImGui::Checkbox("Show pixel info window", &showPixelInfoWindow);
        if (ImGui::Checkbox("Show debug colors", &mainFractalImage.debugColors)) mainFractalImage.refreshVisuals();

        ImGui::End();

#pragma endregion 

#pragma region Aiming_Window

        ImGui::Begin("Aiming", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Image(middleSprite, sf::Vector2f(200, 200));

        ImGui::End();

#pragma endregion 

#pragma region Pixel_information_window

        if (showPixelInfoWindow) {

            sf::Vector2i pos = mouse.getPosition() - window.getPosition() - sf::Vector2i(8, 31);

            int dataArrayIndex;

            if (mainFractalImage.sampleDistance < -1) {
                dataArrayIndex = (pos.x + pos.y * mainFractalImage.width) * mainFractalImage.sampleDistance * mainFractalImage.sampleDistance;
            }
            else {
                dataArrayIndex = ((float)pos.x / mainFractalImage.width) * mainFractalImage.scaledWidth;
                dataArrayIndex += (int)(((float)pos.y / mainFractalImage.height) * mainFractalImage.scaledHeight) * mainFractalImage.scaledWidth;
            }

            dataArrayIndex = std::clamp(dataArrayIndex, 0, (int)mainFractalImage.dataArray.size() - 1);

            fractalData fd = mainFractalImage.dataArray[dataArrayIndex];

            std::string testString;

            switch (fd.iterResult)
            {
            case 0:
                testString = "maxiter";
                break;
            case 1:
                testString = "circletest";
                break;
            case 2:
                testString = "looptest";
                break;
            case 3:
                testString = "dertest";
                break;
            case 4:
                testString = "undef";
                break;
            case 5:
                testString = "notinside";
                break;
            default:
                testString = "error";
                break;
            }

            sf::Color pixelColor = mainFractalImage.mapFractalData(fd);

            ImGui::BeginTooltip();
            sfColorEdit3("##pixelcolor", &pixelColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            ImGui::TextUnformatted((
                "Iterations: " + std::to_string(fd.iterations) + '\n' +
                std::to_string(fd.iterResult) + " (" + testString + ")" + '\n' +
                "Smooth value: " + std::to_string(fd.smoothValue) + '\n' +
                "Shadow value: " + std::to_string(fd.shadowValue) + '\n').c_str());
            ImGui::EndTooltip();

            ImGui::End();
        }

#pragma endregion

#pragma region Palette_Editor_Window

        if (displayPaletteEditor) {
            ImGui::Begin("PaletteEditor", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::InputText("##palettenameeditor", paletteNameBuffer, ImGuiInputTextFlags_AutoSelectAll)) {
                FractalImage::currentPalette.name = paletteNameBuffer;
            }

            ImGui::SameLine();

            if (ImGui::BeginCombo("##palettepickercombo", "", ImGuiComboFlags_NoPreview))
            {
                for (int n = 0; n < FractalImage::paletteList.size(); n++)
                {
                    const bool is_selected = (paletteComboIndex == n);
                    if (ImGui::Selectable(FractalImage::paletteList[n].name.c_str(), is_selected)) {
                        paletteComboIndex = n;

                        FractalImage::currentPalette = FractalImage::paletteList[paletteComboIndex];
                        currentPaletteIndex = paletteComboIndex;

                        mainFractalImage.generatePalette();

                        mainFractalImage.refreshVisuals();

                        paletteNameBuffer = FractalImage::currentPalette.name.data();
                    }


                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImVec2 btn_size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
            if (ImGui::Button("-", btn_size) && FractalImage::currentPalette.paletteColors.size() > 1) {
                FractalImage::currentPalette.paletteColors.erase(FractalImage::currentPalette.paletteColors.end() - 1);
                mainFractalImage.generatePalette();
                mainFractalImage.refreshVisuals();
            }

            for (int i = 0; i < mainFractalImage.currentPalette.paletteColors.size(); i++)
            {
                ImGui::SameLine();

                char temp[24];
                sprintf_s(temp, "color##palettecolor%d", i);
                if(sfColorEdit3(temp, &mainFractalImage.currentPalette.paletteColors[i], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_InputRGB)) {
                mainFractalImage.generatePalette();
                mainFractalImage.refreshVisuals();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("+", btn_size)) {
                FractalImage::currentPalette.paletteColors.push_back(sf::Color());
                mainFractalImage.generatePalette();
                mainFractalImage.refreshVisuals();
            }

            if (ImGui::Button("Save")) {
                FractalImage::paletteList[paletteComboIndex] = FractalImage::currentPalette;
                displayPaletteEditor = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                FractalImage::currentPalette = FractalImage::paletteList[paletteComboIndex];
                currentPaletteIndex = paletteComboIndex;

                mainFractalImage.generatePalette();

                mainFractalImage.refreshVisuals();

                paletteNameBuffer = FractalImage::currentPalette.name.data();
                displayPaletteEditor = false;
            }

            ImGui::End();
        }

#pragma endregion

#pragma endregion

#pragma region Input_Stuff

        if (window.hasFocus() && !ImGui::IsPopupOpen("Render", ImGuiPopupFlags_AnyPopup) && !displayPaletteEditor) {

#pragma region WASD

            double offsetValue = 1.0 * t.asMicroseconds() / 10000000.0;
            if (!keyboard.isKeyPressed(keyboard.LShift)) {
                offsetValue *= 5;
            }

            sf::Vector2f newOrigin = fractalSprite.getOrigin();
            int originOffsetMultiplier = fractalSprite.getLocalBounds().height / fractalSprite.getScale().y * std::max(1, mainFractalImage.sampleDistance);

            if (keyboard.isKeyPressed(keyboard.W)) {
                mainFractalImage.centerY += offsetValue * mainFractalImage.scale;
                newOrigin.y -= offsetValue * originOffsetMultiplier;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }
            if (keyboard.isKeyPressed(keyboard.S)) {
                mainFractalImage.centerY -= offsetValue * mainFractalImage.scale;
                newOrigin.y += offsetValue * originOffsetMultiplier;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }
            if (keyboard.isKeyPressed(keyboard.A)) {
                mainFractalImage.centerX -= offsetValue * mainFractalImage.scale;
                newOrigin.x -= offsetValue * originOffsetMultiplier;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }
            if (keyboard.isKeyPressed(keyboard.D)) {
                mainFractalImage.centerX += offsetValue * mainFractalImage.scale;
                newOrigin.x += offsetValue * originOffsetMultiplier;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }

            fractalSprite.setOrigin(newOrigin);

#pragma endregion
#pragma region Zoomin

            double zoomAmount = 1.0f - (0.00001f * std::clamp(t.asMicroseconds(), (int64_t)0, (int64_t)10000));

            if (keyboard.isKeyPressed(keyboard.Multiply) || keyboard.isKeyPressed(keyboard.PageUp)) {
                if (keyboard.isKeyPressed(keyboard.LShift)) {
                    mainFractalImage.scale *= (float)std::pow(zoomAmount, 1.0 / 4);
                    fractalSprite.setScale(fractalSprite.getScale() / (float)std::pow(zoomAmount, 1.0/4));
                }
                else {
                    mainFractalImage.scale *= zoomAmount;
                    fractalSprite.setScale(fractalSprite.getScale() / (float)zoomAmount);
                }
                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }

            if (keyboard.isKeyPressed(keyboard.Divide) || keyboard.isKeyPressed(keyboard.PageDown)) {
                if (keyboard.isKeyPressed(keyboard.LShift)) {
                    mainFractalImage.scale /= (float)std::pow(zoomAmount, 1.0 / 4);
                    fractalSprite.setScale(fractalSprite.getScale() * (float)std::pow(zoomAmount, 1.0 / 4));
                }
                else {

                    mainFractalImage.scale /= zoomAmount;
                    fractalSprite.setScale(fractalSprite.getScale() * (float)zoomAmount);
                }
                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }

            if (keyboard.isKeyPressed(keyboard.Add)) {

                if (mainFractalImage.iterationMax < 10000000) {
                    if (mainFractalImage.iterationMax < 10) {
                        mainFractalImage.iterationMax += 1;
                    }
                    else {
                        mainFractalImage.iterationMax += pow(10, floor(log10(mainFractalImage.iterationMax)) - 1);
                    }
                    mainFractalImage.renderingStatus = NeedUpdate;
                }
            }
            if (keyboard.isKeyPressed(keyboard.Subtract)) {

                if (mainFractalImage.iterationMax > 1) {
                    if (mainFractalImage.iterationMax < 10) {
                        mainFractalImage.iterationMax -= 1;
                    }
                    else {
                        mainFractalImage.iterationMax -= pow(10, floor(log10(mainFractalImage.iterationMax)) - 1);
                    }
                    mainFractalImage.renderingStatus = NeedUpdate;
                }
            }

#pragma endregion


        }

#pragma endregion

        window.clear();

        if (mainFractalImage.renderingStatus != Ready) {
            renderSprite.setOrigin(fractalSprite.getOrigin() * (renderSprite.getLocalBounds().height / fractalSprite.getLocalBounds().height));
            renderSprite.setScale(fractalSprite.getScale() / (renderSprite.getLocalBounds().height / fractalSprite.getLocalBounds().height));
            renderSprite.setPosition(fractalSprite.getPosition());
            window.draw(renderSprite);
        }

        fractalTexture.update(mainFractalImage.pixelArray.data());
        window.draw(fractalSprite);

        if (isAnyKeyPressed) {
            mainFractalImage.cancelUpdate();
        }

        if ((mainFractalImage.renderingStatus == NeedUpdate || mainFractalImage.renderingStatus == Empty) && !isAnyKeyPressed) {

            renderTexture.update(window);
            mainFractalImage.pixelArray.assign(mainFractalImage.pixelArray.size(), 0);
            std::packaged_task<void()> task([&mainFractalImage] {
                mainFractalImage.updatePixels();
                });
            drawThread = task.get_future();
            std::thread t(std::move(task));
            t.detach();
            fractalSprite.setScale(sf::Vector2f(std::max(mainFractalImage.sampleDistance, 1), std::max(mainFractalImage.sampleDistance, 1)));
            fractalSprite.setOrigin((sf::Vector2f)fractalSprite.getLocalBounds().getSize() / 2.0f);
            fractalSprite.setPosition((sf::Vector2f)window.getSize() / 2.0f);

        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(5));

        windowTexture.update(window);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}