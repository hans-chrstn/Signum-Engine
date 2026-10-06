#version 450

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 p = fragUV * 2.0 - 1.0;
    p.x *= 1280.0 / 720.0;

    float radius = 0.55;
    float distanceSquared = dot(p, p);

    if (distanceSquared  > radius * radius) {
      outColor = vec4(0.005, 0.008, 0.02, 1.0);
      return;
    }

    float z  = sqrt(radius * radius - distanceSquared);

    vec3 normal = normalize(vec3(p.x, p.y, z));
    vec3 lightDirection = normalize(vec3(-0.8, 0.7, 1.0));

    float diffuse = max(dot(normal, lightDirection), 0.0);
    float ambient = 0.08;
    float brightness = ambient + diffuse * 0.92;

    vec3 planetColor = vec3(0.08, 0.32, 0.85);
    vec3 finalColor = planetColor * brightness;

    outColor = vec4(finalColor, 1.0);
}
