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

Personal sandbox project for tinkering with DirectX 12.

## **Features**
- [x] **Deferred Rendering**
- [x] **Bindless Resources**
- [x] **Shaders**:
    - [x] **SM6.x** support - **default**
    - [x] **SM5.x** support
- [x] **Image Based Lighting**:
    - [x] Diffuse
    - [ ] Specular - WIP
- [ ] **Physically Based Rendering**:
    - [x] Epic's UE4 PBR Model
    - [ ] Disney's PBR
    - [ ] Sky reflections
- [ ] **Model loading**:
    - [x] glTF 2.0
    - [ ] glb
    - [x] fbx


## **Built with**
- **C++20**
- **DirectX 12**
- **DXR**
- **Visual Studio 2022:**
    - *MSVC*
    - *MSBuild*
    - *Windows SDK*
    - *Win32 API*
- [**vcpkg**](https://www.vcpkg.io)

> [!NOTE]
> Supports **x64** architecture only.
> 
> **HLSL SM6.6** support is required.

## **Dependencies**
- [**D3D12MemoryAllocator**](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator)
- [**D3D12 Agility SDK**](https://devblogs.microsoft.com/directx/directx12agility/)
- [**ImGui**](https://github.com/ocornut/imgui)
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

|         |         |
|:-------:|:-------:|
|![Screnshot](Media/deferred_scifi_helmet.png)|![Screnshot](Media/deferred_sponza.png)|
