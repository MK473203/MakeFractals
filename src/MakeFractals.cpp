#include "MakeFractals.h"

// The code for combining everything together into an interactive application

int main(int argc, char *argv[])
{

#pragma region Initializations

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

#pragma region OpenGL

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 330
    const char *glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("MakeFractals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 960, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Basic rectangle vertices
    float vertices[] = {
        // positions         // texture coords
        1.0f, 1.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // The render shader transforms the image according to user input in order to make it easier to aim where you're going
    unsigned int renderVertexShader, renderFragmentShader, renderShaderProgram;
    renderVertexShader = glCreateShader(GL_VERTEX_SHADER);
    renderFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    renderShaderProgram = glCreateProgram();

    glShaderSource(renderVertexShader, 1, &renderVertShaderSource, NULL);
    glCompileShader(renderVertexShader);
    glShaderSource(renderFragmentShader, 1, &renderFragShaderSource, NULL);
    glCompileShader(renderFragmentShader);

    glAttachShader(renderShaderProgram, renderVertexShader);
    glAttachShader(renderShaderProgram, renderFragmentShader);
    glLinkProgram(renderShaderProgram);
    glDeleteShader(renderVertexShader);
    glDeleteShader(renderFragmentShader);

    // The screen shader is a very simple shader that draws an image to a rectangle
    unsigned int screenVertexShader, screenFragmentShader, screenShaderProgram;
    screenVertexShader = glCreateShader(GL_VERTEX_SHADER);
    screenFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    screenShaderProgram = glCreateProgram();

    glShaderSource(screenVertexShader, 1, &screenVertShaderSource, NULL);
    glCompileShader(screenVertexShader);
    glShaderSource(screenFragmentShader, 1, &screenFragShaderSource, NULL);
    glCompileShader(screenFragmentShader);

    glAttachShader(screenShaderProgram, screenVertexShader);
    glAttachShader(screenShaderProgram, screenFragmentShader);
    glLinkProgram(screenShaderProgram);
    glDeleteShader(screenVertexShader);
    glDeleteShader(screenFragmentShader);

#pragma endregion

#pragma region ImGui

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    std::string iniPath = prefPath + "imgui.ini";
    io.IniFilename = iniPath.c_str();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

#pragma endregion

    // Initialize the main fractal image
    FractalImage mainFractalImage(1280, 960);
    // mainFractalImage.loadPaletteList();
    mainFractalImage.generatePalette();
    calculateMandelbrotReference(mainFractalImage.centerX, mainFractalImage.centerY, mainFractalImage.scale, mainFractalImage.iterationMax);
    FractalImage renderingFractalImage = FractalImage();

#pragma region OpenGL_textures

    // fractalTexture shows the current rendering progress
    unsigned int fractalTexture;
    glGenTextures(1, &fractalTexture);
    glBindTexture(GL_TEXTURE_2D, fractalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.scaledWidth, mainFractalImage.scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mainFractalImage.pixelArray.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // backgroundTexture collects the previously rendered images or portions of unfinished images to show in the background as the new image is drawn on top of it
    unsigned int backgroundTexture;
    glGenTextures(1, &backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.scaledWidth, mainFractalImage.scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    // renderTexture combines fractalTexture and backgroundTexture
    unsigned int renderTexture;
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.scaledWidth, mainFractalImage.scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Frame buffer.
    unsigned int frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

#pragma endregion

    glm::vec3 translationVector = glm::vec3(0, 0, 0);
    float matrixScale = 100.0f;

    bool isAnyKeyPressed = false;
    float mouseZoomAmount = 1.0f;
    int mouseZoomDir = 0;
    float zoomAmount;
    float slowZoomAmount;
    float offsetValue;
    Uint64 lastClickTime = 0;

    // Intermediate variables between imgui windows and the main fractal image
    int SampleDistanceOrMultisampling = -1;
    int multisamplingTemp = 1;

    int currentPaletteIndex = 0;
    int paletteComboIndex = 0;
    int algorithmComboIndex = 0;
    int indexMapperComboIndex = 0;
    bool displayPaletteEditor = false;
    const char *paletteNameBuffer = FractalImage::currentPalette.name.data();

    float colorOffsetFloat = 0.0f;

    // Settings for image/video rendering
    int renderWidth = 1920;
    int renderHeight = 1080;
    int renderms = 1;
    int keyFrameCounter = 1;
    deltafloat startingScale;

    // Automatically fetch items for imgui drop down lists
    std::vector<const char *> algorithmComboItems;
    for (auto it = fractalAlgorithms.begin(); it != fractalAlgorithms.end(); it++)
    {
        algorithmComboItems.push_back(it->first);
    }

    std::vector<const char *> indexMapperComboItems;
    for (auto it = indexMapAlgorithms.begin(); it != indexMapAlgorithms.end(); it++)
    {
        indexMapperComboItems.push_back(it->first);
    }

    std::string savePath = prefPath + "save.txt";

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    double deltatime = 0;

#pragma endregion

    bool done = false;

    while (!done)
    {

        // Window and input event handling
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type)
            {
            case SDL_QUIT:
                mainFractalImage.cancelUpdate();
                mainFractalImage.savePaletteList();
                done = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.windowID != SDL_GetWindowID(window))
                {
                    break;
                }
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    mainFractalImage.cancelUpdate();
                    mainFractalImage.savePaletteList();
                    done = true;
                }
                else if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {

                    mainFractalImage.cancelUpdate();
                    mainFractalImage.width = event.window.data1;
                    mainFractalImage.height = event.window.data2;
                    mainFractalImage.aspectRatio = (float)event.window.data1 / event.window.data2;
                    mainFractalImage.resizeArrays();
                    mainFractalImage.renderingStatus = NeedUpdate;

                    glViewport(0, 0, event.window.data1, event.window.data2);
                    glBindTexture(GL_TEXTURE_2D, fractalTexture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.scaledWidth, mainFractalImage.scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.width, mainFractalImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, renderTexture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.width, mainFractalImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                    glGenerateMipmap(GL_TEXTURE_2D);
                }
                break;
            case SDL_KEYUP:
                isAnyKeyPressed = false;
                break;
            case SDL_MOUSEBUTTONUP:
                isAnyKeyPressed = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                lastClickTime = SDL_GetPerformanceCounter();
                break;
            case SDL_MOUSEWHEEL:
                mouseZoomAmount = 1.0f - abs(event.wheel.preciseY) / 5;
                mouseZoomDir = event.wheel.y;
                mainFractalImage.renderingStatus = NeedUpdate;
                break;
            default:
                break;
            }
        }

#pragma region INPUT_STUFF

        last = now;
        now = SDL_GetPerformanceCounter();

        deltatime = (double)((now - last) * 1000 / (double)SDL_GetPerformanceFrequency());

        int mousePos[2];
        Uint32 mouseState = SDL_GetMouseState(&mousePos[0], &mousePos[1]);

        int dx, dy;
        SDL_GetRelativeMouseState(&dx, &dy);

        if ((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0 && !ImGui::IsPopupOpen("Render", ImGuiPopupFlags_AnyPopup) && !displayPaletteEditor && !io.WantCaptureKeyboard)
        {
            const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

#pragma region Zoomin

            zoomAmount = 1.0f - (std::clamp(deltatime, 0.0, 10.0) / 200);
            slowZoomAmount = std::pow(zoomAmount, 1.0f / 4);

            if (keyboard[SDL_SCANCODE_KP_MULTIPLY] || keyboard[SDL_SCANCODE_PAGEUP])
            {
                if (keyboard[SDL_SCANCODE_LSHIFT])
                {
                    mainFractalImage.scale *= slowZoomAmount;
                    matrixScale /= slowZoomAmount;
                }
                else
                {
                    mainFractalImage.scale *= zoomAmount;
                    matrixScale /= zoomAmount;
                }
                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }

            if (keyboard[SDL_SCANCODE_KP_DIVIDE] || keyboard[SDL_SCANCODE_PAGEDOWN])
            {
                if (keyboard[SDL_SCANCODE_LSHIFT])
                {
                    mainFractalImage.scale /= slowZoomAmount;
                    matrixScale *= slowZoomAmount;
                }
                else
                {

                    mainFractalImage.scale /= zoomAmount;
                    matrixScale *= zoomAmount;
                }
                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }

#pragma endregion

#pragma region WASD

            offsetValue = 1.0 * deltatime / 10000.0;
            if (!keyboard[SDL_SCANCODE_LSHIFT])
            {
                offsetValue *= 5;
            }
            else
            {
                isAnyKeyPressed = true;
            }

            if (keyboard[SDL_SCANCODE_W])
            {
                mainFractalImage.centerY += mainFractalImage.scale * offsetValue;
                translationVector.y -= 2 * offsetValue / matrixScale;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }
            if (keyboard[SDL_SCANCODE_S])
            {
                mainFractalImage.centerY -= mainFractalImage.scale * offsetValue;
                translationVector.y += 2 * offsetValue / matrixScale;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }
            if (keyboard[SDL_SCANCODE_A])
            {
                mainFractalImage.centerX -= mainFractalImage.scale * offsetValue;
                translationVector.x += 2 * offsetValue / matrixScale / mainFractalImage.aspectRatio;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }
            if (keyboard[SDL_SCANCODE_D])
            {
                mainFractalImage.centerX += mainFractalImage.scale * offsetValue;
                translationVector.x -= 2 * offsetValue / matrixScale / mainFractalImage.aspectRatio;

                mainFractalImage.renderingStatus = NeedUpdate;
                isAnyKeyPressed = true;
            }

#pragma endregion

#pragma region MOUSE

            bool shouldImgMove = false;

            if (mouseZoomDir > 0)
            {
                if (keyboard[SDL_SCANCODE_LSHIFT])
                {
                    mainFractalImage.scale *= std::pow(mouseZoomAmount, 1.0f / 4);
                    matrixScale /= std::pow(mouseZoomAmount, 1.0f / 4);
                }
                else
                {
                    mainFractalImage.scale *= mouseZoomAmount;
                    matrixScale /= mouseZoomAmount;
                }

                if ((mouseState & SDL_BUTTON_LMASK) == 0)
                {

                    dx = (mainFractalImage.width / 2 - mousePos[0]) / 4;
                    dy = (mainFractalImage.height / 2 - mousePos[1]) / 4;

                    shouldImgMove = true;
                }
            }

            if (mouseZoomDir < 0)
            {
                if ((mouseState & SDL_BUTTON_LMASK) == 0)
                {

                    dx = -(mainFractalImage.width / 2 - mousePos[0]) / 4;
                    dy = -(mainFractalImage.height / 2 - mousePos[1]) / 4;

                    shouldImgMove = true;
                }
            }

            if ((mouseState & SDL_BUTTON_LMASK) != 0 && !io.WantCaptureMouse)
            {

                if (SDL_GetPerformanceCounter() - lastClickTime < 100)
                {
                    dx = 0;
                    dy = 0;
                }

                isAnyKeyPressed = true;
                shouldImgMove = true;
            }

            if (shouldImgMove)
            {

                if (dx != 0)
                {

                    float xOffset = ((float)dx / mainFractalImage.width);
                    mainFractalImage.centerX -= mainFractalImage.scale * xOffset * mainFractalImage.aspectRatio;
                    translationVector.x += 2 * xOffset / matrixScale;
                    mainFractalImage.renderingStatus = NeedUpdate;
                }
                if (dy != 0)
                {

                    float yOffset = ((float)dy / mainFractalImage.height);
                    mainFractalImage.centerY += mainFractalImage.scale * yOffset;
                    translationVector.y -= 2 * yOffset / matrixScale;
                    mainFractalImage.renderingStatus = NeedUpdate;
                }
            }

            if (mouseZoomDir < 0)
            {
                if (keyboard[SDL_SCANCODE_LSHIFT])
                {
                    mainFractalImage.scale /= std::pow(mouseZoomAmount, 1.0f / 4);
                    matrixScale *= std::pow(mouseZoomAmount, 1.0f / 4);
                }
                else
                {
                    mainFractalImage.scale /= mouseZoomAmount;
                    matrixScale *= mouseZoomAmount;
                }
            }

#pragma endregion

            if (keyboard[SDL_SCANCODE_KP_PLUS])
            {

                if (mainFractalImage.iterationMax < 10000000)
                {
                    if (mainFractalImage.iterationMax < 10)
                    {
                        mainFractalImage.iterationMax += 1;
                    }
                    else
                    {
                        mainFractalImage.iterationMax += pow(10, floor(log10(mainFractalImage.iterationMax)) - 1);
                    }
                    mainFractalImage.renderingStatus = NeedUpdate;
                }
            }
            if (keyboard[SDL_SCANCODE_MINUS])
            {

                if (mainFractalImage.iterationMax > 1)
                {
                    if (mainFractalImage.iterationMax < 10)
                    {
                        mainFractalImage.iterationMax -= 1;
                    }
                    else
                    {
                        mainFractalImage.iterationMax -= pow(10, floor(log10(mainFractalImage.iterationMax)) - 1);
                    }
                    mainFractalImage.renderingStatus = NeedUpdate;
                }
            }
        }

#pragma endregion

        mouseZoomAmount = 1;
        mouseZoomDir = 0;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

#pragma region ImGui_Windows

#pragma region Main_settings_window

        ImGui::Begin("Main##mainsettings", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::BeginTabBar("MainTabBar", 0))
        {

            // ALGORITHM TAB
            if (ImGui::BeginTabItem("Algorithm"))
            {

                if (ImGui::InputInt("Iterations", &mainFractalImage.iterationMax, pow(10, floor(log10(mainFractalImage.iterationMax)) - 1), 0, 0 | ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (mainFractalImage.iterationMax > 1000000000 || mainFractalImage.iterationMax < 10)
                    {
                        mainFractalImage.iterationMax = std::max(std::min(mainFractalImage.iterationMax, 1000000), 10);
                    }
                    else
                    {
                        mainFractalImage.renderingStatus = NeedUpdate;
                    }
                }

                if (ImGui::Combo("Algorithm##algorithmcombo", &algorithmComboIndex, algorithmComboItems.data(), algorithmComboItems.size()))
                {
                    FractalImage::currentAlgorithm = fractalAlgorithms[algorithmComboItems[algorithmComboIndex]];

                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                if (FractalImage::currentAlgorithm == &MandelbrotSAPerturbation)
                {

                    ImGui::Checkbox("Automate SA", &useAutomaticSeriesApproximation);

                    ImGui::BeginDisabled(useAutomaticSeriesApproximation);

                    if (ImGui::InputInt("Approximation terms##satermsinput", &seriesLength, 1, 10, 0 | ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
                    {

                        mainFractalImage.renderingStatus = NeedUpdate;
                    }

                    if (ImGui::InputInt("Approximation Iterations##saitersinput", (int *)&perturbationStartingIter, 1, 10, 0 | ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
                    {

                        mainFractalImage.renderingStatus = NeedUpdate;
                    }

                    ImGui::EndDisabled();
                }

                if (ImGui::Button("Refresh"))
                {
                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                /*if (ImGui::CollapsingHeader("Iteration flags")) {
                    static bool disableTests = false;
                    static bool disableSmooth = false;
                    static bool disableShadow = false;

                    int flags = 0;
                    if (disableTests) flags |= DisableTests;
                    if (disableSmooth) flags |= DisableSmoothing;
                    if (disableShadow) flags |= DisableShadowCalculation;

                    if (ImGui::Checkbox("Disable tests", &disableTests)) mainFractalImage.renderingStatus = NeedUpdate;
                    ImGui::SameLine(); HelpMarker("Disables some checks that test if a given point is inside the set.\nFaster when left unchecked in most cases except when there are very few points belonging to the set visible");
                    if (ImGui::Checkbox("Disable smoothing", &disableSmooth)) mainFractalImage.renderingStatus = NeedUpdate;
                    if (ImGui::Checkbox("Disable shadow value calculation", &disableShadow)) mainFractalImage.renderingStatus = NeedUpdate;

                    mainFractalImage.flags = flags;
                }*/

                ImGui::EndTabItem();
            }

            // RENDERING TAB
            if (ImGui::BeginTabItem("Rendering"))
            {

                ImGui::RadioButton("Sample Distance", &SampleDistanceOrMultisampling, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Multisampling", &SampleDistanceOrMultisampling, -1);
                if (ImGui::InputInt("##mutisamplinginput", &multisamplingTemp, 1, 1, 0 | ImGuiInputTextFlags_CharsDecimal))
                {

                    mainFractalImage.cancelUpdate();
                    if (multisamplingTemp == 0)
                        multisamplingTemp = 1;
                    if (multisamplingTemp * SampleDistanceOrMultisampling < -4)
                    {
                        multisamplingTemp = 4;
                    }
                    mainFractalImage.sampleDistance = multisamplingTemp * SampleDistanceOrMultisampling;
                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                if (ImGui::Combo("Order##indexmappercombo", &indexMapperComboIndex, indexMapperComboItems.data(), indexMapperComboItems.size()))
                {
                    FractalImage::indexMapper = indexMapAlgorithms[indexMapperComboItems[indexMapperComboIndex]];
                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                if (ImGui::Checkbox("Reversed##reverseordercheckbox", &FractalImage::reverseRenderOrder))
                {
                }

                if (ImGui::Button("Refresh"))
                {
                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                if (ImGui::Button("Quick render"))
                {
                    mainFractalImage.saveToImage((prefPath + "images/").c_str(), false);
                    mainFractalImage.renderingStatus = Ready;
                }
                ImGui::SameLine();
                if (ImGui::Button("Advanced"))
                {
                    renderingFractalImage = FractalImage(mainFractalImage);
                    renderingFractalImage.width = 1280;
                    renderingFractalImage.height = 720;
                    renderingFractalImage.renderingStatus = Empty;
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
                    if (renderingFractalImage.renderingStatus != Ready && renderingFractalImage.renderingStatus != Empty)
                    {
                        ImGui::BeginDisabled(true);
                    }

                    ImGui::BeginGroup();
                    ImGui::Text("Image resolution");

                    static int resolutionComboIndex = 1;
                    if (ImGui::Combo("Presets", &resolutionComboIndex, "480p\000720p\0001080p\0001440p\0004k\0008k\000\000"))
                    {
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

                    if (renderType != 0)
                    {
                        // Different video settings here
                    }

                    int keyFrameAmount = floor(log(startingScale / 3.0) / log(0.5)).convert_to<int>();

                    if (ImGui::Button("Render"))
                    {
                        renderingFractalImage.sampleDistance = -renderms;
                        FractalImage::indexMapper = &topToBottom;

                        if (renderType != 0)
                        {

                            std::thread t([&renderingFractalImage, &keyFrameCounter]
                                          { renderingFractalImage.generateZoomVideo(keyFrameCounter); });
                            t.detach();
                        }
                        else
                        {
                            std::thread t([&renderingFractalImage]
                                          { renderingFractalImage.saveToImage((prefPath + "images/").c_str(), false); });
                            t.detach();
                        }
                    }

                    if (renderingFractalImage.renderingStatus != Ready && renderingFractalImage.renderingStatus != Empty)
                    {
                        ImGui::EndDisabled();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        renderingFractalImage.cancelUpdate();
                        renderingFractalImage.pixelArray.clear();
                        renderingFractalImage.dataArray.clear();
                        keyFrameCounter = 1;
                        ImGui::CloseCurrentPopup();
                    }

                    if (renderingFractalImage.renderingStatus == Rendering)
                    {
                        if (renderType != 0)
                        {
                            char buf[32];
                            snprintf(buf, sizeof(buf), "keyframe %d/%d: %.0f%%", keyFrameCounter, keyFrameAmount, renderingFractalImage.renderProgress * 100 + 0.01f);
                            ImGui::ProgressBar(renderingFractalImage.renderProgress, ImVec2(-FLT_MIN, 0), buf);
                        }
                        else
                        {
                            ImGui::ProgressBar(renderingFractalImage.renderProgress);
                        }
                    }
                    else if (renderingFractalImage.renderingStatus == Ready && renderType == 0)
                    {
                        ImGui::Text("Saving image to file...");
                    }
                    else if ((renderingFractalImage.renderingStatus == ImageSaved && renderType == 0) ||
                             renderingFractalImage.renderingStatus == VideoSaved)
                    {
                        renderingFractalImage.pixelArray.clear();
                        renderingFractalImage.dataArray.clear();
                        ImGui::CloseCurrentPopup();
                        FractalImage::indexMapper = indexMapAlgorithms[indexMapperComboItems[indexMapperComboIndex]];
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::InputInt("Threads", &mainFractalImage.threadAmount, 1, 1, 0 | ImGuiInputTextFlags_CharsDecimal))
                {
                    mainFractalImage.threadAmount = std::clamp(mainFractalImage.threadAmount, 1, 4 * (int)std::thread::hardware_concurrency());
                }

                ImGui::EndTabItem();
            }

            // VISUALS TAB
            if (ImGui::BeginTabItem("Visuals"))
            {

                static int colorMapTypeCombo = 0;
                if (ImGui::Combo("Coloring method", &colorMapTypeCombo, "Linear\000Exponential\000Logarithmic\000Histogram\000\000"))
                {
                    FractalImage::currentColorMapType = (colorMapType)colorMapTypeCombo;
                    if (FractalImage::currentColorMapType == Histogram)
                        mainFractalImage.generateHistogram();
                    mainFractalImage.refreshVisuals();
                }

                if (FractalImage::currentColorMapType == Exponential)
                {
                    if (ImGui::DragFloat("Exponent", &FractalImage::colorMapExponent, 0.01f, 0.0f, 2.0f, "%.2f", 0 | ImGuiSliderFlags_AlwaysClamp))
                    {
                        mainFractalImage.refreshVisuals();
                    }
                }

                if (FractalImage::shadowFx)
                {
                    if (ImGui::DragFloat("##shadowstrength", &FractalImage::shadowStrength, 0.01f, 0.0f, 1.0f, "%.2f", 0 | ImGuiSliderFlags_AlwaysClamp))
                    {
                        mainFractalImage.refreshVisuals();
                    }

                    ImGui::SameLine();
                }

                if (ImGui::Checkbox("Shadow##shadowcheckbox", &FractalImage::shadowFx))
                {
                    mainFractalImage.refreshVisuals();
                }

                if (FractalImage::shadowFx)
                {

                    if (ImGui::DragInt("Shadow angle", &shadowAngle, 1.0f, 0, 360, "%d", 0 | ImGuiSliderFlags_AlwaysClamp))
                    {
                        updateShadowVars();
                        mainFractalImage.renderingStatus = NeedUpdate;
                    }
                    ImGui::SameLine();
                    HelpMarker("Warning! Requires re-rendering");

                    if (ImGui::DragFloat("Shadow height", &h2, 0.01f, 0.0f, 10.0f, "%.2f", 0 | ImGuiSliderFlags_AlwaysClamp))
                    {
                        mainFractalImage.renderingStatus = NeedUpdate;
                    }
                    ImGui::SameLine();
                    HelpMarker("Warning! Requires re-rendering");
                }

                if (ImGui::Button("<<"))
                {
                    mainFractalImage.iterDiv >>= 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                if (ImGui::Button("-1"))
                {
                    mainFractalImage.iterDiv -= 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                if (ImGui::DragInt("##iterdivslider", &mainFractalImage.iterDiv))
                {
                    mainFractalImage.refreshVisuals();
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button("+1"))
                {
                    mainFractalImage.iterDiv += 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                if (ImGui::Button(">>"))
                {
                    mainFractalImage.iterDiv <<= 1;
                    mainFractalImage.refreshVisuals();
                }
                ImGui::SameLine();
                ImGui::Text("Iter div");

                const char *combo_preview_value = FractalImage::paletteList[paletteComboIndex].name.c_str();
                ImGui::PushItemWidth(100);
                if (ImGui::BeginCombo("##palettepickercombo", combo_preview_value, 0))
                {
                    for (int n = 0; n < FractalImage::paletteList.size(); n++)
                    {
                        const bool is_selected = (paletteComboIndex == n);
                        std::string selectableLabel = FractalImage::paletteList[n].name + std::string("##palette") + std::to_string(n);
                        if (ImGui::Selectable(selectableLabel.c_str(), is_selected))
                        {
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

                if (ImGui::Button("Edit##editpalette"))
                {
                    displayPaletteEditor = !displayPaletteEditor;
                }

                ImGui::SameLine();
                ImGui::Text("Color palette");

                if (ImGui::DragFloat("Color offset", &colorOffsetFloat, 0.005f, 0.0f, 1.0f, "%.3f", 0 | ImGuiSliderFlags_AlwaysClamp))
                {

                    FractalImage::ColorOffset = FractalImage::ColorAmount * colorOffsetFloat;

                    mainFractalImage.refreshVisuals();
                }

                ImGui::EndTabItem();
            }

            // MISC TAB
            if (ImGui::BeginTabItem("Misc"))
            {

                if (ImGui::Button("Save", ImVec2(80, 25)))
                {

                    mainFractalImage.savePaletteList();

                    std::ofstream ofs(savePath);
                    if (ofs.good())
                    {
                        ofs << mainFractalImage << '\n';
                        ofs << currentPaletteIndex;
                    }

                    ofs.close();
                }

                ImGui::SameLine(0.0f, 10.0f);

                if (ImGui::Button("Load", ImVec2(80, 25)))
                {

                    mainFractalImage.loadPaletteList();

                    std::ifstream ifs(savePath);
                    if (ifs.good())
                    {
                        ifs >> mainFractalImage;
                        ifs >> currentPaletteIndex;
                    }
                    ifs.close();

                    FractalImage::currentPalette = FractalImage::paletteList[currentPaletteIndex];

                    mainFractalImage.generatePalette();

                    int comboindex = 0;
                    for (auto i = fractalAlgorithms.begin(); i != fractalAlgorithms.end(); i++)
                    {
                        if (i->second == FractalImage::currentAlgorithm)
                        {
                            algorithmComboIndex = comboindex;
                            break;
                        }
                        comboindex++;
                    }
                    comboindex = 0;
                    for (auto i = indexMapAlgorithms.begin(); i != indexMapAlgorithms.end(); i++)
                    {
                        if (i->second == FractalImage::indexMapper)
                        {
                            indexMapperComboIndex = comboindex;
                            break;
                        }
                        comboindex++;
                    }

                    mainFractalImage.renderingStatus = NeedUpdate;
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

#pragma endregion

#pragma region Debug_info_window

        ImGui::Begin("Debug info", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text((std::to_string((int)ImGui::GetIO().Framerate) + " FPS").c_str());
        ImGui::Text(("Last render took " + std::to_string(mainFractalImage.lastRenderTime) + " ms").c_str());
        ImGui::Text(mainFractalImage.debugInfo().c_str());
        ImGui::Text(("Lowest iter: " + std::to_string(lowestIter)).c_str());
        ImGui::Text(("Highest iter: " + std::to_string(highestIter)).c_str());
        ImGui::Text(("Perturbation start iter: " + std::to_string(perturbationStartingIter)).c_str());
        ImGui::Text(("Highest reference iter: " + std::to_string(perturbationEndIter)).c_str());
        ImGui::Text(("Reference X: " + std::to_string(((refPointX - mainFractalImage.centerX) / mainFractalImage.scale).convert_to<double>())).c_str());
        ImGui::Text(("Reference Y: " + std::to_string(((refPointY - mainFractalImage.centerY) / mainFractalImage.scale).convert_to<double>())).c_str());

        static bool showPixelInfoWindow = false;

        ImGui::Checkbox("Show pixel info window", &showPixelInfoWindow);
        if (ImGui::Checkbox("Show debug colors", &FractalImage::debugColors))
        {
            mainFractalImage.refreshVisuals();
        }

        ImGui::End();

#pragma endregion

#pragma region Aiming_Window

        // Shows a zoomed in image of the center of the window in order to make precise aiming easier
        ImGui::Begin("Aiming", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        int aimingSquareSize = 16 << 1;

        ImVec2 uv1 = ImVec2((float)(mainFractalImage.width / 2 - aimingSquareSize) / mainFractalImage.width, (float)(mainFractalImage.height / 2 + aimingSquareSize) / mainFractalImage.height);
        ImVec2 uv2 = ImVec2((float)(mainFractalImage.width / 2 + aimingSquareSize) / mainFractalImage.width, (float)(mainFractalImage.height / 2 - aimingSquareSize) / mainFractalImage.height);

        ImGui::Image((ImTextureID)renderTexture, ImVec2(200, 200), uv1, uv2);

        ImGui::End();

#pragma endregion

#pragma region Pixel_information_window

        if (showPixelInfoWindow)
        {

            mousePos[1] = mainFractalImage.height - mousePos[1];

            int dataArrayIndex;

            if (mainFractalImage.sampleDistance <= -1)
            {
                dataArrayIndex = (mousePos[0] + mousePos[1] * mainFractalImage.width) * mainFractalImage.sampleDistance * mainFractalImage.sampleDistance;
            }
            else
            {
                dataArrayIndex = (float)mousePos[0] / mainFractalImage.width * mainFractalImage.scaledWidth;
                dataArrayIndex += (int)((float)mousePos[1] / mainFractalImage.height * mainFractalImage.scaledHeight) * mainFractalImage.scaledWidth;
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

            SDL_Color pixelColor = mainFractalImage.mapFractalData(fd);

            ImGui::BeginTooltip();
            sdlColorEdit3("##pixelcolor", &pixelColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            ImGui::TextUnformatted((
                                       "Iterations: " + std::to_string(fd.iterations) + '\n' +
                                       std::to_string(fd.iterResult) + " (" + testString + ")" + '\n' +
                                       "Smooth value: " + std::to_string(fd.smoothValue) + '\n' +
                                       "Shadow value: " + std::to_string(fd.shadowValue) + '\n')
                                       .c_str());
            ImGui::EndTooltip();
        }

#pragma endregion

#pragma region Palette_Editor_Window

        if (displayPaletteEditor)
        {
            ImGui::Begin("PaletteEditor", 0, 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::Button("Load defaults##defaultpalettes"))
            {
                FractalImage::paletteList = FractalImage::defaultPaletteList;

                paletteComboIndex = 0;

                FractalImage::currentPalette = FractalImage::paletteList[paletteComboIndex];
                currentPaletteIndex = paletteComboIndex;

                mainFractalImage.generatePalette();

                mainFractalImage.refreshVisuals();

                paletteNameBuffer = FractalImage::currentPalette.name.data();
            }

            if (ImGui::InputText("##palettenameeditor", (char *)paletteNameBuffer, ImGuiInputTextFlags_AutoSelectAll))
            {
                FractalImage::currentPalette.name = paletteNameBuffer;
            }

            ImGui::SameLine();

            if (ImGui::BeginCombo("##palettepickercombo", "", ImGuiComboFlags_NoPreview))
            {
                for (int n = 0; n < FractalImage::paletteList.size(); n++)
                {
                    const bool is_selected = (paletteComboIndex == n);
                    std::string selectableLabel = FractalImage::paletteList[n].name + std::string("##palette") + std::to_string(n);
                    if (ImGui::Selectable(selectableLabel.c_str(), is_selected))
                    {
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

            if (ImGui::Button("New##newpalette"))
            {
                FractalImage::paletteList.push_back(FractalImage::defaultPaletteList[0]);
                FractalImage::paletteList.back().name = "New Palette";

                paletteComboIndex = FractalImage::paletteList.size() - 1;

                FractalImage::currentPalette = FractalImage::paletteList[paletteComboIndex];
                currentPaletteIndex = paletteComboIndex;

                mainFractalImage.generatePalette();

                mainFractalImage.refreshVisuals();

                paletteNameBuffer = FractalImage::currentPalette.name.data();
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete##deletepalette") && FractalImage::paletteList.size() > 1)
            {
                FractalImage::paletteList.erase(FractalImage::paletteList.begin() + paletteComboIndex);
                paletteComboIndex = std::min(paletteComboIndex, (int)FractalImage::paletteList.size() - 1);

                FractalImage::currentPalette = FractalImage::paletteList[paletteComboIndex];
                currentPaletteIndex = paletteComboIndex;

                mainFractalImage.generatePalette();

                mainFractalImage.refreshVisuals();

                paletteNameBuffer = FractalImage::currentPalette.name.data();
            }

            ImVec2 btn_size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

            if (ImGui::Button("-##removecolor", btn_size) && FractalImage::currentPalette.paletteColors.size() > 1)
            {
                FractalImage::currentPalette.paletteColors.erase(FractalImage::currentPalette.paletteColors.end() - 1);
                mainFractalImage.generatePalette();
                mainFractalImage.refreshVisuals();
            }

            for (int i = 0; i < FractalImage::currentPalette.paletteColors.size(); i++)
            {
                ImGui::SameLine();

                char temp[32];
                snprintf(temp, sizeof(temp), "color##palettecolor%d", i);
                if (sdlColorEdit3(temp, &FractalImage::currentPalette.paletteColors[i], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_InputRGB))
                {
                    mainFractalImage.generatePalette();
                    mainFractalImage.refreshVisuals();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("+##addcolor", btn_size))
            {
                FractalImage::currentPalette.paletteColors.push_back({0, 0, 0});
                mainFractalImage.generatePalette();
                mainFractalImage.refreshVisuals();
            }

            if (ImGui::Button("Save"))
            {
                FractalImage::paletteList[paletteComboIndex] = FractalImage::currentPalette;
                displayPaletteEditor = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
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

        /*
        #pragma region Texture windows

                int texWindowSize = 144;

                ImGui::Begin("FracTex", 0, 0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

                uv1 = ImVec2(0, 1);
                uv2 = ImVec2(1, 0);


                ImGui::Image((ImTextureID)fractalTexture, ImVec2(texWindowSize * mainFractalImage.aspectRatio, texWindowSize), uv1, uv2);

                ImGui::End();

                ImGui::Begin("BackTex", 0, 0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

                ImGui::Image((ImTextureID)backgroundTexture, ImVec2(texWindowSize * mainFractalImage.aspectRatio, texWindowSize), uv1, uv2);

                ImGui::End();
                ImGui::Begin("RendTex", 0, 0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

                ImGui::Image((ImTextureID)renderTexture, ImVec2(texWindowSize * mainFractalImage.aspectRatio, texWindowSize), uv1, uv2);

                ImGui::End();

        #pragma endregion
        */

#pragma endregion

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(renderShaderProgram);

        glm::mat4 fullTransform = glm::translate(glm::mat4(1.0f), translationVector);
        fullTransform = glm::translate(fullTransform, -translationVector);
        fullTransform = glm::scale(fullTransform, glm::vec3(matrixScale, matrixScale, 1.0f));
        fullTransform = glm::translate(fullTransform, translationVector);

        glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "transform"), 1, 0, glm::value_ptr(fullTransform));

        if (mainFractalImage.renderingStatus != Ready)
        {
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glBindTexture(GL_TEXTURE_2D, fractalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainFractalImage.scaledWidth, mainFractalImage.scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mainFractalImage.pixelArray.data());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // We're moving. Let's cancel the current rendering process.
        if (isAnyKeyPressed)
        {
            mainFractalImage.cancelUpdate();
        }

        // We've settled on a location to start rendering. Let's start the rendering process.
        if ((mainFractalImage.renderingStatus == NeedUpdate || mainFractalImage.renderingStatus == Empty) && !isAnyKeyPressed)
        {

            // Copy the window content to backgroundTexture
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);

            glUseProgram(screenShaderProgram);

            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

            // Reset the fractalImage transform
            translationVector = glm::vec3(0, 0, 0);
            matrixScale = 1.0f;

            // Clear the contents of the main fractal image and then start rendering once again
            mainFractalImage.pixelArray.assign(mainFractalImage.pixelArray.size(), 0);
            mainFractalImage.updatePixels();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(screenShaderProgram);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Imgui rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}