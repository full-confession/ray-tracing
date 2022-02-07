# ray-tracing

# BSDFs and Materials
Implemented BSDFs are shown in the table. Microfacet BSDFs use Smith+GGX model together with the implementation of the sampling of the distribution of visible normals (<a href="https://jcgt.org/published/0007/04/01/">Link</a>).
| Shader | Preview | Shader | Preview |
| --- | --- | --- | --- |
| Diffuse <br/> [Lambertian BRDF](src/bsdfs/lambertian_reflection.hpp) | <img src="/img/diffuse.png" alt="Diffuse" width="256"/> | Plastic <br/> [Lambertian BRDF](src/bsdfs/lambertian_reflection.hpp) <br/> [Microfacet BRDF](src/bsdfs/microfacet_reflection.hpp#L37) | <img src="/img/plastic.png" alt="Plastic" width="256"/> |
| Smooth Mirror <br/> [Specular BRDF](src/bsdfs/specular_reflection.hpp) | <img src="/img/specular_mirror.png" alt="Smooth Mirror" width="256"/> | Rough Mirror <br/> [Microfacet BRDF](src/bsdfs/microfacet_reflection.hpp#L37) | <img src="/img/microfacet_mirror.png" alt="Rough Mirror" width="256"/> |
| Smooth Glass <br/> [Specular Glass BSDF](src/bsdfs/specular_glass.hpp) | <img src="/img/specular-glass.png" alt="Smooth Glass" width="256"/> | Smooth Glass with Uniform Medium <br/> [Specular Glass BSDF](src/bsdfs/specular_glass.hpp) <br/> [Uniform medium](src/core/medium.hpp#L40) | <img src="/img/specular_medium_glass.png" alt="Smooth Glass with Uniform Medium" width="256"/> |
| Rough Glass <br/> [Microfacet Glass BSDF](src/bsdfs/microfacet_glass.hpp) | <img src="/img/microfacet_glass.png" alt="Rough Glass" width="256"/> | Rough Glass with Uniform Medium <br/> [Microfacet Glass BSDF](src/bsdfs/microfacet_glass.hpp) <br/> [Uniform medium](src/core/medium.hpp#L40)  | <img src="/img/microfacet_medium_glass.png" alt="Rough Glass with Uniform Medium" width="256"/> |

BSDFs can be combined together and they can be parameterized with textures.

<table>
  <tbody align="center">
    <tr>
      <td rowspan=4><img src="/img/mask.png" alt="Lambertian" width="400"/><br/><a href="https://sketchfab.com/3d-models/venice-mask-4aace12762ee44cf97d934a6ced12e65">Venice Mask</a></td>
      <td>Basecolor texture<br/><img src="/img/mask_basecolor.png" alt="Basecolor" width="512"/></td>
    </tr>
    <tr>
      <td>Metalness texture<br/><img src="/img/mask_metalness.png" alt="Metalness" width="512"/></td>
    </tr>
    <tr>
      <td>Normal texture<br/><img src="/img/mask_normal.png" alt="Normal" width="512"/></td>
    </tr>
    <tr>
      <td>Roughness texture<br/><img src="/img/mask_roughness.png" alt="Roughness" width="512"/></td>
    </tr>
  </tbody>
</table>
 
 # Integrators
 - Forward BSDF Inegrators. Performs a random walk starting from the camera and using the BSDF at the intersection points to choose the next direction.
 - Forward MIS Integrator. Same as previous but additionally performing direct lighting at the intersection points.
 - Backward Integrator. Performs a random walk starting from the light source.
 - BDPT Integrator. Builds two subpaths starting from the camera and the light source using random walk and then tries to connect different subpaths prefixes.
 <table>
  <thead>
    <tr>
      <th colspan=2>Result in 1 minute</th>
    </tr>
  </thead>
  <tbody align="center">
    <tr>
      <td><img src="/img/room_bsdf_1m.png" alt="BSDF 1m" width="600"/><br/><a href="src/integrators/forward_bsdf_integrator.hpp">Forward BSDF Integrator</a></td>
      <td><img src="/img/room_mis_1m.png" alt="MIS 1m" width="600"/><br/><a href="src/integrators/forward_mis_integrator.hpp">Forward MIS Integrator</a></td>
    </tr>
    <tr>
      <td><img src="/img/room_back_1m.png" alt="BACK 1m" width="600"/><br/><a href="src/integrators/backward_integrator.hpp">Backward Integrator</a></td>
      <td><img src="/img/room_bdpt_1m.png" alt="BDPT 1m" width="600"/><br/><a href="src/integrators/bidirectional_integrator.hpp">BDPT Integrator</a></td>
    </tr>
  </tbody>
 </table>
 
<table>
  <thead>
    <tr>
      <th colspan=2>Result in 10 minutes</th>
    </tr>
  </thead>
  <tbody align="center">
    <tr>
     <td><img src="/img/room_bsdf_10m.png" alt="BSDF 10m" width="600"/><br/><a href="src/integrators/forward_bsdf_integrator.hpp">Forward BSDF Integrator</a></td>
     <td><img src="/img/room_mis_10m.png" alt="MIS 10m" width="600"/><br/><a href="src/integrators/forward_mis_integrator.hpp">Forward MIS Integrator</a></td>
    </tr>
    <tr>
     <td><img src="/img/room_back_10m.png" alt="BACK 10m" width="600"/><br/><a href="src/integrators/backward_integrator.hpp">Backward Integrator</a></td>
     <td><img src="/img/room_bdpt_10m.png" alt="BDPT 10m" width="600"/><br/><a href="src/integrators/bidirectional_integrator.hpp">BDPT Integrator</a></td>
    </tr>
   </tbody>
 </table>
 
 # Mediums
 Every closed surface can act as a simple medium. Each medium parameterized by refractive index, absorption coefficient and priority. Only a single medium can exist at any given point, the one with the highest priority. Intersection on the medium boundary is reported only when the medium above the boundary differs from the medium below the boundary. 
 In the picture, glass and ice have a priority of two, tea/water/something has a priority of one.
 
<img src="/img/glass.png" alt="BSDF 10m" width="512"/>
 
 # Normal mapping
 
 - Altering shading frame. Construct a new shading frame using old shading tangent and bitangent with shading normal obtained from the normal map. Invalid cases (such as when a vector is in the upper hemisphere around the geometric normal but is in the lower hemisphere aroung geometric normal) are clipped to preserve the symmetry of the light transport.
 - Microfacet-based normal mapping. Idea is to construct a geometrically valid microfacet surface made of two facets per shading point: the one given by the normal map at the shading point and an additional facet that compensates for it such that the average normal of the microsurface equals the geometric normal (<a href="https://blog.unity.com/technology/microfacet-based-normal-mapping-for-robust-monte-carlo-path-tracing">Link</a>).
<table>
  <thead>
    <tr>
      <th></th>
      <th>Diffuse</th>
      <th>Rough Mirror</th>
      <th>Specular Mirror</th>
    </tr>
  </thead>
  <tbody align="center">
    <tr>
      <td>Altering shading frame</td>
      <td><img src="img/shading_normal_diffuse.png"/></td>
      <td><img src="img/shading_normal_rough.png"/></td>
      <td><img src="img/shading_normal_specular.png"/></td>
    </tr>
    <tr>
     <td><a href="src/bsdfs/normal_mapping.hpp">Microfacet-based normal mapping</a></td>
     <td><img src="img/microfacet_normal_diffuse.png"/></td>
     <td><img src="img/microfacet_normal_rough.png"/></td>
     <td><img src="img/microfacet_normal_specular.png"/></td>
    </tr>
  </tbody>
</table>
