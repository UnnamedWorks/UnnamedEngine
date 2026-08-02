#include <pch.h>

#include <core/math/Math.h>

namespace Math {
	float Lerp(const float a, const float b, const float t) {
		return a * (1.0f - t) + b * t;
	}

	bool Equal(
		const float lhs, const float rhs, const float tolerance
	) noexcept {
		// NaN または Inf の場合は等しくないと判定する
		if (std::isnan(lhs) || std::isnan(rhs)) {
			return false;
		}

		if (std::isinf(lhs) || std::isinf(rhs)) {
			return std::isinf(lhs) &&
			       std::isinf(rhs) &&
			       std::signbit(lhs) == std::signbit(rhs);
		}

		return std::abs(lhs - rhs) <= tolerance;
	}

	[[nodiscard]] bool ExactlyEqual(
		const float first, const float second
	) noexcept {
		if (
			std::fpclassify(first) == FP_ZERO &&
			std::fpclassify(second) == FP_ZERO
		) {
			return true;
		}

		return std::bit_cast<uint32_t>(first) ==
		       std::bit_cast<uint32_t>(second);
	}

	float DeltaAngle(const float current, const float target) {
		float delta = std::fmod(target - current, 2.0f * pi);
		if (delta > pi) {
			delta -= 2.0f * pi;
		}
		if (delta < -pi) {
			delta += 2.0f * pi;
		}
		return delta;
	}

	float DeltaAngleDegrees(const float current, const float target) {
		float delta = std::fmod(target - current, 360.0f);
		if (delta > 180.0f) {
			delta -= 360.0f;
		}
		if (delta < -180.0f) {
			delta += 360.0f;
		}
		return delta;
	}

	float CubicBezier(const float t, const Vec2 p1, const Vec2 p2) {
		if (t <= 0.0f) {
			return 0.0f;
		}
		if (t >= 1.0f) {
			return 1.0f;
		}

		float         u      = t;
		constexpr int maxItr = 10;
		for (int i = 0; i < maxItr; i++) {
			const float oneMinusU = 1.0f - u;
			const float bezierX   = 3.0f * oneMinusU * oneMinusU * u * p1.x +
			                      3.0f * oneMinusU * u * u * p2.x +
			                      u * u * u;

			const float derivative = 3.0f * oneMinusU * oneMinusU * p1.x +
			                         6.0f * oneMinusU * u * (p2.x - p1.x) +
			                         3.0f * u * u * (1.0f - p2.x);

			if (std::fabs(derivative) < 1e-6f) {
				break;
			}

			const float diff = bezierX - t;
			u                -= diff / derivative;
		}

		// u を用いて y(u) を計算
		const float oneMinusU = 1.0f - u;
		const float bezierY   = 3.0f * oneMinusU * oneMinusU * u * p1.y +
		                      3.0f * oneMinusU * u * u * p2.y +
		                      u * u * u;
		return bezierY;
	}

	float CubicBezier(
		const float t,
		const float p1, const float p2,
		const float p3, const float p4
	) {
		return CubicBezier(t, Vec2(p1, p2), Vec2(p3, p4));
	}

	Vec2 Lerp(const Vec2& a, const Vec2& b, const float t) {
		return a * (1.0f - t) + b * t;
	}

	/// TODO: 使えるように修正
	/// @brief ワールド座標をスクリーン座標と画面中心からの角度に変換します
	/// @param worldPos 変換するワールド座標
	/// @param screenSize 画面サイズ
	/// @param bClamp 画面外の座標を画面端にクランプするか?
	/// @param margin 画面端からのマージン[px]
	/// @param outIsOffscreen 画面外にあるか?の結果
	/// @param outAngle 画面中心からの角度 [rad]
	/// @return スクリーン座標
	Vec2 WorldToScreen(
		const Vec3& worldPos, const Vec2 screenSize,
		const bool& bClamp, const float& margin,
		bool&       outIsOffscreen,
		float&      outAngle
	) {
		worldPos;
		screenSize;
		bClamp;
		margin;
		outIsOffscreen;
		outAngle;

		if (true) {
			return Vec2::zero;
		}

		// const Vec4 viewSpace = Vec4(worldPos, 1.0f) * camera->GetViewMat();
		// const Vec4 clipSpace = viewSpace * camera->GetProjMat();
		//
		// const Vec2 screenCenter = screenSize * 0.5f;
		//
		// // w除算を行いNDC空間に変換
		// const float invW = 1.0f / clipSpace.w;
		// const auto  ndc  = Vec3(
		// 	clipSpace.x * invW,
		// 	clipSpace.y * invW,
		// 	clipSpace.z * invW
		// );
		//
		// // スクリーン座標変換
		// auto screenPos = Vec2(
		// 	(ndc.x * 0.5f + 0.5f) * screenSize.x,
		// 	(1.0f - (ndc.y * 0.5f + 0.5f)) * screenSize.y
		// );
		//
		// // 画面中心からの方向ベクトル計算
		// const Vec2 direction = screenPos - screenCenter;
		//
		// // 角度計算
		// outAngle = std::atan2(direction.x, -direction.y);
		// if (viewSpace.z < 0.0f) { outAngle += pi; }
		// outAngle = std::fmod(outAngle + 2.0f * pi, 2.0f * pi);
		//
		// // 画面外か判定する
		// if (!bClamp) {
		// 	outIsOffscreen =
		// 		viewSpace.z < 0.0f ||
		// 		screenPos.x < 0.0f ||
		// 		screenPos.x > screenSize.x ||
		// 		screenPos.y < 0.0f ||
		// 		screenPos.y > screenSize.y;
		// 	return outIsOffscreen ? -Vec2::one * 0xFFFFFF : screenPos;
		// }
		//
		// // クランプ処理
		// outIsOffscreen = false;
		// if (
		// 	viewSpace.z < 0.0f ||
		// 	screenPos.x < margin ||
		// 	screenPos.x > screenSize.x - margin ||
		// 	screenPos.y < margin ||
		// 	screenPos.y > screenSize.y - margin
		// ) {
		// 	outIsOffscreen      = true;
		// 	Vec2 clampDirection = viewSpace.z < 0.0f ?
		// 		                      Vec2(viewSpace.x, -viewSpace.y) :
		// 		                      direction;
		//
		// 	const float length =
		// 		std::hypot(clampDirection.x, clampDirection.y);
		// 	if (length > 0.0f) {
		// 		clampDirection /= length; // 正規化
		// 	}
		//
		// 	const float screenRight  = screenSize.x - margin;
		// 	const float screenBottom = screenSize.y - margin;
		//
		// 	// 境界との交差点を計算
		// 	const std::vector tValues{
		// 		(margin - screenCenter.x) / clampDirection.x,
		// 		(screenRight - screenCenter.x) / clampDirection.x,
		// 		(margin - screenCenter.y) / clampDirection.y,
		// 		(screenBottom - screenCenter.y) / clampDirection.y
		// 	};
		//
		// 	float minT = std::numeric_limits<float>::max();
		// 	for (const float t : tValues) {
		// 		if (t > 0.0f && t < minT) { minT = t; }
		// 	}
		//
		// 	screenPos = screenCenter + clampDirection * minT;
		// 	screenPos.Clamp(
		// 		{margin, margin},
		// 		{std::max(screenRight, margin), std::max(screenBottom, margin)}
		// 	);
		// }
		//
		// return screenPos;
	}

	Vec3 ProjectOnPlane(const Vec3& vector, const Vec3& normal) {
		return vector - normal * vector.Dot(normal);
	}

	Vec3 GetMoveDirection(const Vec3& forward, const Vec3& groundNormal) {
		const Vec3 projectedForward = ProjectOnPlane(forward, groundNormal);
		return projectedForward.Normalized();
	}

	Vec3 Lerp(const Vec3& a, const Vec3& b, const float t) {
		return a * (1.0f - t) + b * t;
	}

	Vec3 Min(const Vec3 a, const Vec3 b) {
		return {
			std::min(a.x, b.x),
			std::min(a.y, b.y),
			std::min(a.z, b.z)
		};
	}

	Vec3 Max(const Vec3 a, const Vec3 b) {
		return {
			std::max(a.x, b.x),
			std::max(a.y, b.y),
			std::max(a.z, b.z)
		};
	}

	Vec4 Lerp(const Vec4& a, const Vec4& b, const float t) {
		return a * (1.0f - t) + b * t;
	}

	Vec3 HtoM(const Vec3& vec) {
		return vec * 0.0254f;
	}

	float HtoM(const float val) {
		return val * 0.0254f;
	}

	Vec3 MtoH(const Vec3& vec) {
		return vec * 39.3701f;
	}

	float MtoH(const float val) {
		return val * 39.3701f;
	}
}
