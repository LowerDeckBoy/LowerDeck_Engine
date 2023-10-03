# LowerDeck Engine

<!--TOC-->
  - [**Overview**](#overview)
  - [**Features**](#features)
  - [**Built with**](#built-with)
  - [**Dependencies**](#dependencies)
  - [**Controls**](#controls)
  - [**Sample screenshots**](#sample-screenshots)
<!--/TOC-->

## **Overview**
**WIP** - calling it *engine* at this point is an overstatement.<br/><br/>
Restructured and improved version of my other project: [**DirectX 12 Rendering**](https://github.com/LowerDeckBoy/DirectX-12-Rendering).<br/><br/>Current goals are focused more on ECS based graphics pipeline workflow for hybrid rendering path rather then graphics techniques.

## **Features**
- [x] **Deferred Rendering**
- [x] **Bindless Resources**:
    - [x] Texturing
    - [ ] Buffers
- [ ] **DXR**:
    - [ ] Raytraced Shadows
    - [ ] Raytraced Ambient Occlusion
    - [ ] Raytraced Reflections
- [ ] **Mesh Shading**:
    - [ ] *Meshletizing*
    - [ ] Amplification/Tesselation
- [ ] **Image Based Lighting**:
    - [ ] Diffuse
    - [ ] Specular
- [ ] **Physically Based Rendering**:
    - [ ] Epic's UE4 PBR Model
    - [ ] Disney's PBR
    - [ ] Sky reflections
- [ ] **Model loading**:
    - [x] glTF 2.0
    - [ ] glb
    - [ ] fbx
- [ ] **Physics**:
    - [ ] *PhysX*
- [ ] **Particle System**


## **Built with**
- **C++ 20**
- **DirectX 12**
- **DXR**
- **Visual Studio 2022:**
    - *MSVC*
    - *MSBuild*
    - *Windows SDK*
    - *Win32 API*
- [**vcpkg**](https://www.vcpkg.io)
- [**XML Documentation**](https://learn.microsoft.com/en-us/cpp/build/reference/xml-documentation-visual-cpp?view=msvc-170)

**Note:<br/>Supports **x64** architecture only.<br/>Windows 11 and Visual Studio 2022 are required in order to access **HLSL SM6.6.**<br/>DLLs for compiling HLSL 6.x are provided via Post-Build event.**

## **Dependencies**
- [**D3D12MemoryAllocator**](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator)
- [**ImGui**](https://github.com/ocornut/imgui) - Docking branch
- [**assimp**](https://github.com/assimp/assimp)
- [**EnTT**](https://github.com/skypjack/entt)
- [**DirectXTex**](https://github.com/microsoft/DirectXTex)
- [**DirectXTK12**](https://github.com/Microsoft/DirectXTK12)

## **Controls**
- Hold **RMB** to enable camera movement.
- **WASD** to move scene camera around.
- **E** to move camera up, **Q** to move camera down.
- **R** resets camera position and rotation.
- **ESC** to exit app.

## **Sample screenshots**
