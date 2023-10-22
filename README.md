# idgenerator

`lanthing-svr`的ID生成工具。

这个工具分两部分：idgenerator和idwriter。

## idgenerator

idgenerator使用C++17编写，没有依赖第三方库。可以使用任意支持C++17的编译器直接编译，比如`g++ main.cpp`，或者使用CMake。

运行idgenerator无需传参，运行结果以文件输出。生成的ID都是9位数，从100'000'000到999'999'999，中间会剔除掉一些ID，每个ID占用4字节，紧凑地写入文件，每20万个ID存一个文件，最终输出约3.5GB的内容。

## idwriter
idwriter使用GO编写，直接使用go build命令即可。

运行idwriter需要传入两个参数`-db=/paht/to/lanthing-svr.db`和`-id=/path/to/id_xxx_file`。前者是`lanthing-svr`要用的数据库文件，如果不存在会自动生成、自动建表。后者是idgenerator生成的id文件，随便指定一个没用过的即可。idwriter会把后者的内容全部写入前者。