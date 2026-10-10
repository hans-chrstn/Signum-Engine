#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265358979323846;

const float SCHWARZSCHILD_RADIUS = 1.0;
const float MASS = 0.5;

const float DISK_INNER_RADIUS = 3.0;
const float DISK_OUTER_RADIUS = 9.0;

const int GEODESIC_STEPS = 360;
const float MAX_PHI = 4.0 * PI;
const float STEP_PHI = MAX_PHI / float(GEODESIC_STEPS);

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

vec2 hash22(vec2 p) {
    float n = hash21(p);

    return vec2(
        n,
        hash21(p + 17.31)
    );
}

vec3 starField(vec3 direction) {
    direction = normalize(direction);

    float longitude =
        atan(direction.z, direction.x) /
        (2.0 * PI) +
        0.5;

    float latitude =
        asin(clamp(direction.y, -1.0, 1.0)) /
        PI +
        0.5;

    vec2 uv =
        vec2(longitude, latitude);

    vec3 color =
        vec3(
            0.0002,
            0.0003,
            0.0007
        );

    vec2 grid1 =
        uv *
        vec2(
            850.0,
            425.0
        );

    vec2 id1 =
        floor(grid1);

    vec2 cell1 =
        fract(grid1) -
        0.5;

    vec2 offset1 =
        hash22(id1) -
        0.5;

    float noise1 =
        hash21(id1 + 13.7);

    float distance1 =
        length(
            cell1 -
            offset1 * 0.7
        );

    float star1 =
        1.0 -
        smoothstep(
            0.018,
            0.045,
            distance1
        );

    star1 *=
        step(
            0.982,
            noise1
        );

    vec2 grid2 =
        uv *
        vec2(
            380.0,
            190.0
        ) +
        19.31;

    vec2 id2 =
        floor(grid2);

    vec2 cell2 =
        fract(grid2) -
        0.5;

    vec2 offset2 =
        hash22(id2) -
        0.5;

    float noise2 =
        hash21(id2 + 71.4);

    float distance2 =
        length(
            cell2 -
            offset2 * 0.6
        );

    float star2 =
        1.0 -
        smoothstep(
            0.025,
            0.070,
            distance2
        );

    star2 *=
        step(
            0.992,
            noise2
        );

    vec3 tint =
        mix(
            vec3(0.72, 0.82, 1.0),
            vec3(1.0, 0.82, 0.62),
            hash21(id2 + 92.1)
        );

    color +=
        vec3(0.8, 0.88, 1.0) *
        star1 *
        0.85;

    color +=
        tint *
        star2 *
        2.4;

    return color;
}

float geodesicAcceleration(float u) {
    return
        3.0 * MASS * u * u -
        u;
}

void integrateGeodesic(
    inout float u,
    inout float du
) {
    float k1u =
        du;

    float k1v =
        geodesicAcceleration(u);

    float u2 =
        u +
        0.5 *
        STEP_PHI *
        k1u;

    float v2 =
        du +
        0.5 *
        STEP_PHI *
        k1v;

    float k2u =
        v2;

    float k2v =
        geodesicAcceleration(u2);

    float u3 =
        u +
        0.5 *
        STEP_PHI *
        k2u;

    float v3 =
        du +
        0.5 *
        STEP_PHI *
        k2v;

    float k3u =
        v3;

    float k3v =
        geodesicAcceleration(u3);

    float u4 =
        u +
        STEP_PHI *
        k3u;

    float v4 =
        du +
        STEP_PHI *
        k3v;

    float k4u =
        v4;

    float k4v =
        geodesicAcceleration(u4);

    u +=
        STEP_PHI *
        (
            k1u +
            2.0 * k2u +
            2.0 * k3u +
            k4u
        ) /
        6.0;

    du +=
        STEP_PHI *
        (
            k1v +
            2.0 * k2v +
            2.0 * k3v +
            k4v
        ) /
        6.0;
}

vec3 radialDirection(
    float phi,
    vec3 initialRadial,
    vec3 initialTangent
) {
    return
        cos(phi) *
        initialRadial +
        sin(phi) *
        initialTangent;
}

vec3 angularDirection(
    float phi,
    vec3 initialRadial,
    vec3 initialTangent
) {
    return
        -sin(phi) *
        initialRadial +
        cos(phi) *
        initialTangent;
}

vec3 positionFromState(
    float u,
    float phi,
    vec3 initialRadial,
    vec3 initialTangent
) {
    return
        radialDirection(
            phi,
            initialRadial,
            initialTangent
        ) /
        u;
}

vec3 directionFromState(
    float u,
    float du,
    float phi,
    vec3 initialRadial,
    vec3 initialTangent
) {
    vec3 radial =
        radialDirection(
            phi,
            initialRadial,
            initialTangent
        );

    vec3 angular =
        angularDirection(
            phi,
            initialRadial,
            initialTangent
        );

    return normalize(
        angular -
        radial *
        (du / max(u, 0.00001))
    );
}

float diskMask(float radius) {
    float inner =
        smoothstep(
            DISK_INNER_RADIUS,
            DISK_INNER_RADIUS + 0.35,
            radius
        );

    float outer =
        1.0 -
        smoothstep(
            DISK_OUTER_RADIUS - 1.5,
            DISK_OUTER_RADIUS,
            radius
        );

    return
        inner *
        outer;
}

vec3 diskEmission(
    vec3 position,
    vec3 tracedDirection
) {
    float radius =
        length(position.xz);

    float angle =
        atan(
            position.z,
            position.x
        );

    float mask =
        diskMask(radius);

    float radialScale =
        radius /
        DISK_INNER_RADIUS;

    float temperatureProfile =
        max(
            1.0 -
            sqrt(
                DISK_INNER_RADIUS /
                radius
            ),
            0.0
        );

    temperatureProfile /=
        radialScale *
        radialScale *
        radialScale;

    temperatureProfile =
        pow(
            max(
                temperatureProfile,
                0.0
            ),
            0.25
        );

    float heat =
        smoothstep(
            0.34,
            0.49,
            temperatureProfile
        );

    vec3 deepRed =
        vec3(
            0.55,
            0.008,
            0.001
        );

    vec3 redOrange =
        vec3(
            1.0,
            0.055,
            0.003
        );

    vec3 orange =
        vec3(
            1.0,
            0.24,
            0.012
        );

    vec3 gold =
        vec3(
            1.0,
            0.58,
            0.075
        );

    vec3 hot =
        vec3(
            1.0,
            0.88,
            0.48
        );

    vec3 color =
        mix(
            deepRed,
            redOrange,
            smoothstep(
                0.00,
                0.22,
                heat
            )
        );

    color =
        mix(
            color,
            orange,
            smoothstep(
                0.18,
                0.52,
                heat
            )
        );

    color =
        mix(
            color,
            gold,
            smoothstep(
                0.48,
                0.82,
                heat
            )
        );

    color =
        mix(
            color,
            hot,
            smoothstep(
                0.88,
                1.0,
                heat
            )
        );

    float structure =
        0.78 +
        0.12 *
        sin(
            radius * 17.0 -
            angle * 7.0
        ) +
        0.06 *
        sin(
            radius * 41.0 +
            angle * 13.0
        );

    structure =
        clamp(
            structure,
            0.35,
            1.25
        );

    float orbitalSpeed =
        sqrt(
            MASS /
            max(
                radius -
                SCHWARZSCHILD_RADIUS,
                0.001
            )
        );

    orbitalSpeed =
        clamp(
            orbitalSpeed,
            0.0,
            0.78
        );

    vec3 orbitalDirection =
        normalize(
            vec3(
                -position.z,
                0.0,
                position.x
            )
        );

    vec3 photonToCamera =
        -normalize(
            tracedDirection
        );

    float velocityProjection =
        dot(
            orbitalDirection,
            photonToCamera
        );

    float gamma =
        1.0 /
        sqrt(
            max(
                1.0 -
                orbitalSpeed *
                orbitalSpeed,
                0.01
            )
        );

    float doppler =
        1.0 /
        (
            gamma *
            max(
                1.0 -
                orbitalSpeed *
                velocityProjection,
                0.15
            )
        );

    float gravitationalShift =
        sqrt(
            max(
                1.0 -
                SCHWARZSCHILD_RADIUS /
                radius,
                0.0
            )
        );

    float frequencyShift =
        gravitationalShift *
        doppler;

    float beaming =
        pow(
            clamp(
                frequencyShift,
                0.50,
                1.45
            ),
            2.4
        );

    float approaching =
        smoothstep(
            1.05,
            1.45,
            frequencyShift
        );

    float receding =
        1.0 -
        smoothstep(
            0.65,
            0.95,
            frequencyShift
        );

    color *=
        mix(
            vec3(
                1.08,
                0.76,
                0.62
            ),
            vec3(1.0),
            1.0 - receding
        );

    color *=
        mix(
            vec3(1.0),
            vec3(
                0.93,
                1.00,
                1.10
            ),
            approaching * 0.18
        );

    float radialBrightness =
        mix(
            0.65,
            1.35,
            heat
        );

    return
        color *
        structure *
        mask *
        radialBrightness *
        (
            0.32 +
            beaming * 0.68
        );
}

void main() {
    vec2 p =
        fragUV *
        2.0 -
        1.0;

    p.x *=
        1280.0 /
        720.0;

    vec3 cameraPosition =
        vec3(
            0.0,
            3.0,
            20.0
        );

    vec3 cameraForward =
        normalize(
            -cameraPosition
        );

    vec3 cameraRight =
        normalize(
            cross(
                cameraForward,
                vec3(
                    0.0,
                    1.0,
                    0.0
                )
            )
        );

    vec3 cameraUp =
        normalize(
            cross(
                cameraRight,
                cameraForward
            )
        );

    float verticalFov =
        radians(45.0);

    float focalLength =
        1.0 /
        tan(
            verticalFov *
            0.5
        );

    vec3 rayDirection =
        normalize(
            cameraForward *
            focalLength +
            cameraRight *
            p.x +
            cameraUp *
            p.y
        );

    float cameraRadius =
        length(
            cameraPosition
        );

    float u =
        1.0 /
        cameraRadius;

    vec3 initialRadial =
        cameraPosition /
        cameraRadius;

    vec3 tangentComponent =
        rayDirection -
        initialRadial *
        dot(
            rayDirection,
            initialRadial
        );

    float tangentLength =
        length(
            tangentComponent
        );

    if (tangentLength < 0.0005) {
        outColor =
            vec4(
                0.0,
                0.0,
                0.0,
                1.0
            );

        return;
    }

    vec3 initialTangent =
        tangentComponent /
        tangentLength;

    float du =
        -dot(
            rayDirection,
            initialRadial
        ) /
        tangentLength *
        u;

    float phi =
        0.0;

    vec3 oldPosition =
        cameraPosition;

    vec3 finalDirection =
        rayDirection;

    vec3 color =
        vec3(0.0);

    float transmittance =
        1.0;

    bool captured =
        false;

    for (
        int i = 0;
        i < GEODESIC_STEPS;
        ++i
    ) {
        if (
            u >=
            1.0 /
            SCHWARZSCHILD_RADIUS
        ) {
            captured =
                true;

            break;
        }

        float nextU =
            u;

        float nextDu =
            du;

        integrateGeodesic(
            nextU,
            nextDu
        );

        float nextPhi =
            phi +
            STEP_PHI;

        if (
            nextU <=
            0.0
        ) {
            finalDirection =
                directionFromState(
                    u,
                    du,
                    phi,
                    initialRadial,
                    initialTangent
                );

            break;
        }

        if (
            nextU <
            1.0 / 80.0 &&
            nextDu < 0.0
        ) {
            finalDirection =
                directionFromState(
                    nextU,
                    nextDu,
                    nextPhi,
                    initialRadial,
                    initialTangent
                );

            break;
        }

        if (
            nextU >=
            1.0 /
            SCHWARZSCHILD_RADIUS
        ) {
            captured =
                true;

            break;
        }

        vec3 nextPosition =
            positionFromState(
                nextU,
                nextPhi,
                initialRadial,
                initialTangent
            );

        vec3 segment =
            nextPosition -
            oldPosition;

        vec3 segmentDirection =
            normalize(
                segment
            );

        if (
            oldPosition.y *
            nextPosition.y <=
            0.0
        ) {
            float denominator =
                oldPosition.y -
                nextPosition.y;

            if (
                abs(denominator) >
                0.000001
            ) {
                float intersectionT =
                    oldPosition.y /
                    denominator;

                if (
                    intersectionT >= 0.0 &&
                    intersectionT <= 1.0
                ) {
                    vec3 intersection =
                        mix(
                            oldPosition,
                            nextPosition,
                            intersectionT
                        );

                    float diskRadius =
                        length(
                            intersection.xz
                        );

                    if (
                        diskRadius >
                        DISK_INNER_RADIUS &&
                        diskRadius <
                        DISK_OUTER_RADIUS
                    ) {
                        float radialMask =
                            diskMask(
                                diskRadius
                            );

                        float opacity =
                            radialMask *
                            0.82;

                        vec3 emission =
                            diskEmission(
                                intersection,
                                segmentDirection
                            );

                        color +=
                            transmittance *
                            emission *
                            opacity;

                        transmittance *=
                            1.0 -
                            opacity;

                        if (
                            transmittance <
                            0.035
                        ) {
                            transmittance =
                                0.0;

                            break;
                        }
                    }
                }
            }
        }

        finalDirection =
            directionFromState(
                nextU,
                nextDu,
                nextPhi,
                initialRadial,
                initialTangent
            );

        oldPosition =
            nextPosition;

        u =
            nextU;

        du =
            nextDu;

        phi =
            nextPhi;
    }

    if (
        !captured &&
        transmittance >
        0.0
    ) {
        color +=
            starField(
                finalDirection
            ) *
            transmittance;
    }

    color *= 1.15;

    float luminance =
        dot(
            color,
            vec3(
                0.2126,
                0.7152,
                0.0722
            )
        );

    color /=
        1.0 +
        luminance;

    color =
        clamp(
            color,
            vec3(0.0),
            vec3(1.0)
        );

    outColor =
        vec4(
            color,
            1.0
        );
}
