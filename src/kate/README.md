Kate is how the render API is named, which will have the job of abstracting platform specific GPU code as an agnostic API.

- The GPU API should be implemented in gpu/ where platform specific GPU code would live in gpu/vk and gpu/d3d12.

- Kate uses Kate Translator as an ahead-of-time shader translator which translates KSL (an agnostic shader language) to GLSL, and in the future, to HLSL.
Everything belonging to Kate Translator should live in translator/
Kate Translator should be fully independent from gpu/

