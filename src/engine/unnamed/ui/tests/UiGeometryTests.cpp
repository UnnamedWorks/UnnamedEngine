#include "pch.h"
#include "UiGeometryTests.h"

#include "../geometry/UiGeometry.h"

namespace Unnamed {
	namespace {
		constexpr float kTestEpsilon = 0.0001f;

		[[nodiscard]]
		bool IsNearlyEqual(
			const float first,
			const float second
		) noexcept {
			return
				std::abs(first - second) <=
				kTestEpsilon;
		}

		void TestDefaultRect() {
			const UiRect rect;

			assert(rect.pos.x == 0.0f);
			assert(rect.pos.y == 0.0f);
			assert(rect.size.x == 0.0f);
			assert(rect.size.y == 0.0f);

			assert(rect.IsValid());
			assert(rect.IsEmpty());

			assert(!rect.Contains(Vec2(0.0f, 0.0f)));
		}

		void TestRectBounds() {
			constexpr UiRect rect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			const Vec2 min      = rect.GetMin();
			const Vec2 max      = rect.GetMax();
			const Vec2 center   = rect.GetCenter();
			const Vec2 halfSize = rect.GetHalfSize();

			assert(min.x == 10.0f);
			assert(min.y == 20.0f);

			assert(max.x == 110.0f);
			assert(max.y == 70.0f);

			assert(center.x == 60.0f);
			assert(center.y == 45.0f);

			assert(halfSize.x == 50.0f);
			assert(halfSize.y == 25.0f);

			assert(rect.GetLeft() == 10.0f);
			assert(rect.GetTop() == 20.0f);
			assert(rect.GetRight() == 110.0f);
			assert(rect.GetBottom() == 70.0f);

			assert(rect.GetWidth() == 100.0f);
			assert(rect.GetHeight() == 50.0f);

			assert(rect.IsValid());
			assert(!rect.IsEmpty());
		}

		void TestRectContains() {
			constexpr UiRect rect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			// 左上は含まれる。
			assert(rect.Contains(Vec2(10.0f, 20.0f)));

			assert(rect.Contains(Vec2(50.0f, 40.0f)));

			// 右端と下端は含まれない。
			assert(!rect.Contains(Vec2(110.0f, 40.0f)));
			assert(!rect.Contains(Vec2(50.0f, 70.0f)));

			assert(!rect.Contains(Vec2(9.99f, 40.0f)));
			assert(!rect.Contains(Vec2(50.0f, 19.99f)));
		}

		void TestInvalidRect() {
			constexpr UiRect negativeWidth{
				.pos  = Vec2(0.0f, 0.0f),
				.size = Vec2(-1.0f, 10.0f)
			};

			assert(!negativeWidth.IsValid());
			assert(negativeWidth.IsEmpty());

			assert(
				!negativeWidth.Contains(
					Vec2(0.0f, 0.0f)
				)
			);
		}

		void TestRectFromMinMax() {
			const UiRect rect =
				UiRect::FromMinMax(
					Vec2(10.0f, 20.0f),
					Vec2(50.0f, 80.0f)
				);

			assert(rect.pos.x == 10.0f);
			assert(rect.pos.y == 20.0f);
			assert(rect.size.x == 40.0f);
			assert(rect.size.y == 60.0f);
		}

		void TestThickness() {
			const UiThickness uniform =
				UiThickness::Uniform(8.0f);

			assert(uniform.left == 8.0f);
			assert(uniform.top == 8.0f);
			assert(uniform.right == 8.0f);
			assert(uniform.bottom == 8.0f);

			assert(uniform.GetHorizontal() == 16.0f);
			assert(uniform.GetVertical() == 16.0f);
			assert(uniform.IsFinite());

			const UiThickness symmetric =
				UiThickness::Symmetric(
					12.0f,
					6.0f
				);

			assert(symmetric.left == 12.0f);
			assert(symmetric.right == 12.0f);
			assert(symmetric.top == 6.0f);
			assert(symmetric.bottom == 6.0f);

			assert(
				symmetric.GetHorizontal() ==
				24.0f
			);

			assert(
				symmetric.GetVertical() ==
				12.0f
			);
		}

		void TestUniformCornerRadii() {
			const UiCornerRadii radii =
				UiCornerRadii::Uniform(16.0f);

			assert(radii.topLeft == 16.0f);
			assert(radii.topRight == 16.0f);
			assert(radii.bottomRight == 16.0f);
			assert(radii.bottomLeft == 16.0f);

			assert(radii.IsFinite());
		}

		void TestValidCornerRadiiRemainUnchanged() {
			constexpr UiCornerRadii radii{
				.topLeft     = 10.0f,
				.topRight    = 20.0f,
				.bottomRight = 30.0f,
				.bottomLeft  = 10.0f
			};

			const UiCornerRadii normalized =
				radii.GetNormalized(
					Vec2(100.0f, 100.0f)
				);

			assert(normalized.topLeft == 10.0f);
			assert(normalized.topRight == 20.0f);
			assert(normalized.bottomRight == 30.0f);
			assert(normalized.bottomLeft == 10.0f);
		}

		void TestCornerRadiiNormalization() {
			constexpr UiCornerRadii radii{
				.topLeft     = 80.0f,
				.topRight    = 40.0f,
				.bottomRight = 0.0f,
				.bottomLeft  = 0.0f
			};

			const UiCornerRadii normalized =
				radii.GetNormalized(
					Vec2(100.0f, 100.0f)
				);

			assert(
				IsNearlyEqual(
					normalized.topLeft,
					66.66667f
				)
			);

			assert(
				IsNearlyEqual(
					normalized.topRight,
					33.33333f
				)
			);

			assert(normalized.bottomRight == 0.0f);
			assert(normalized.bottomLeft == 0.0f);

			assert(
				IsNearlyEqual(
					normalized.topLeft +
					normalized.topRight,
					100.0f
				)
			);
		}

		void TestUniformCornerRadiiNormalization() {
			const UiCornerRadii radii =
				UiCornerRadii::Uniform(80.0f);

			const UiCornerRadii normalized =
				radii.GetNormalized(
					Vec2(100.0f, 50.0f)
				);

			// 高さ50pxなので、上下の半径合計は
			// 最大50pxまで縮小される。
			assert(
				IsNearlyEqual(
					normalized.topLeft,
					25.0f
				)
			);

			assert(
				IsNearlyEqual(
					normalized.topRight,
					25.0f
				)
			);

			assert(
				IsNearlyEqual(
					normalized.bottomRight,
					25.0f
				)
			);

			assert(
				IsNearlyEqual(
					normalized.bottomLeft,
					25.0f
				)
			);
		}

		void TestNegativeCornerRadii() {
			constexpr UiCornerRadii radii{
				.topLeft     = -10.0f,
				.topRight    = 20.0f,
				.bottomRight = -30.0f,
				.bottomLeft  = 40.0f
			};

			const UiCornerRadii normalized =
				radii.GetNormalized(
					Vec2(100.0f, 100.0f)
				);

			assert(normalized.topLeft == 0.0f);
			assert(normalized.topRight == 20.0f);
			assert(normalized.bottomRight == 0.0f);
			assert(normalized.bottomLeft == 40.0f);
		}

		void TestCapsuleRadii() {
			const UiCornerRadii horizontal =
				UiCornerRadii::Capsule(
					Vec2(100.0f, 40.0f)
				);

			assert(horizontal.topLeft == 20.0f);
			assert(horizontal.topRight == 20.0f);
			assert(horizontal.bottomRight == 20.0f);
			assert(horizontal.bottomLeft == 20.0f);

			const UiCornerRadii circle =
				UiCornerRadii::Capsule(
					Vec2(40.0f, 40.0f)
				);

			assert(circle.topLeft == 20.0f);
		}

		void TestRoundedRectNormalization() {
			const UiRoundedRect roundedRect{
				.rect = UiRect{
					.pos  = Vec2(10.0f, 20.0f),
					.size = Vec2(100.0f, 50.0f)
				},
				.radii =
				UiCornerRadii::Uniform(80.0f)
			};

			const UiRoundedRect normalized =
				roundedRect.GetNormalized();

			assert(normalized.rect.pos.x == 10.0f);
			assert(normalized.rect.pos.y == 20.0f);
			assert(normalized.rect.size.x == 100.0f);
			assert(normalized.rect.size.y == 50.0f);

			assert(
				IsNearlyEqual(
					normalized.radii.topLeft,
					25.0f
				)
			);

			assert(normalized.IsValid());
		}
	} // namespace

	void RunUiGeometryTests() {
		TestDefaultRect();
		TestRectBounds();
		TestRectContains();
		TestInvalidRect();
		TestRectFromMinMax();
		TestThickness();
		TestUniformCornerRadii();
		TestValidCornerRadiiRemainUnchanged();
		TestCornerRadiiNormalization();
		TestUniformCornerRadiiNormalization();
		TestNegativeCornerRadii();
		TestCapsuleRadii();
		TestRoundedRectNormalization();
	}
} // namespace Unnamed
