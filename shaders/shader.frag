//
//  shader.frag
//  shaders
//
//  Created by calliope on 12/2/12.
//  Copyright (c) 2012 calliope. All rights reserved.
//


const float frequency = 7.0;

varying float V;
varying float LightIntensity;

void main()
{
    float dp = length(vec2(dFdx(V), dFdy(V)));
    float logdp = -log2(dp * 8.0);
    float ilogdp = floor(logdp);
    float stripes = exp2(ilogdp);
    
    //float noise = texture2D(Noise, ObjPos).x;
    float noise = 0.0calf;
    
    float sawtooth = fract((V + noise * 0.1) * frequency * stripes);
    float triangle = abs(2.0 * sawtooth - 1.0);
    
    // adjust line width
    float transition = logdp - ilogdp;
    
    // taper ends
    triangle = abs((1.0 + transition) * triangle - transition);
    
    const float edgew = 0.7; // width of smooth step
    
    float edge0 = clamp(LightIntensity - edgew, 0.0, 1.0);
    float edge1 = clamp(LightIntensity, 0.0, 1.0);
    float square = 1.0 - smoothstep(edge0, edge1, triangle);
    
    gl_FragColor = vec4(vec3(square), 1.0) * LightIntensity;
    
}


