# Math Conventions

## Types

| Type       | Float         | Double     |
| ---------- | ------------- | ---------- |
| Vector2    | `Vector2f`    | `Vector2d` |
| Vector3    | `Vector3f`    | `Vector3d` |
| Vector4    | `Vector4f`    | `Vector4d` |
| Matrix4    | `Matrix4f`    | `Matrix4d` |
| Quaternion | `Quaternionf` | —          |

Both precisions are supported. The renderer currently uses `float`.

Precision conversions across subsystem boundaries must be explicit. Conversion functions and their error handling are not implemented yet.

## Matrices

- Column-major storage
- Column vectors
- Elements accessed using `at(row, column)`
- Angles in radians
- Right-handed axis rotations
- Default construction produces a zero matrix

Storage index:

```cpp
column * 4 + row
```

Matrix multiplication applies the right operand first.

`composeTRS` uses:

```text
T * Rz * Ry * Rx * S
```

`transformPoint` includes translation. `transformDirection` ignores translation. Both functions use affine transformation semantics.

## Numerical behavior

### Normalization

`normalize` requires a positive, finite vector magnitude. Invalid magnitudes trigger a precondition assertion.

### Inversion

`tryInverse` uses Gauss-Jordan elimination with partial pivoting.

Returns `std::nullopt` for non-finite input, zero pivots, or detected non-finite results.

Near-singular matrices may produce inaccurate inverses. No condition-number estimation is implemented.

### Comparisons

Tests use exact comparisons where appropriate and tolerances for floating-point operations.

There is no global epsilon. Tolerances depend on the operation and expected numerical error.

## Shader layout

The Vulkan renderer uploads `Matrix4f` directly through push constants.

- Matrix size: 64 bytes
- Matrix member offset: 0 bytes
- Matrix stride: 16 bytes
- CPU storage: column-major
- Shader type: Slang `float4x4`
- Shader multiplication: `mul(matrix, vector)`

Shaders are compiled with `-matrix-layout-column-major`.

The generated SPIR-V uses `RowMajor` and `OpVectorTimesMatrix`.
