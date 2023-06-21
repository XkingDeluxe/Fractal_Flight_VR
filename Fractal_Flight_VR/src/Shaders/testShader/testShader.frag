#version 330 core

in vec4 gl_FragCoord;

out vec4 frag_color;



uniform vec3 camera_pos;
uniform vec4 orientation;
uniform vec2 iResolution;

#define MAX_STEPS 600
#define MAX_DIST 1e10
#define EPS .001

float GetSceneDistance(vec3 point)
{
    vec4 sphere = vec4(0, 1., 6, 1.); // (xyz, radius)
  
    float sphere_dist = length(point - sphere.xyz)-sphere.w;
    float plane_dist = point.y + 0.1; 
    
    float d = min(sphere_dist, plane_dist); 
    
    return d; 
}

float RayMarch(vec3 ray_origin, vec3 ray_dir)
{
    float d = 0.; 
    for(int i = 0; i < MAX_STEPS; i++)
    {
        vec3 p = ray_origin + ray_dir * d;
        float ds = GetSceneDistance(p); 
        d += ds; 
        if(d > MAX_DIST || ds < EPS) 
            break;  // hit object or out of scene
    }
    return d; 
}

vec3 GetNormal(vec3 point)
{
    float d = GetSceneDistance(point); 
    vec2 e = vec2(0.01, 0); 
    vec3 n = d - vec3(
        GetSceneDistance(point - e.xyy),
        GetSceneDistance(point - e.yxy),
        GetSceneDistance(point - e.yyx)
    );
    
    return normalize(n); 
}

float GetLight(vec3 point)
{    
    vec3 light_pos = vec3(3, 5, 2); 
     
    vec3 to_light = normalize(light_pos - point); 
    vec3 normal = GetNormal(point); 
    
    float intensity = 0.6;
    float light = intensity * clamp(dot(to_light, normal), 0., 1.); 
    
    float d = RayMarch(point+normal*2.*EPS, to_light);
    
    if (d < length(light_pos - point))
        light *= 0.6;
    
    return light;
}


void main()
{
    vec2 uv = (gl_FragCoord.xy - .5*iResolution.xy)/iResolution.x;

    // Time varying pixel color
    vec3 ray_origin = camera_pos;
    vec3 ray_dir = normalize(vec3(uv.x, uv.y, 1.));


   
    float d = RayMarch(ray_origin, ray_dir);
    
    vec3 point = ray_origin + d * ray_dir; 
   
    float diffuse_light = GetLight(point); 
    
    vec3 col = vec3(diffuse_light);
    
    col = pow(col, vec3(0.4545)); // Gamma correction
    // Output to screen
    frag_color = vec4(col,1.0);
}