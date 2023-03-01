# Vulkan

### Install
Download the SDK on [https://vulkan.lunarg.com/](https://vulkan.lunarg.com/) and install it.

If for some reason *Project properties* vanish, please write those lines: 
#### Project properties
##### VC++ Directories
###### Include Directories
```
../externals/assimp-5.0.1/include
../externals/GLFW/include
../externals/GLM/include
```
##### C/C++
###### Additional Include Directories
```
$(SolutionDir)/../externals/GLM
$(SolutionDir)/../externals/GLFW/include
C:\VulkanSDK\1.3.239.0\Include
```
##### Linker
###### Additional Library Directories
```
$(SolutionDir)/../externals/GLFW/lib-vc2019
C:\VulkanSDK\1.3.239.0\Lib
```