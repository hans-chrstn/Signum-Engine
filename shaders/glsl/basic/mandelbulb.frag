#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265358979323846;

const int MAX_STEPS = 120;
const int FRACTAL_ITERATIONS = 12;
const int SHADOW_STEPS = 36;

const float MAX_DISTANCE = 20.0;
const float SURFACE_EPSILON = 0.0008;

const float POWER = 8.0;
const float BAILOUT = 8.0;

mat2 rotate2D(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

vec3 backgroundColor(vec3 rd) {
    float sky = 0.5 * (rd.y + 1.0);

    vec3 color = mix(
        vec3(0.002, 0.003, 0.008),
        vec3(0.010, 0.020, 0.045),
        sky
    );

    vec2 uv = vec2(
        atan(rd.z, rd.x) / (2.0 * PI) + 0.5,
        asin(clamp(rd.y, -1.0, 1.0)) / PI + 0.5
    );

    vec2 gridA = uv * vec2(420.0, 210.0);
    vec2 idA = floor(gridA);
    vec2 cellA = fract(gridA) - 0.5;
    vec2 offA = vec2(hash21(idA), hash21(idA + 7.13)) - 0.5;
    float starA = 1.0 - smoothstep(
        0.015,
        0.050,
        length(cellA - offA * 0.7)
    );
    starA *= step(0.992, hash21(idA + 13.1));

    vec2 gridB = uv * vec2(180.0, 90.0) + 19.7;
    vec2 idB = floor(gridB);
    vec2 cellB = fract(gridB) - 0.5;
    vec2 offB = vec2(hash21(idB + 9.21), hash21(idB + 4.83)) - 0.5;
    float starB = 1.0 - smoothstep(
        0.020,
        0.070,
        length(cellB - offB * 0.6)
    );
    starB *= step(0.997, hash21(idB + 31.7));

    color += vec3(0.70, 0.80, 1.00) * starA * 0.85;
    color += vec3(1.00, 0.90, 0.70) * starB * 2.0;

    float haze = pow(max(1.0 - abs(rd.y), 0.0), 10.0);
    color += vec3(0.03, 0.04, 0.08) * haze;

    return color;
}

float mandelbulbDE(vec3 p, out vec4 orbitTrap) {
    vec3 z = p;
    float dr = 1.0;
    float r = 0.0;

    orbitTrap = vec4(1e9);

    for (int i = 0; i < FRACTAL_ITERATIONS; ++i) {
        r = length(z);

        orbitTrap = min(
            orbitTrap,
            vec4(abs(z), r)
        );

        if (r > BAILOUT) {
            break;
        }

        float rSafe = max(r, 1e-6);
        float theta = acos(clamp(z.z / rSafe, -1.0, 1.0));
        float phi = atan(z.y, z.x);

        float zr = pow(rSafe, POWER);
        dr = pow(rSafe, POWER - 1.0) * POWER * dr + 1.0;

        theta *= POWER;
        phi *= POWER;

        z = zr * vec3(
            sin(theta) * cos(phi),
            sin(theta) * sin(phi),
            cos(theta)
        );

        z += p;
    }

    r = max(r, 1e-6);
    return 0.5 * log(r) * r / dr;
}

float sceneDE(vec3 p, out vec4 orbitTrap) {
    vec3 q = p;

    q.xz *= rotate2D(0.85);
    q.xy *= rotate2D(-0.35);

    float scale = 1.15;
    q *= scale;

    float d = mandelbulbDE(q, orbitTrap);
    return d / scale;
}

bool rayMarch(
    vec3 ro,
    vec3 rd,
    out float t,
    out vec4 trap
) {
    t = 0.0;
    trap = vec4(1e9);

    for (int i = 0; i < MAX_STEPS; ++i) {
        vec3 p = ro + rd * t;

        vec4 localTrap;
        float d = sceneDE(p, localTrap);

        trap = localTrap;

        if (d < SURFACE_EPSILON * (1.0 + 0.12 * t)) {
            return true;
        }

        t += d * 0.85;

        if (t > MAX_DISTANCE) {
            break;
        }
    }

    return false;
}

vec3 calcNormal(vec3 p) {
    vec4 trap;
    float e = 0.0012;

    float dx = sceneDE(p + vec3(e, 0.0, 0.0), trap) - sceneDE(p - vec3(e, 0.0, 0.0), trap);
    float dy = sceneDE(p + vec3(0.0, e, 0.0), trap) - sceneDE(p - vec3(0.0, e, 0.0), trap);
    float dz = sceneDE(p + vec3(0.0, 0.0, e), trap) - sceneDE(p - vec3(0.0, 0.0, e), trap);

    return normalize(vec3(dx, dy, dz));
}

float softShadow(vec3 ro, vec3 rd) {
    float result = 1.0;
    float t = 0.02;

    for (int i = 0; i < SHADOW_STEPS; ++i) {
        vec4 trap;
        float h = sceneDE(ro + rd * t, trap);

        result = min(result, 10.0 * h / t);
        t += clamp(h, 0.02, 0.25);

        if (result < 0.001 || t > 8.0) {
            break;
        }
    }

    return clamp(result, 0.0, 1.0);
}

float ambientOcclusion(vec3 p, vec3 n) {
    float occ = 0.0;
    float weight = 1.0;

    for (int i = 0; i < 5; ++i) {
        float h = 0.02 + 0.06 * float(i);
        vec4 trap;
        float d = sceneDE(p + n * h, trap);
        occ += (h - d) * weight;
        weight *= 0.7;
    }

    return clamp(1.0 - 1.6 * occ, 0.0, 1.0);
}

vec3 fractalColor(vec3 p, vec4 trap) {
    vec3 trapColor = exp(-3.5 * trap.xyz);

    float band =
        0.5 +
        0.5 *
        sin(
            10.0 * trap.w +
            3.0 * p.y +
            4.0 * trap.x
        );

    vec3 color = vec3(0.08, 0.10, 0.16);

    color += vec3(0.95, 0.28, 0.10) * trapColor.x;
    color += vec3(0.10, 0.35, 0.95) * trapColor.y;
    color += vec3(1.00, 0.80, 0.22) * trapColor.z;

    color *= mix(0.75, 1.25, band);

    float coreGlow = exp(-6.0 * trap.w);
    color = mix(color, vec3(1.0, 0.92, 0.72), coreGlow * 0.45);

    return clamp(color, 0.0, 2.0);
}

vec3 tonemapACES(vec3 x) {
    return clamp(
        (x * (2.51 * x + 0.03)) /
        (x * (2.43 * x + 0.59) + 0.14),
        0.0,
        1.0
    );
}

void main() {
    vec2 p = fragUV * 2.0 - 1.0;
    p.x *= 1280.0 / 720.0;

    vec3 ro = vec3(0.0, 0.15, -4.4);
    vec3 target = vec3(0.0, 0.0, 0.0);

    vec3 forward = normalize(target - ro);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = normalize(cross(right, forward));

    float focalLength = 1.35;

    vec3 rd = normalize(
        forward * focalLength +
        right * p.x +
        up * p.y
    );

    vec3 bg = backgroundColor(rd);

    float t;
    vec4 trap;

    if (!rayMarch(ro, rd, t, trap)) {
        outColor = vec4(bg, 1.0);
        return;
    }

    vec3 pos = ro + rd * t;
    vec3 normal = calcNormal(pos);

    vec3 lightDir = normalize(vec3(0.65, 0.75, -0.55));
    vec3 fillDir = normalize(vec3(-0.4, 0.3, -0.8));

    float shadow = softShadow(pos + normal * 0.005, lightDir);
    float ao = ambientOcclusion(pos, normal);

    float diffuse = max(dot(normal, lightDir), 0.0);
    float fill = max(dot(normal, fillDir), 0.0);

    vec3 halfVector = normalize(lightDir - rd);
    float specular = pow(max(dot(normal, halfVector), 0.0), 32.0);

    float rim = pow(1.0 - max(dot(normal, -rd), 0.0), 3.0);

    vec3 albedo = fractalColor(pos, trap);

    vec3 color = vec3(0.0);

    color += albedo * 0.10 * ao;
    color += albedo * diffuse * shadow * 1.15;
    color += albedo * fill * 0.22;
    color += vec3(1.0, 0.92, 0.82) * specular * shadow * 0.22;
    color += vec3(0.35, 0.45, 0.90) * rim * 0.20 * ao;

    float fog = 1.0 - exp(-0.018 * t * t);
    color = mix(color, bg, fog);

    color = tonemapACES(color);

    outColor = vec4(color, 1.0);
}
