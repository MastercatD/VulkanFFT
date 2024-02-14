//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm>
#include <fstream>

//Ширина и высота окна
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
//Количество одновременно обраматываемых кадров
const int MAX_FRAMES_IN_FLIGHT = 2;




//Перечисление слоёв валидации, которые мы хотим подключить
const std::vector <const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

//Разрешение подключение слоёв валидации
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//Вспомогательная функция для загрузки бинарных файлов
static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary); //Открытие и перемещение указателя на конец файла

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    //Выделения памяти для буфера
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    //Считывание всех байт
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    //Закрытие файла и возврат в буфер
    file.close();
    return buffer;
}

//Создание объекта VkDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//Уничтожение VkDebugUtilsMessengerEXT
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}




class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    VkInstance instance;    //Дескриптор экземпляра для инициализации библиотеки Vulkan
    VkDebugUtilsMessengerEXT debugMessenger;    //Дескриптор callback функции
    GLFWwindow* window;   //Окно  
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;   //Дескриптор видеокарты
    VkDevice device;    //Дескриптор логического устройства
    VkQueue graphicsQueue;  //Дескриптор очереди графических команд
    VkSurfaceKHR surface;   //Абстрактный тип поверхности для показа отрендеренных изображений
    VkQueue presentQueue;   //Дескриптор очереди
    VkSwapchainKHR swapChain;   //Дескриптор swapchain
    std::vector<VkImage> swapChainImages;   //дескрипторы VkImages
    VkFormat swapChainImageFormat;  //Формат изображений
    VkExtent2D swapChainExtent; //Разрешение изображений
    std::vector<VkImageView> swapChainImageViews;   //Описание image
    VkRenderPass renderPass;    //Объект прохода рендера
    VkPipelineLayout pipelineLayout;    //uniform глобальные переменные
    VkPipeline graphicsPipeline;    //Графический конвейер
    std::vector<VkFramebuffer> swapChainFramebuffers;   //Фреймбуфер
    VkCommandPool commandPool;  //Пул команд
    std::vector<VkCommandBuffer> commandBuffers;    //Буфер команд
    std::vector<VkSemaphore> imageAvailableSemaphores;    //Список семафор, image готов к рендерингу
    std::vector<VkSemaphore> renderFinishedSemaphores;    //Список семафор, рендеринг image закончен
    std::vector<VkFence> inFlightFences;    //Барьер
    std::vector<VkFence> imagesInFlight;    //Image в обработке
    size_t currentFrame = 0;    //Индекс текущего кадра


    //Список требуемых расширений
    const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME //Очередь изображений
    };
   
    //Индексы очередей
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value();
        }
    };
    //Свойства для создания swap chain
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    //Создание окна
    void initWindow() {
        glfwInit();         //Инициализация библиотеки GLFW
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        
    }



    //Настройка и инициализация Vulkan
    void initVulkan() {
        createInstance();   //Создание дескриптора экземпляра Vulkan API
        setupDebugMessenger();  //Установка callback функции
        createSurface();    //Создание surface
        pickPhysicalDevice();   //Выбор физического устройства
        createLogicalDevice();  //Создание логического устройства
        createSwapChain();  //Создание swapchain
        createImageViews(); //Создание Image view
        createRenderPass(); //Создания обхода рендера
        createGraphicsPipeline();   //Создание конвеера
        createFramebuffers();    //Создание фреймбуфера
        createCommandPool();    //Создание пула команд
        createCommandBuffers();  //Создание буфера команд
        createSyncObjects(); //Создание барьеров и семафоров для синхронизации
    }
    //Синхронизация
    void createSyncObjects() {
        //Размер списка семафор и барьеров
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);  //Семафор
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);    //Барьер
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);  //Image в обработке

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        //Создание семафоров и барьеров
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    //Буфер команд
    void createCommandBuffers() {
        commandBuffers.resize(swapChainFramebuffers.size());    //Выделение места

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        //Создание буфера команд
        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");    
        }
        //Запись в буферы команд
        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional
            //Начало записи буфера команд
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            //Начало прохода рендера
            VkRenderPassBeginInfo renderPassInfo{}; //Параметры прохода рендера
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];  //Фреймбуфер
            //Область рендеринга
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;
            //Параметры очистки буфера
            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;
            //Начало прохода рендера
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            //Рисование
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);    //Подключение графического конвейера
            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);   //Рисование треугольника
            vkCmdEndRenderPass(commandBuffers[i]);  //Конец прохода рендера
            //Завершение записи буфера команд
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
           
        }


    }

    //Пул команд
    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = 0; // Optional
        //Создание пула команд
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    //Создание фреймбуфера
    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());   //Выделение места в контейнере
        //Обход всех swap chain и создание в них фреймбуферов
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }

    }

    //Создание конвейера 
    void createGraphicsPipeline() {
        //Загружаем байт код шейдеров
        auto vertShaderCode = readFile("shaders/vert.spv"); //Шейдер вершин
        auto fragShaderCode = readFile("shaders/frag.spv"); //Фрагментный шейдер
        //Создаём шейдерные модули
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
        //Структура с информацией для стадии шейдера
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //Стадия конвейера
        //Указание шейдерного модуля
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        vertShaderStageInfo.pSpecializationInfo = nullptr;

        //Аналогично для фрагментного шейдера
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        fragShaderStageInfo.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };  //Массив с созданными структурами

        
        //Непрограммируемые этапы
        //Входные данные вершин
        //Описание формата вершин
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        //Выбор геометрии
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        //Viewport, растягивание изображения
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        //Scissor, отсечение изображения
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        
        //Объединение информации о Viewport и Scissor
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        //Растеризатор
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;  //Стадию растеризации можно отключить
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  //Способ генерации фрагментов
        rasterizer.lineWidth = 1.0f;    //Толщина отрезков
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;    //Тип сечения
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //Порядок обхода вершин
        //Параметры глубины, выключены
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        //Мультисэмплинг - сглаживание, выключен
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        //Смешивание цветов, выключено
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        //Динамическое состояние
        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        //Глобальные переменные для шейдеров
        VkPipelineLayout pipelineLayout;

        //Константы для шейдеров
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        //Объединение объектов и структур, создание графического конвейера
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        //Шейдерные модули
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        //Непрограммируемые этапы
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        //Проход
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        //
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        //Создание конвейера
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        VkPipeline graphicsPipeline;
        //Уничтожение шейдерных модулей
        //vkDestroyShaderModule(device, fragShaderModule, nullptr);   //Уничтожение шейдерных модулей 
        //vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    //Обход рендера
    void createRenderPass() {
        //Настройка буферов
        //Цветовой буфер
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;  //Соответствие формату из swap chain
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        //Данные в буфере цвета и глубины
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   //Очищается в проходе рендера, изменить
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Сохраняется в память для дальнейшего рендера
        //Данные в буфере трафарета, не используем
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //Текстуры и фреймбуферы
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  //До рендера
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  //После рендера

        //Подпроход
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        //Проход рендера
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        //Зависимости подпроходов
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }



    }

    //Создание шейдерного модуля
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        //Создание информации о шейдерном модуле
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        //Преобразования указателя байт кода
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;    //Шейдерный модуль
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        //Нужно освободить буфер code
        return shaderModule;
    }

    //Создание Image view
    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size()); //Выделение места в контейнере
        //Обход всех image в swap chain
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{}; //Параметры для создания шьфпу
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            //Интерпритация image
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            //Цветовые каналы
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            //Описание используемой части image
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            //Создание image view
            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    //Создание swapchain
    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        //Минимальное и маскимальное количество объектов image в swapchain
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;  
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};  //Структура с информацией для создания
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;   //surface к которому привязан swapchain
        //Информация для создания image объектов
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;    //Число слоёв
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    //Для каких операций будут использоваться image
        //Обработка image, используемых в разных очередях
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        
        if (indices.graphicsFamily != indices.presentFamily) {
            //VK_SHARING_MODE_CONCURRENT: объекты могут использоваться в нескольких семействах очередей без явной передачи права владения.
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            //VK_SHARING_MODE_EXCLUSIVE: объект принадлежит одному семейству очередей
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;   //Отсутствие преобразований изображения
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  //Игнорирование альфа-канала
        createInfo.presentMode = presentMode; 
        createInfo.clipped = VK_TRUE;   //Включённый clipping
        createInfo.oldSwapchain = VK_NULL_HANDLE;   //Старый swapchain, необходим для изменения размеров окна
    
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }
        //Получение дескрипторов изображений
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
        //Сохранение формата и разрешения изображений
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    //Создание surface
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
   
    //Заполнение структуры SwapChainSupportDetails
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);  //surface требования
        //Список поддерживаемых форматов surface
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        //Поддерживаемые режимы работы surface
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    //Создание логического устройства
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice); //Поиск семейства
        //Создание очереди для каждого из семейств
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        //Список уникальных семейств
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};  //Структура с информацией об очереди 
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);    //Очередь для каждого уникального семейства
        }
        /*
        VkDeviceQueueCreateInfo queueCreateInfo{};  //Структура с информацией об очереди 
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f; //Приоритет очереди
        queueCreateInfo.pQueuePriorities = &queuePriority;
        */
        VkPhysicalDeviceFeatures deviceFeatures{};

        
        VkDeviceCreateInfo createInfo{};    //Структура с информацией об очередях и возможностях устройства
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        /*  Для одной очереди
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        */
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        //Расширения
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        //Слои валидации
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }
        createInfo.pEnabledFeatures = &deviceFeatures;
        //Создание дескриптора логического устройства
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);    //Создание дескриптора очереди для рисования
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);  //Создание дескриптора очереди для отображение surface
    }

    //Выбор видеокарты
    void pickPhysicalDevice() {
        //Поиск устройства, поддерживающего Vulkan API
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        //Выбор устройства
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        
        VkPhysicalDeviceProperties deviceProperties;    //Свойства устройства
        VkPhysicalDeviceFeatures deviceFeatures;    //Возможности устройства
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        std::cout << "API version: "<<deviceProperties.apiVersion << std::endl;
        std::cout << "Device ID: " << deviceProperties.deviceID << std::endl;
        std::cout << "Device name: " << deviceProperties.deviceName << std::endl;
        std::cout << "Device type: " << deviceProperties.deviceType << std::endl;
        std::cout << "Driver version: " << deviceProperties.driverVersion << std::endl;
        //std::cout << "Device ID: " << deviceFeatures.geometryShader << std::endl;
    }

    //Проверка устройства
    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device); //Проверка семейств
        bool extensionsSupported = checkDeviceExtensionSupport(device); //Проверка поддерживаемых расширений
        //Проверка для swap chain
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }
    //Проверка поддержки расширения на устройстве
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        //Список расширений
        uint32_t extensionCount;   
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    //Поиск очередей
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        //Поиск семейства поддерживающего VK_QUEUE_GRAPHICS_BIT
        for (const auto& queueFamily : queueFamilies) {
            //Семейство для surface
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }
            //Семейство для рисования
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }

    void drawFrame() {
        //Ожидание создания предыдущего кадра
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        //Получение image из swap chain
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        //Проверка предыдущего и текущего image
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        //Отметка участия image в текущем кадре
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        //Запуск соответствующего буфера команд для этого image
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //Ожидаемые семафоры
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] }; //Список семафор, которые необходимо дождаться 
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };  //Список флагов ожидания
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        //Выбор отправляемых буферов команд
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        //Список семафор, уведомляющих о завершении
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        //Отправка буфера команд в графическую очередь
        vkResetFences(device, 1, &inFlightFences[currentFrame]);    //Сброс барьера
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        //Возвращение image в swap chain для вывода на экран
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        //Ожидаемые семафоры
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        //Указание image и swap chain 
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        //Массив для проверки каждого swap chain
        presentInfo.pResults = nullptr; // Optional
        //Отправка запроса на отображение image в swap chain
        vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;   //Переход к следующему кадру
    }

    //Главный цикл
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(device);   //Ожидание завершения всех логических операций устройства
    }

    //Освобождение ресурсов
    void cleanup() {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);   //Уничтожение семафоров
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr); //Уничтожение барьеров
        }   
        vkDestroyCommandPool(device, commandPool, nullptr); //Уничтожение пула команд
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr); //Уничтожение фреймбуферов
        }   
        vkDestroyPipeline(device, graphicsPipeline, nullptr); //Уничтожение графического конвейера
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);   //Уничтожение конвейера 
        vkDestroyRenderPass(device, renderPass, nullptr);   //Уничтожение прохода рендера
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr); //Уничтожение image view
        }
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        vkDestroySurfaceKHR(instance, surface, nullptr);    //Уничтожение surface
        vkDestroyInstance(instance, nullptr);   //Уничтожение экземпляра Vulkan API
        glfwDestroyWindow(window);  //Уничтожение окна
        glfwTerminate();    //Уничтожение GLFW
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);   //Уничтожение логического устройства
    }

    //Создание экземпляра Vulkan API
    void createInstance() {
        //Проверка слоёв валидации
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        //Специальная структура для определения информации об приложении
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        //Структура с настройками для создания экземпляра Vulkan API
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        //Определение необходимых глобальных расширений
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

        //Определение глобальных слоёв валидации
        createInfo.enabledLayerCount = 0;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        //Создание дескриптора экземпляра
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

    }

    //Выбор формата surface, опционально
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        //Проверяем наличие форматов VK_FORMAT_B8G8R8A8_SRGB - наличие B, G, R и альфа каналов по 8 бит, всего 32 бита на пиксель
        //VK_COLOR_SPACE_SRGB_NONLINEAR_KHR - поддержка цветового пространства SRGB
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }
    //Выбор режима работы, опционально
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        //VK_PRESENT_MODE_MAILBOX_KHR - изображения в очереди заменяются новыми
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        //VK_PRESENT_MODE_FIFO_KHR - изображения, отправленные вашим приложением, немедленно отправляются на экран
        return VK_PRESENT_MODE_FIFO_KHR;    //Данный режим есть гарантированно 
    }
    //Разрешение изображений
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    //Слои валидации
    //===================================================================================
    //Создание списка необходимых расширений в зависимости от подключения слоёв валидации
    std::vector<const char*> getRequiredExtensions() {
        //Определение необходимых глобальных расширений
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        //Подключение расширений в зависимости от подключения слоёв валидации
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    //Проверка доступа требуемых слоёв
    bool checkValidationLayerSupport() {
        //Получение списка доступных слоёв
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        //Проверка наличия выбранных слоёв из validationLayers в списке доступных
        for (const char* layerName : validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        return true;
    }

   
    //Callback функция для вывода сообщений слоёв валидации
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        // Сообщения о некорректном поведении
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }

    //Отладочный мессенджер
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; // Optional
    }
    //Указание callback функции
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;
        //Настройки месседжера и callback функции
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; 

        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}