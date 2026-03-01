#include <iostream>
#include "EncodingTool.hpp"
#include "window.hpp"
#include "Scenes.hpp"

int main() {
    std::cout << "encodingtest: gbk" << std::endl;
    GameWindow window;
    // 设置初始场景为引导页（任意键继续），之后切换到选择界面
    window.scenes().requestChangeScene(std::make_unique<IntroScene>(&window.scenes()));
    window.run();
    return 0;
}
