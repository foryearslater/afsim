# afsim仿真介绍

## 1.环境配置

visual studio 2019 

cmake 3.3 以上

Qt 5.12

conda 24.11.3

设置系统变量

![image](https://github.com/foryearslater/afsim/blob/main/image-20250809223711800.png)


构建环境  
![image](https://github.com/foryearslater/afsim/blob/main/image-20250809223404159.png)

<<<<<<< HEAD
=======
![image-20250809223711800](C:\Users\26459\Desktop\afsim\image-20250809223711800.png)
>>>>>>> 84206d3 (no message)

![image-20250809223404159](C:\Users\26459\Desktop\afsim\image-20250809223404159.png

![image-20250809223404159](C:\Users\26459\Desktop\afsim\image-20250809223404159.png)

## 2.模块介绍

Warlock 核心模块

- **根目录**：包含 1.txt 文件，以及 dependencies 和 src 两个主要文件夹。

- dependencies 文件夹

  ：存放项目依赖相关内容

  - 3rd_party：包含多个第三方库的压缩包，如 CURL、gdal 等，且部分库已解压，内含 bin、include、lib 等子文件夹，分别存放可执行文件、头文件和库文件。
  - resources：包含 vtk 资源压缩包，以及 data、maps、models、shaders 等子文件夹，涵盖数据文件、地图数据、模型文件和着色器文件等资源。

- src 文件夹

  ：是项目源代码及相关文件的主要存放地

  - 包含 cmake、core、doc、tools 等子文件夹。
  - cmake：涉及编译配置相关文件。
  - core：包含多个与核心功能相关的模块，如 wsf、wsf_cyber 等，每个模块下有源代码、文档、测试任务等内容。
  - doc：存放文档相关文件，如变更日志、开发指南、用户手册等。
  - tools：包含第三方库的 cmake 配置、dis 和 genio 等工具的源代码等。
