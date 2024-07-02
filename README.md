# DNS-Relay

## 依赖管理方式以及编译方式

我们使用conan来管理项目的依赖

conan的安装 ，使用pip或者conda来安装conan

例如对于conda，我们可以先创建一个conda的新环境cpp，然后在这个环境里安装conan

安装完成conan之后需要 执行`conan install .. --build=missing -s build_type=Debug`。这个是conan自动给你根据conanfile.txt安装依赖

然后编译需要给cmake指定conan的配置文件的位置，`cmake .. -DCMAKE_TOOLCHAIN_FILE=./Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles"`

然后执行make即可

整体流程
1. `conda activate cpp`
2. `cd build`
3. `conan install .. --build=missing -s build_type=Debug`
4. `cmake .. -DCMAKE_TOOLCHAIN_FILE=./Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles"`
5. `make`
6. 然后执行bin目录里面的test和main



## 文档目录结构

1. build 所有编译中的临时文件都在这里，方便以后删除，清除历史记录后重新编译
2. build/bin 可执行文件在这里
3. lib 编译产生的dll或者so文件再这里
4. include 所有的头文件
5. src cpp文件 包含程序的main文件
6. test 单元测试的文件，链接到dll上 进行单元测试
