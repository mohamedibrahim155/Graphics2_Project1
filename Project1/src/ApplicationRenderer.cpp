#include"ApplicationRenderer.h"


ApplicationRenderer::ApplicationRenderer()
{
    sceneViewcamera = new Camera();
    sceneViewcamera->name = "Sceneview Camera";

    gameScenecamera = new Camera();
    gameScenecamera->name = "GameScene Camera";

    renderTextureCamera = new Camera();
    renderTextureCamera->name = "RenderTexture Camera";

    renderTextureCamera2 = new Camera();
    renderTextureCamera2->name = "RenderTexture Camera2";
}

ApplicationRenderer::~ApplicationRenderer()
{
}



void ApplicationRenderer::WindowInitialize(int width, int height,  std::string windowName)
{
    windowWidth = width;
    WindowHeight = height;
    lastX = windowWidth / 2.0f;
    lastY= WindowHeight / 2.0f;

    glfwInit();



    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int x, int y)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(w))->SetViewPort(w, x, y);
        });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->KeyCallBack(window, key, scancode, action, mods);
        });


    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xposIn, double yposIn)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->MouseCallBack(window, xposIn, yposIn);
        });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->MouseScroll(window, xoffset, yoffset);
        });
   
   

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 2.0f;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
   // ImGui_ImplOpenGL3_Init("#version 130");

    //Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Query and print OpenGL version
    const GLubyte* version = glGetString(GL_VERSION);
    if (version) {
        std::cout << "OpenGL Version: " << version << std::endl;
    }
    else 
    {
        std::cerr << "Failed to retrieve OpenGL version\n";
     
    }


    FrameBufferSpecification specification;

    specification.width = windowWidth;
    specification.height = WindowHeight;
    specification.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH };
    

    sceneViewframeBuffer = new FrameBuffer(specification);

    gameframeBuffer = new FrameBuffer(specification);

    EditorLayout::GetInstance().applicationRenderer = this;
  

    InitializeShaders();
   
    GraphicsRender::GetInstance().InitializeGraphics();

    DebugModels::GetInstance().defaultCube = new Model("Models/DefaultCube/DefaultCube.fbx", false, true);
    DebugModels::GetInstance().defaultSphere = new Model("Models/DefaultSphere/DefaultSphere.fbx", false, true);
    DebugModels::GetInstance().defaultQuad = new Model("Models/DefaultQuad/DefaultQuad.fbx", false, true);

    InitializeSkybox();

    GraphicsRender::GetInstance().SetCamera(sceneViewcamera);

    sceneViewcamera->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 100.0f);
    sceneViewcamera->transform.position = glm::vec3(0.08f, 0.56f, 0.23f);
   // sceneViewcamera->transform.SetRotation(glm::vec3(-10.60f, 2.20f,0));
    sceneViewcamera->transform.SetRotation(glm::vec3(-2.30f,0,0));

    gameScenecamera->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 100.0f);
    gameScenecamera->transform.position = glm::vec3(0.08f, 0.56f, 0.23f);
    gameScenecamera->transform.SetRotation(glm::vec3(-2.30f, 0, 0));

    renderTextureCamera->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 100.0f);
    renderTextureCamera->transform.position = glm::vec3(25.59f, 0.26f, 6.79f);
    renderTextureCamera->transform.SetRotation(  glm::vec3(-5.60f, 166.10f, 0.00f));

    renderTextureCamera->IntializeRenderTexture(specification);

    renderTextureCamera2->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 100.0f);
    renderTextureCamera2->transform.position = glm::vec3(7.53f, 1.51f, 20.04f);
    renderTextureCamera2->transform.SetRotation(glm::vec3(-9.70f, 353.20f, 0.00f));

    renderTextureCamera2->IntializeRenderTexture(specification);
   // renderTextureCamera->IntializeRenderTexture(new RenderTexture());
  
    isImguiPanelsEnable = true;
}

void ApplicationRenderer::InitializeShaders()
{
    defaultShader = new Shader("Shaders/DefaultShader_Vertex.vert", "Shaders/DefaultShader_Fragment.frag");
    solidColorShader = new Shader("Shaders/SolidColor_Vertex.vert", "Shaders/SolidColor_Fragment.frag", SOLID);
    stencilShader = new Shader("Shaders/StencilOutline.vert", "Shaders/StencilOutline.frag", OPAQUE);
    //ScrollShader = new Shader("Shaders/ScrollTexture.vert", "Shaders/ScrollTexture.frag");

    alphaBlendShader = new Shader("Shaders/DefaultShader_Vertex.vert", "Shaders/DefaultShader_Fragment.frag", ALPHA_BLEND);
    alphaBlendShader->blendMode = ALPHA_BLEND;

    alphaCutoutShader = new Shader("Shaders/DefaultShader_Vertex.vert", "Shaders/DefaultShader_Fragment.frag", ALPHA_CUTOUT);
    alphaCutoutShader->blendMode = ALPHA_CUTOUT;

    skyboxShader = new Shader("Shaders/SkyboxShader.vert", "Shaders/SkyboxShader.frag");
    skyboxShader->modelUniform = false;

    GraphicsRender::GetInstance().defaultShader = defaultShader;
    GraphicsRender::GetInstance().solidColorShader = solidColorShader;
    GraphicsRender::GetInstance().stencilShader = stencilShader; 

   

}

void ApplicationRenderer::InitializeSkybox()
{
    skyBoxModel = new Model("Models/DefaultCube/DefaultCube.fbx", false, true);
    skyBoxModel->meshes[0]->meshMaterial = new SkyboxMaterial();

    skyBoxMaterial = skyBoxModel->meshes[0]->meshMaterial->skyboxMaterial();

    std::vector<std::string> faces
    {
       ("Textures/skybox/right.jpg"),
       ("Textures/skybox/left.jpg"),
       ("Textures/skybox/top.jpg"),
       ("Textures/skybox/bottom.jpg"),
       ("Textures/skybox/front.jpg"),
       ("Textures/skybox/back.jpg")
    };

    skyBoxMaterial->skyBoxTexture->LoadTexture(faces);

    GraphicsRender::GetInstance().SkyBoxModel = skyBoxModel;
}



void ApplicationRenderer::Start()
{


    sceneViewcamera->postprocessing->InitializePostProcessing();

    gameScenecamera->postprocessing->InitializePostProcessing();
   // gameScenecamera->isPostprocessing = true;

    renderTextureCamera->postprocessing->InitializePostProcessing();
    renderTextureCamera->isPostprocessing = true;
    renderTextureCamera->postprocessing->chromatic->isEnabled = true;
    renderTextureCamera->postprocessing->pixelization->isEnabled = false;

    renderTextureCamera2->postprocessing->InitializePostProcessing();
    renderTextureCamera2->isPostprocessing = true;

    renderTextureCamera2->postprocessing->chromatic->isEnabled = false;

    renderTextureCamera2->postprocessing->pixelization->isEnabled = true;


    LightRender();

    ModelRender();
}

void ApplicationRenderer::LightRender()
{
    Light* directionLight = new Light();
    directionLight->Initialize(LightType::DIRECTION_LIGHT, 1);
    directionLight->SetAmbientColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));

    directionLight->SetColor(glm::vec4(1, 1, 1, 1.0f));
    directionLight->SetAttenuation(1, 1, 0.01f);
    directionLight->SetInnerAndOuterCutoffAngle(11, 12);

    directionLight->transform.SetRotation(glm::vec3(0, 78, 0));
    directionLight->transform.SetPosition(glm::vec3(0, 0, 5));


}

void ApplicationRenderer::ModelRender()
{
   


    std::string texturePath = "Models/Scene/dungeon_texture.png";

    Texture* dungeonTexure = new Texture(texturePath);


    Model* floor1 = new Model("Models/Scene/floor_tile_small.fbx");

    for (int row = 1; row <15; row++)
    {
        for (size_t column = 1; column < 15; column++)
        {
            Model* floorCopy = new Model(*floor1);
            floorCopy->name = "Floor " + std::to_string(row);
            floorCopy->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
            floorCopy->transform.SetPosition(glm::vec3(2 * column, -1.10f, 2*row));
            floorCopy->transform.SetScale(glm::vec3(1));
            GraphicsRender::GetInstance().AddModelAndShader(floorCopy, defaultShader);
        }

    }

    Model* wall = new Model("Models/Scene/wall.fbx");

    int wallNumber = 0;
    for (int i = 1; i <8; ++i) 
    {
        Model* walCopy = new Model(*wall);
        walCopy->name = "Wall " + std::to_string(wallNumber);
        walCopy->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
        walCopy->transform.SetPosition(glm::vec3(3 + (i - 1) * 4, -1.10f, 1.5f));
        GraphicsRender::GetInstance().AddModelAndShader(walCopy, defaultShader);
        wallNumber++;
    }


    for (int i = 1; i < 8; ++i)
    {
        Model* walCopy = new Model(*wall);
        walCopy->name = "Wall " + std::to_string(wallNumber);
        walCopy->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
        walCopy->transform.SetPosition(glm::vec3(3 + (i - 1) * 4, -1.10f, 28.5f));
        GraphicsRender::GetInstance().AddModelAndShader(walCopy, defaultShader);

        wallNumber++;

    }


    Model* walCopy = new Model(*wall);
    walCopy->name = "Wall " + std::to_string(wallNumber++);
    walCopy->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    walCopy->transform.SetPosition(glm::vec3(1.50f, -1.10f, 9.00f));
    walCopy->transform.SetRotation(glm::vec3(0, 90.00f, 0));
    walCopy->transform.SetScale(glm::vec3(3.5f,1.0f,1.0f));
    GraphicsRender::GetInstance().AddModelAndShader(walCopy, defaultShader);

    Model* walCopy3 = new Model(*wall);
    walCopy3->name = "Wall " + std::to_string(wallNumber++);
    walCopy3->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    walCopy3->transform.SetPosition(glm::vec3(1.50f, -1.10f, 21.0f));
    walCopy3->transform.SetRotation(glm::vec3(0, 90.00f, 0));
    walCopy3->transform.SetScale(glm::vec3(3.5f, 1.0f, 1.0f));
    GraphicsRender::GetInstance().AddModelAndShader(walCopy3, defaultShader);

    Model* walCopy2 = new Model(*wall);
    walCopy2->name = "Wall " + std::to_string(wallNumber++);
    walCopy2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    walCopy2->transform.SetPosition(glm::vec3(16.50f, -1.10f, 9.00f));
    walCopy2->transform.SetRotation(glm::vec3(0, 90.00f, 0));
    walCopy2->transform.SetScale(glm::vec3(3.5f, 1.0f, 1.0f));
    GraphicsRender::GetInstance().AddModelAndShader(walCopy2, defaultShader);

    Model* walCopy6 = new Model(*wall);
    walCopy6->name = "Wall " + std::to_string(wallNumber++);
    walCopy6->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    walCopy6->transform.SetPosition(glm::vec3(16.50f, -1.10f, 21.0f));
    walCopy6->transform.SetRotation(glm::vec3(0, 90.00f, 0));
    walCopy6->transform.SetScale(glm::vec3(3.5f, 1.0f, 1.0f));
    GraphicsRender::GetInstance().AddModelAndShader(walCopy6, defaultShader);


    Model* walCopy4 = new Model(*wall);
    walCopy4->name = "Wall " + std::to_string(wallNumber++);
    walCopy4->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    walCopy4->transform.SetPosition(glm::vec3(28.5f, -1.10f, 9.00f));
    walCopy4->transform.SetRotation(glm::vec3(0, 90.00f, 0));
    walCopy4->transform.SetScale(glm::vec3(3.5f, 1.0f, 1.0f));
    GraphicsRender::GetInstance().AddModelAndShader(walCopy4, defaultShader);


    Model* walCopy5 = new Model(*wall);
    walCopy5->name = "Wall " + std::to_string(wallNumber++);
    walCopy5->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    walCopy5->transform.SetPosition(glm::vec3(28.5f, -1.10f, 21.0f));
    walCopy5->transform.SetRotation(glm::vec3(0, 90.00f, 0));
    walCopy5->transform.SetScale(glm::vec3(3.5f, 1.0f, 1.0f));
    GraphicsRender::GetInstance().AddModelAndShader(walCopy5, defaultShader);


    Model* tableLong = new Model("Models/Scene/table_long.fbx");

    tableLong->name = "Table ";
    tableLong->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    tableLong->transform.SetPosition(glm::vec3(13.00f, -1.10f, 10.00f));
    GraphicsRender::GetInstance().AddModelAndShader(tableLong, defaultShader);

    Model* chair = new Model("Models/Scene/chair.fbx");

    chair->name = "chair ";
    chair->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    chair->transform.SetPosition(glm::vec3(15.00f, -1.00f, 9.90f));

    GraphicsRender::GetInstance().AddModelAndShader(chair, defaultShader);

    Model* chair2 = new Model("Models/Scene/chair.fbx");

    chair2->name = "chair2 ";
    chair2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    chair2->transform.SetPosition(glm::vec3(24.50f, -1.00f, 6.40f));
    chair2->transform.SetRotation(glm::vec3(0, 132.50f, 0));

    GraphicsRender::GetInstance().AddModelAndShader(chair2, defaultShader);

    Model* chair3 = new Model("Models/Scene/chair.fbx");

    chair3->name = "chair3 ";
    chair3->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    chair3->transform.SetPosition(glm::vec3(24.60f, -1.00f, 9.90f));
    chair3->transform.SetRotation(glm::vec3(0, 194.50f, 0));

    GraphicsRender::GetInstance().AddModelAndShader(chair3, defaultShader);


    Model* crates_stacked = new Model("Models/Scene/crates_stacked.fbx");
    crates_stacked->name = "crates_stacked ";
    crates_stacked->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    crates_stacked->transform.SetPosition(glm::vec3(15.00f, -1.0f, 3.50f));

    GraphicsRender::GetInstance().AddModelAndShader(crates_stacked, defaultShader);


    Model* crates_stacked2 = new Model("Models/Scene/crates_stacked.fbx");
    crates_stacked2->name = "crates_stacked ";
    crates_stacked2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    crates_stacked2->transform.SetPosition(glm::vec3(4.10f, -1.0f, 14.20f));
    crates_stacked2->transform.SetRotation(glm::vec3(0,90,0));

    GraphicsRender::GetInstance().AddModelAndShader(crates_stacked2, defaultShader);


    Model* coin_stack_medium = new Model("Models/Scene/coin_stack_medium.fbx");

    coin_stack_medium->name = "chest_gold ";
    coin_stack_medium->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    coin_stack_medium->transform.SetPosition(glm::vec3(12.90f, -0.10f, 11.00f));

    GraphicsRender::GetInstance().AddModelAndShader(coin_stack_medium, defaultShader);



    Model* bottle_A_labeled_brown = new Model("Models/Scene/bottle_A_labeled_brown.fbx");

    bottle_A_labeled_brown->name = "bottle_A_labeled_brown ";
    bottle_A_labeled_brown->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    bottle_A_labeled_brown->transform.SetPosition(glm::vec3(12.50f, -0.10f, 9.00f));
    bottle_A_labeled_brown->meshes[0]->meshMaterial->material()->alphaTexture = dungeonTexure;
    bottle_A_labeled_brown->meshes[0]->meshMaterial->material()->useMaskTexture = true;
    bottle_A_labeled_brown->meshes[0]->meshMaterial->material()->SetBaseColor(glm::vec4(1,1,1, 0.4f));
    GraphicsRender::GetInstance().AddModelAndShader(bottle_A_labeled_brown, alphaBlendShader);

    Model* bottle_A_labeled_green = new Model("Models/Scene/bottle_A_labeled_green.fbx");

    bottle_A_labeled_green->name = "bottle_A_labeled_green ";
    bottle_A_labeled_green->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    bottle_A_labeled_green->transform.SetPosition(glm::vec3(13.50f, -0.10f, 9.50f));
    bottle_A_labeled_green->meshes[0]->meshMaterial->material()->alphaTexture = dungeonTexure;
    bottle_A_labeled_green->meshes[0]->meshMaterial->material()->useMaskTexture = true;
    bottle_A_labeled_green->meshes[0]->meshMaterial->material()->SetBaseColor(glm::vec4(1, 1, 1, 0.4f));
    GraphicsRender::GetInstance().AddModelAndShader(bottle_A_labeled_green, alphaBlendShader);
    

    Model* keyring = new Model("Models/Scene/keyring.fbx");
    keyring->name = "keyring ";
    keyring->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    keyring->transform.SetPosition(glm::vec3(13.30f, -0.10f, 8.70f));
    GraphicsRender::GetInstance().AddModelAndShader(keyring, defaultShader);

    Model* keg_decorated = new Model("Models/Scene/keg_decorated.fbx");
    keg_decorated->name = "keg_decorated ";
    keg_decorated->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    keg_decorated->transform.SetPosition(glm::vec3(4.00f, -1.00f, 4.50f));
    keg_decorated->transform.SetRotation(glm::vec3(0, 105.00f, 0));
    GraphicsRender::GetInstance().AddModelAndShader(keg_decorated, defaultShader);


    Model* sword_shield_gold = new Model("Models/Scene/sword_shield_gold.fbx");
    sword_shield_gold->name = "sword_shield_gold ";
    sword_shield_gold->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    sword_shield_gold->transform.SetPosition(glm::vec3(9.00f, 1.00f, 2.00f));
    GraphicsRender::GetInstance().AddModelAndShader(sword_shield_gold, defaultShader);

    Model* sword_shield_gold2 = new Model("Models/Scene/sword_shield_gold.fbx");
    sword_shield_gold2->name = "sword_shield_gold2 ";
    sword_shield_gold2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    sword_shield_gold2->transform.SetPosition(glm::vec3(23.00f, 1.00f, 28.00f));
    sword_shield_gold2->transform.SetRotation(glm::vec3(0, 180, 0));
    GraphicsRender::GetInstance().AddModelAndShader(sword_shield_gold2, defaultShader);



    Model* trunk_large_A = new Model("Models/Scene/trunk_large_A.fbx");
    trunk_large_A->name = "trunk_large_A ";
    trunk_large_A->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    trunk_large_A->transform.SetPosition(glm::vec3(27.00f, -1, 16.00f));
    trunk_large_A->transform.SetRotation(glm::vec3(0, 90.00, 0));
    GraphicsRender::GetInstance().AddModelAndShader(trunk_large_A, defaultShader);

    Model* trunk_large_A1 = new Model("Models/Scene/trunk_large_A.fbx");
    trunk_large_A1->name = "trunk_large_A1 ";
    trunk_large_A1->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    trunk_large_A1->transform.SetPosition(glm::vec3(27.00f, -1, 18.00f));
    trunk_large_A1->transform.SetRotation(glm::vec3(0, 90.00, 0));
    GraphicsRender::GetInstance().AddModelAndShader(trunk_large_A1, defaultShader);

    Model* trunk_large_A2 = new Model("Models/Scene/trunk_large_A.fbx");
    trunk_large_A2->name = "trunk_large_A2 ";
    trunk_large_A2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    trunk_large_A2->transform.SetPosition(glm::vec3(27.00f, -1, 20.0f));
    trunk_large_A2->transform.SetRotation(glm::vec3(0, 90.00, 0));
    GraphicsRender::GetInstance().AddModelAndShader(trunk_large_A2, defaultShader);


    Model* table_medium_tablecloth_decorated_B = new Model("Models/Scene/table_medium_tablecloth_decorated_B.fbx");
    table_medium_tablecloth_decorated_B->name = "table_medium_tablecloth_decorated_B ";
    table_medium_tablecloth_decorated_B->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    table_medium_tablecloth_decorated_B->transform.SetPosition(glm::vec3(26.20f, -1, 10.20f));
    GraphicsRender::GetInstance().AddModelAndShader(table_medium_tablecloth_decorated_B, defaultShader);
    

    Model* table_long_decorated_A = new Model("Models/Scene/table_long_decorated_A.fbx");
    table_long_decorated_A->name = "table_long_decorated_A ";
    table_long_decorated_A->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    table_long_decorated_A->transform.SetPosition(glm::vec3(25.90f, -1, 5.80f));
    GraphicsRender::GetInstance().AddModelAndShader(table_long_decorated_A, defaultShader);

    Model* stairs_wood_decorated = new Model("Models/Scene/stairs_wood_decorated.fbx");
    table_long_decorated_A->name = "stairs_wood_decorated ";
    stairs_wood_decorated->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    stairs_wood_decorated->transform.SetPosition(glm::vec3(18.60f, -1, 1.90f));
    GraphicsRender::GetInstance().AddModelAndShader(stairs_wood_decorated, defaultShader);

    Model* pillar_decorated = new Model("Models/Scene/pillar_decorated.fbx");
    pillar_decorated->name = "pillar_decorated ";
    pillar_decorated->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    pillar_decorated->transform.SetPosition(glm::vec3(8.80f, -1, 17.70f));
    pillar_decorated->transform.SetRotation(glm::vec3(0.00, 180.0f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(pillar_decorated, defaultShader);


    Model* pillar_decorated2 = new Model("Models/Scene/pillar_decorated.fbx");
    pillar_decorated2->name = "pillar_decorated ";
    pillar_decorated2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    pillar_decorated2->transform.SetPosition(glm::vec3(21.00f, -1, 17.70f));
    pillar_decorated2->transform.SetRotation(glm::vec3(0.00, 180.0f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(pillar_decorated2, defaultShader);

#pragma region Lights



    Model* torch_mounted1 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted1->name = "torch_mounted1 ";
    torch_mounted1->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted1->transform.SetPosition(glm::vec3(16.90f, 1.30f, 17.70f));
    torch_mounted1->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted1, defaultShader);

    Light* pointLight1 = new Light();
    pointLight1->Initialize(POINT_LIGHT, 0.75f);
    pointLight1->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f,1));
    pointLight1->SetAttenuation(0.5f,1,0.5f);
    pointLight1->SetInnerAndOuterCutoffAngle(10,15);
    pointLight1->SetColor(1,0,0,1);
    pointLight1->transform.SetPosition(glm::vec3(16.90f, 1.750f, 17.70f));
    pointLight1->isVisible = false;



    Model* torch_mounted2 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted2->name = "torch_mounted2 ";
    torch_mounted2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted2->transform.SetPosition(glm::vec3(16.90f, 1.30f, 20.50));
    torch_mounted2->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted2, defaultShader);


    Light* pointLight2 = new Light();
    pointLight2->Initialize(POINT_LIGHT, 0.75f);
    pointLight2->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight2->SetAttenuation(0.5f, 1, 0.5f);
    pointLight2->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight2->SetColor(1, 0, 0, 1);
    pointLight2->transform.SetPosition(glm::vec3(16.90f, 1.750f, 20.50));
    pointLight2->isVisible = false;

    Model* torch_mounted3 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted3->name = "torch_mounted3 ";
    torch_mounted3->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted3->transform.SetPosition(glm::vec3(16.90f, 1.30f, 23.50));
    torch_mounted3->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted3, defaultShader);


    Light* pointLight3 = new Light();
    pointLight3->Initialize(POINT_LIGHT, 0.75f);
    pointLight3->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight3->SetAttenuation(0.5f, 1, 0.5f);
    pointLight3->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight3->SetColor(1, 0, 0, 1);
    pointLight3->transform.SetPosition(glm::vec3(16.90f, 1.750f, 23.50));
    pointLight3->isVisible = false;


    Model* torch_mounted4 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted4->name = "torch_mounted4 ";
    torch_mounted4->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted4->transform.SetPosition(glm::vec3(16.90f, 1.30f, 13.50f));
    torch_mounted4->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted4, defaultShader);


    Light* pointLight4 = new Light();
    pointLight4->name = "pointLight4";

    pointLight4->Initialize(POINT_LIGHT, 0.75f);
    pointLight4->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight4->SetAttenuation(0.5f, 1, 0.5f);
    pointLight4->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight4->SetColor(1, 0, 0, 1);
    pointLight4->transform.SetPosition(glm::vec3(16.90f, 1.750f, 13.50f));
    pointLight4->isVisible = false;

    Model* torch_mounted5 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted5->name = "torch_mounted5 ";
    torch_mounted5->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted5->transform.SetPosition(glm::vec3(1.75f, 1.30f, 13.50f));
    torch_mounted5->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted5, defaultShader);


    Light* pointLight5 = new Light();
    pointLight5->name = "Point light5";
    pointLight5->Initialize(POINT_LIGHT, 0.75f);
    pointLight5->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight5->SetAttenuation(0.5f, 1, 0.5f);
    pointLight5->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight5->SetColor(1, 0, 0, 1);
    pointLight5->transform.SetPosition(glm::vec3(2, 1.750f, 13.50f));
    pointLight5->isVisible = false;


    Model* torch_mounted6 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted6->name = "torch_mounted6 ";
    torch_mounted6->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted6->transform.SetPosition(glm::vec3(1.75f, 1.30f, 23.50f));
    torch_mounted6->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted6, defaultShader);

    Light* pointLight6 = new Light();
    pointLight6->name = "Point light6";

    pointLight6->Initialize(POINT_LIGHT, 0.75f);
    pointLight6->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight6->SetAttenuation(0.5f, 1, 0.5f);
    pointLight6->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight6->SetColor(1, 0, 0, 1);
    pointLight6->transform.SetPosition(glm::vec3(2, 1.750f, 23.50f));
    pointLight6->isVisible = false;

    Model* torch_mounted7 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted7->name = "torch_mounted7 ";
    torch_mounted7->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted7->transform.SetPosition(glm::vec3(1.75f, 1.30f, 17.70f));
    torch_mounted7->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted7, defaultShader);


    Light* pointLight7 = new Light();
    pointLight7->Initialize(POINT_LIGHT, 0.75f);
    pointLight7->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight7->SetAttenuation(0.5f, 1, 0.5f);
    pointLight7->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight7->SetColor(1, 0, 0, 1);
    pointLight7->transform.SetPosition(glm::vec3(2, 1.750f, 17.70f));
    pointLight7->isVisible = false;


    Model* torch_mounted8 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted8->name = "torch_mounted8 ";
    torch_mounted8->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted8->transform.SetPosition(glm::vec3(1.75f, 1.30f, 20.50f));
    torch_mounted8->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted8, defaultShader);

    Light* pointLight8 = new Light();
    pointLight8->Initialize(POINT_LIGHT, 0.75f);
    pointLight8->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight8->SetAttenuation(0.5f, 1, 0.5f);
    pointLight8->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight8->SetColor(1, 0, 0, 1);
    pointLight8->transform.SetPosition(glm::vec3(2, 1.750f, 20.50f));
    pointLight8->isVisible = false;

    Model* torch_mounted9 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted9->name = "torch_mounted9 ";
    torch_mounted9->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted9->transform.SetPosition(glm::vec3(1.75f, 1.30f, 9.10f));
    torch_mounted9->transform.SetRotation(glm::vec3(0.00, 90.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted9, defaultShader);

    Light* pointLight9 = new Light();
    pointLight9->Initialize(POINT_LIGHT, 0.75f);
    pointLight9->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight9->SetAttenuation(0.5f, 1, 0.5f);
    pointLight9->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight9->SetColor(1, 0, 0, 1);
    pointLight9->transform.SetPosition(glm::vec3(2, 1.750f, 9.10f));
    pointLight9->isVisible = false;


    Model* torch_mounted10 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted10->name = "torch_mounted10 ";
    torch_mounted10->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted10->transform.SetPosition(glm::vec3(13.45f, 1.30f, 28.00f));
    torch_mounted10->transform.SetRotation(glm::vec3(0.00, 180.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted10, defaultShader);

    Light* pointLight10 = new Light();
    pointLight10->Initialize(POINT_LIGHT, 0.75f);
    pointLight10->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight10->SetAttenuation(0.5f, 1, 0.5f);
    pointLight10->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight10->SetColor(1, 0, 0, 1);
    pointLight10->transform.SetPosition(glm::vec3(13.45f, 1.750f, 28.00f));
    pointLight10->isVisible = false;

    Model* torch_mounted11 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted11->name = "torch_mounted11 ";
    torch_mounted11->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted11->transform.SetPosition(glm::vec3(9.25f, 1.30f, 28.00f));
    torch_mounted11->transform.SetRotation(glm::vec3(0.00, 180.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted11, defaultShader);



    Light* pointLight11 = new Light();
    pointLight11->Initialize(POINT_LIGHT, 0.75f);
    pointLight11->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight11->SetAttenuation(0.5f, 1, 0.5f);
    pointLight11->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight11->SetColor(1, 0, 0, 1);
    pointLight11->transform.SetPosition(glm::vec3(9.25f, 1.750f, 28.00f));
    pointLight11->isVisible = false;


    Model* torch_mounted12 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted12->name = "torch_mounted12 ";
    torch_mounted12->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted12->transform.SetPosition(glm::vec3(4.45f, 1.30f, 28.00f));
    torch_mounted12->transform.SetRotation(glm::vec3(0.00, 180.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted12, defaultShader);

    Light* pointLight12 = new Light();
    pointLight12->Initialize(POINT_LIGHT, 0.75f);
    pointLight12->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight12->SetAttenuation(0.5f, 1, 0.5f);
    pointLight12->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight12->SetColor(1, 0, 0, 1);
    pointLight12->transform.SetPosition(glm::vec3(4.45f, 1.750f, 28.00f));
    pointLight12->isVisible = false;


    Model* torch_mounted13 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted13->name = "torch_mounted13 ";
    torch_mounted13->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted13->transform.SetPosition(glm::vec3(6.05f, 1.20f, 1.80f));
    torch_mounted13->transform.SetRotation(glm::vec3(0.00, 0, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted13, defaultShader);

    Light* pointLight13 = new Light();
    pointLight13->Initialize(POINT_LIGHT, 0.75f);
    pointLight13->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight13->SetAttenuation(0.5f, 1, 0.5f);
    pointLight13->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight13->SetColor(1, 0, 0, 1);
    pointLight13->transform.SetPosition(glm::vec3(6.05f, 1.650f, 1.80f));
    pointLight13->isVisible = false;


    Model* torch_mounted14 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted14->name = "torch_mounted14 ";
    torch_mounted14->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted14->transform.SetPosition(glm::vec3(12.10f, 1.20f, 1.80f));
    torch_mounted14->transform.SetRotation(glm::vec3(0.00, 0, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted14, defaultShader);

    Light* pointLight14 = new Light();
    pointLight14->Initialize(POINT_LIGHT, 0.75f);
    pointLight14->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight14->SetAttenuation(0.5f, 1, 0.5f);
    pointLight14->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight14->SetColor(1, 0, 0, 1);
    pointLight14->transform.SetPosition(glm::vec3(12.10f, 1.650f, 1.80f));
    pointLight14->isVisible = false;

    Model* torch_mounted15 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted15->name = "torch_mounted15 ";
    torch_mounted15->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted15->transform.SetPosition(glm::vec3(20.00f, 1.20f, 28.30f));
    torch_mounted15->transform.SetRotation(glm::vec3(0.00, 180.00f, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted15, defaultShader);

    Light* pointLight15 = new Light();
    pointLight15->Initialize(POINT_LIGHT, 0.75f);
    pointLight15->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight15->SetAttenuation(0.5f, 1, 0.5f);
    pointLight15->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight15->SetColor(1, 0, 0, 1);
    pointLight15->transform.SetPosition(glm::vec3(20.00f, 1.650f, 28.30f));
    pointLight15->isVisible = false;


    Model* torch_mounted16 = new Model("Models/Scene/torch_mounted.fbx");
    torch_mounted16->name = "torch_mounted16 ";
    torch_mounted16->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    torch_mounted16->transform.SetPosition(glm::vec3(26.10f, 1.20f, 28.30f));
    torch_mounted16->transform.SetRotation(glm::vec3(0.00, 180.00, 0.00f));
    GraphicsRender::GetInstance().AddModelAndShader(torch_mounted16, defaultShader);

    Light* pointLight16 = new Light();
    pointLight16->Initialize(POINT_LIGHT, 0.75f);
    pointLight16->SetAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1));
    pointLight16->SetAttenuation(0.5f, 1, 0.5f);
    pointLight16->SetInnerAndOuterCutoffAngle(10, 15);
    pointLight16->SetColor(1, 0, 0, 1);
    pointLight16->transform.SetPosition(glm::vec3(26.10f, 1.650f, 28.30f));
    pointLight16->isVisible = false;


    /*Model* sword_shield_gold2 = new Model("Models/Scene/sword_shield_gold.fbx");
    sword_shield_gold2->name = "sword_shield_gold2 ";
    sword_shield_gold2->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    sword_shield_gold2->transform.SetPosition(glm::vec3(23.00f, 1.00f, 28.00f));
    sword_shield_gold2->transform.SetRotation(glm::vec3(0, 180, 0));
    GraphicsRender::GetInstance().AddModelAndShader(sword_shield_gold2, defaultShader);*/



    Light* spotLight1 = new Light();
    spotLight1->Initialize(SPOT_LIGHT, 0.75f);
    spotLight1->SetAmbientColor(glm::vec4(0.4f, 0.4f, 0.4f, 1));
    spotLight1->SetAttenuation(0.5f, 0.5f, 0.5f);
    spotLight1->SetInnerAndOuterCutoffAngle(20, 25);
    spotLight1->SetColor(10, 4, 0, 1);
    spotLight1->transform.SetPosition(glm::vec3(23.00f, 3, 28.00f));
    spotLight1->transform.SetRotation(glm::vec3(-90, -90, 0));
    spotLight1->isVisible = false;


    //Model* sword_shield_gold = new Model("Models/Scene/sword_shield_gold.fbx");
    //sword_shield_gold->name = "sword_shield_gold ";
    //sword_shield_gold->meshes[0]->meshMaterial->material()->diffuseTexture = dungeonTexure;
    //sword_shield_gold->transform.SetPosition(glm::vec3(9.00f, 1.00f, 2.00f));
    //GraphicsRender::GetInstance().AddModelAndShader(sword_shield_gold, defaultShader);

    Light* spotLight2 = new Light();
    spotLight2->Initialize(SPOT_LIGHT, 0.75f);
    spotLight2->SetAmbientColor(glm::vec4(0.4f, 0.4f, 0.4f, 1));
    spotLight2->SetAttenuation(0.5f, 0.5f, 0.5f);
    spotLight2->SetInnerAndOuterCutoffAngle(20, 25);
    spotLight2->SetColor(10, 4, 0, 1);
    spotLight2->transform.SetPosition(glm::vec3(9.00f, 3, 2.00f));
    spotLight2->transform.SetRotation(glm::vec3(-90, -90, 0));
    spotLight2->isVisible = false;

#pragma endregion

    Model* interiorAsset1 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-0.obj", true);
    Model* interiorAsset2 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-1.obj", true);
    Model* interiorAsset3 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-2.obj", true);
    Model* interiorAsset4 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-3.obj", true);
    Model* interiorAsset5 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-4.obj", true);
    Model* interiorAsset6 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-5.obj", true);
    Model* interiorAsset7 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-6.obj", true);
    Model* interiorAsset8 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-7.obj", true);
    Model* interiorAsset9 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-8.obj", true);
    Model* interiorAsset10 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-9.obj", true);
    Model* interiorAsset11 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-10.obj", true);
    Model* interiorAsset12 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-11.obj", true);
    Model* interiorAsset13 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-12.obj", true);
    Model* interiorAsset14 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-13.obj", true);
    Model* interiorAsset15 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-14.obj", true);
    Model* interiorAsset16 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-15.obj", true);
    Model* interiorAsset17 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-16.obj", true);
    Model* interiorAsset18 = new Model("Models/Scene/Free Sample/House Interior Rooms - Free Sample-17.obj", true);

    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset1, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset2, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset3, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset4, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset5, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset6, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset7, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset8, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset9, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset10, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset11, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset12, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset14, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset15, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset16, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset17, defaultShader);
    GraphicsRender::GetInstance().AddModelAndShader(interiorAsset18, defaultShader);


    Model* TV = new Model("Models/Scene/TV/TV.obj", true);
    TV->transform.SetPosition(glm::vec3(-0.30, 0.34f, -0.80f));
    TV->transform.SetRotation(glm::vec3(0, -90, 0));
    TV->transform.SetScale(glm::vec3(5.00));
    GraphicsRender::GetInstance().AddModelAndShader(TV, defaultShader);


    Model* TV2 = new Model("Models/Scene/TV/TV.obj", true);
    TV2->transform.SetPosition(glm::vec3(0.25f, 0.58f, -0.80f));
    TV2->transform.SetRotation(glm::vec3(0, -90, 0));
    TV2->transform.SetScale(glm::vec3(5.00));
    GraphicsRender::GetInstance().AddModelAndShader(TV2, defaultShader);

     Model* quadWithTexture = new Model("Models/DefaultQuad/DefaultQuad.fbx");
    quadWithTexture->transform.SetPosition(glm::vec3(-0.31f, 0.42f, -0.72f));
    quadWithTexture->transform.SetScale(glm::vec3(0.08f, -0.06, 1.0f));

    quadWithTexture->meshes[0]->meshMaterial->material()->diffuseTexture = renderTextureCamera->renderTexture;

    GraphicsRender::GetInstance().AddModelAndShader(quadWithTexture, defaultShader);

    Model* quadWithTexture2 = new Model("Models/DefaultQuad/DefaultQuad.fbx");
    quadWithTexture2->transform.SetPosition(glm::vec3(0.24f, 0.658f, -0.72f));
    quadWithTexture2->transform.SetScale(glm::vec3(0.08f, -0.06, 1.0f));

    quadWithTexture2->meshes[0]->meshMaterial->material()->diffuseTexture = renderTextureCamera2->renderTexture;

    GraphicsRender::GetInstance().AddModelAndShader(quadWithTexture2, defaultShader);





}


void ApplicationRenderer::PreRender()
{
    projection = sceneViewcamera->GetProjectionMatrix();

    view = sceneViewcamera->GetViewMatrix();

    skyBoxView = glm::mat4(glm::mat3(sceneViewcamera->GetViewMatrix()));
  

    defaultShader->Bind();
    LightManager::GetInstance().UpdateUniformValuesToShader(defaultShader);

    defaultShader->setMat4("projection", projection);
    defaultShader->setMat4("view", view);
    defaultShader->setVec3("viewPos", sceneViewcamera->transform.position.x, sceneViewcamera->transform.position.y, sceneViewcamera->transform.position.z);
    defaultShader->setFloat("time", scrollTime);
    defaultShader->setBool("isDepthBuffer", false);

    alphaBlendShader->Bind();
    LightManager::GetInstance().UpdateUniformValuesToShader(alphaBlendShader);
    alphaBlendShader->setMat4("projection", projection);
    alphaBlendShader->setMat4("view", view);
    alphaBlendShader->setVec3("viewPos", sceneViewcamera->transform.position.x, sceneViewcamera->transform.position.y, sceneViewcamera->transform.position.z);
    alphaBlendShader->setFloat("time", scrollTime);
    alphaBlendShader->setBool("isDepthBuffer", false);

    alphaCutoutShader->Bind();
    LightManager::GetInstance().UpdateUniformValuesToShader(alphaCutoutShader);
    alphaCutoutShader->setMat4("projection", projection);
    alphaCutoutShader->setMat4("view", view);
    alphaCutoutShader->setVec3("viewPos", sceneViewcamera->transform.position.x, sceneViewcamera->transform.position.y, sceneViewcamera->transform.position.z);
    alphaCutoutShader->setFloat("time", scrollTime);
    alphaCutoutShader->setBool("isDepthBuffer", false);

    solidColorShader->Bind();
    solidColorShader->setMat4("projection", projection);
    solidColorShader->setMat4("view", view);

    stencilShader->Bind();
    stencilShader->setMat4("projection", projection);
    stencilShader->setMat4("view", view);

    glDepthFunc(GL_LEQUAL);
    skyboxShader->Bind();
    skyboxShader->setMat4("projection", projection);
    skyboxShader->setMat4("view", skyBoxView);

    GraphicsRender::GetInstance().SkyBoxModel->Draw(*skyboxShader);
    glDepthFunc(GL_LESS);


    /* ScrollShader->Bind();
       ScrollShader->setMat4("ProjectionMatrix", _projection);*/

}

void ApplicationRenderer::Render()
{
   
    Start();
  
    EditorLayout::GetInstance().InitializeEditors();

    Time::GetInstance().lastFrame = glfwGetTime();
   // glEnable(GL_BLEND);
  //  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
    while (!glfwWindowShouldClose(window))
    {

        Time::GetInstance().SetCurrentTime(glfwGetTime());
       
      
       // scrollTime += Time::GetInstance().deltaTime;

        EngineGameLoop();

        EngineGraphicsRender();


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void ApplicationRenderer::EngineGraphicsRender()
{

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    if (isImguiPanelsEnable)
    {
        PanelManager::GetInstance().Update((float)windowWidth, (float)WindowHeight);
    }

   


    /*sceneViewframeBuffer->Bind();

    GraphicsRender::GetInstance().Clear();
    PreRender();
    GraphicsRender::GetInstance().Draw();

    sceneViewframeBuffer->Unbind();*/
    RenderForCamera(sceneViewcamera, sceneViewframeBuffer);


  /*  RenderForCamera(gameScenecamera, gameframeBuffer);

    RenderForCamera(renderTextureCamera, renderTextureCamera->renderTexture->framebuffer);*/


    for (Camera* camera :  CameraManager::GetInstance().GetCameras())
    {
        if (camera->renderTexture == nullptr)
        {
            RenderForCamera(camera, gameframeBuffer);                  // GAME SCENE CAMERA

           
        }
        else
        {
            RenderForCamera(camera, camera->renderTexture->framebuffer); 
        }
       
    }

    //gameframeBuffer->Bind();
    //GraphicsRender::GetInstance().Clear();
    //PreRender();
    //GraphicsRender::GetInstance().Draw();

    //gameframeBuffer->Unbind();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void ApplicationRenderer::EngineGameLoop()
{
    ProcessInput(window);

    if (isPlayMode)
    {
        EntityManager::GetInstance().Update(Time::GetInstance().deltaTime);
    }

    PostRender();
}
void ApplicationRenderer::RenderForCamera(Camera* camera, FrameBuffer* framebuffer)
{

  /*  sceneViewframeBuffer->Bind();

    GraphicsRender::GetInstance().Clear();
    PreRender();
    GraphicsRender::GetInstance().Draw();*/


    framebuffer->Bind();

    GraphicsRender::GetInstance().Clear();

    projection = camera->GetProjectionMatrix();

    view = camera->GetViewMatrix();

    skyBoxView = glm::mat4(glm::mat3(camera->GetViewMatrix()));


    defaultShader->Bind();
    LightManager::GetInstance().UpdateUniformValuesToShader(defaultShader);

    defaultShader->setMat4("projection", projection);
    defaultShader->setMat4("view", view);
    defaultShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    defaultShader->setFloat("time", scrollTime);
    defaultShader->setBool("isDepthBuffer", false);

    alphaBlendShader->Bind();
    LightManager::GetInstance().UpdateUniformValuesToShader(alphaBlendShader);
    alphaBlendShader->setMat4("projection", projection);
    alphaBlendShader->setMat4("view", view);
    alphaBlendShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    alphaBlendShader->setFloat("time", scrollTime);
    alphaBlendShader->setBool("isDepthBuffer", false);

    alphaCutoutShader->Bind();
    LightManager::GetInstance().UpdateUniformValuesToShader(alphaCutoutShader);
    alphaCutoutShader->setMat4("projection", projection);
    alphaCutoutShader->setMat4("view", view);
    alphaCutoutShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    alphaCutoutShader->setFloat("time", scrollTime);
    alphaCutoutShader->setBool("isDepthBuffer", false);

    solidColorShader->Bind();
    solidColorShader->setMat4("projection", projection);
    solidColorShader->setMat4("view", view);

    stencilShader->Bind();
    stencilShader->setMat4("projection", projection);
    stencilShader->setMat4("view", view);

    glDepthFunc(GL_LEQUAL);
    skyboxShader->Bind();
    skyboxShader->setMat4("projection", projection);
    skyboxShader->setMat4("view", skyBoxView);

    GraphicsRender::GetInstance().SkyBoxModel->Draw(*skyboxShader);
    glDepthFunc(GL_LESS);

    
    /* ScrollShader->Bind();
       ScrollShader->setMat4("ProjectionMatrix", _projection);*/

    GraphicsRender::GetInstance().Draw();

    framebuffer->Unbind();

    if (camera->isPostprocessing)
    {
       // if (camera->postprocessing.isPostProccesingEnabled)
        {
            camera->postprocessing->ApplyPostprocessing(framebuffer);
        }
    }



}
void ApplicationRenderer::PostRender()
{
   // glDisable(GL_BLEND);

}

void ApplicationRenderer::Clear()
{
    GLCALL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
   //glStencilMask(0x00);
}


void ApplicationRenderer::ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed=10;

    if (EditorLayout::GetInstance().IsViewportHovered() && (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS))
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(FORWARD, Time::GetInstance().deltaTime * cameraSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(BACKWARD, Time::GetInstance().deltaTime * cameraSpeed);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(LEFT, Time::GetInstance().deltaTime * cameraSpeed);

        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(RIGHT, Time::GetInstance().deltaTime * cameraSpeed);

        }
    }

}


 void ApplicationRenderer::SetViewPort(GLFWwindow* window, int width, int height)
{
 //   glViewport(0, 0, width, height);
}

 void ApplicationRenderer::KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
 {  
         if (key == GLFW_KEY_V && action == GLFW_PRESS)
         {

            
             std::cout << "V pressed" << std::endl;

             std::vector<Model*> listOfModels = GraphicsRender::GetInstance().GetModelList();
            
          

             selectedModelCount++;

             if (selectedModelCount > listOfModels.size()-1)
             {
                 selectedModelCount = 0;
             }

            
             GraphicsRender::GetInstance().selectedModel = listOfModels[selectedModelCount];


         }
     
         if (action == GLFW_PRESS)
         {
             InputManager::GetInstance().OnKeyPressed(key);
         }
         else if(action == GLFW_RELEASE)
         {
             InputManager::GetInstance().OnKeyReleased(key);
         }
         else if (action == GLFW_REPEAT)
         {
             InputManager::GetInstance().OnkeyHold(key);
         }
     
 }

 void ApplicationRenderer::MouseCallBack(GLFWwindow* window, double xposIn, double yposIn)
 {

    float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);
     
        if (firstMouse)
        {

        }

         if (firstMouse)
         {
             lastX = xpos;
             lastY = ypos;
             firstMouse = false;
         }
     
         float xoffset = xpos - lastX;
         float yoffset = lastY - ypos;
     
         lastX = xpos;
         lastY = ypos;
     
         if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS )&& EditorLayout::GetInstance().IsViewportHovered())
         {
             sceneViewcamera->ProcessMouseMovement(xoffset, yoffset);
         }
 }

 void ApplicationRenderer::MouseScroll(GLFWwindow* window, double xoffset, double yoffset)
 {
     sceneViewcamera->ProcessMouseScroll(static_cast<float>(yoffset));
 }
