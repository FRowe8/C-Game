#include "Types.h"
#include <cmath>

// Vec2 implementation
f32 Vec2::Length() const {
    return std::sqrt(x * x + y * y);
}

Vec2 Vec2::Normalized() const {
    f32 len = Length();
    if (len > 0.0f) {
        return Vec2(x / len, y / len);
    }
    return Vec2(0, 0);
}

f32 Vec2::Dot(const Vec2& other) const {
    return x * other.x + y * other.y;
}

// Vec3 implementation
f32 Vec3::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::Normalized() const {
    f32 len = Length();
    if (len > 0.0f) {
        return Vec3(x / len, y / len, z / len);
    }
    return Vec3(0, 0, 0);
}

f32 Vec3::Dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::Cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}
