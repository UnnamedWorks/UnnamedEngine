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

			UASSERT(rect.pos.x == 0.0f);
			UASSERT(rect.pos.y == 0.0f);
			UASSERT(rect.size.x == 0.0f);
			UASSERT(rect.size.y == 0.0f);

			UASSERT(rect.IsValid());
			UASSERT(rect.IsEmpty());

			UASSERT(!rect.Contains(Vec2(0.0f, 0.0f)));
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

			UASSERT(min.x == 10.0f);
			UASSERT(min.y == 20.0f);

			UASSERT(max.x == 110.0f);
			UASSERT(max.y == 70.0f);

			UASSERT(center.x == 60.0f);
			UASSERT(center.y == 45.0f);

			UASSERT(halfSize.x == 50.0f);
			UASSERT(halfSize.y == 25.0f);

			UASSERT(rect.GetLeft() == 10.0f);
			UASSERT(rect.GetTop() == 20.0f);
			UASSERT(rect.GetRight() == 110.0f);
			UASSERT(rect.GetBottom() == 70.0f);

			UASSERT(rect.GetWidth() == 100.0f);
			UASSERT(rect.GetHeight() == 50.0f);

			UASSERT(rect.IsValid());
			UASSERT(!rect.IsEmpty());
		}

		void TestRectContains() {
			constexpr UiRect rect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			// 左上は含まれる。
			UASSERT(rect.Contains(Vec2(10.0f, 20.0f)));

			UASSERT(rect.Contains(Vec2(50.0f, 40.0f)));

			// 右端と下端は含まれない。
			UASSERT(!rect.Contains(Vec2(110.0f, 40.0f)));
			UASSERT(!rect.Contains(Vec2(50.0f, 70.0f)));

			UASSERT(!rect.Contains(Vec2(9.99f, 40.0f)));
			UASSERT(!rect.Contains(Vec2(50.0f, 19.99f)));
		}

		void TestInvalidRect() {
			constexpr UiRect negativeWidth{
				.pos  = Vec2(0.0f, 0.0f),
				.size = Vec2(-1.0f, 10.0f)
			};

			UASSERT(!negativeWidth.IsValid());
			UASSERT(negativeWidth.IsEmpty());

			UASSERT(
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

			UASSERT(rect.pos.x == 10.0f);
			UASSERT(rect.pos.y == 20.0f);
			UASSERT(rect.size.x == 40.0f);
			UASSERT(rect.size.y == 60.0f);
		}

		void TestThickness() {
			const UiThickness uniform =
				UiThickness::Uniform(8.0f);

			UASSERT(uniform.left == 8.0f);
			UASSERT(uniform.top == 8.0f);
			UASSERT(uniform.right == 8.0f);
			UASSERT(uniform.bottom == 8.0f);

			UASSERT(uniform.GetHorizontal() == 16.0f);
			UASSERT(uniform.GetVertical() == 16.0f);
			UASSERT(uniform.IsFinite());

			const UiThickness symmetric =
				UiThickness::Symmetric(
					12.0f,
					6.0f
				);

			UASSERT(symmetric.left == 12.0f);
			UASSERT(symmetric.right == 12.0f);
			UASSERT(symmetric.top == 6.0f);
			UASSERT(symmetric.bottom == 6.0f);

			UASSERT(
				symmetric.GetHorizontal() ==
				24.0f
			);

			UASSERT(
				symmetric.GetVertical() ==
				12.0f
			);
		}

		void TestUniformCornerRadii() {
			const UiCornerRadii radii =
				UiCornerRadii::Uniform(16.0f);

			UASSERT(radii.topLeft == 16.0f);
			UASSERT(radii.topRight == 16.0f);
			UASSERT(radii.bottomRight == 16.0f);
			UASSERT(radii.bottomLeft == 16.0f);

			UASSERT(radii.IsFinite());
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

			UASSERT(normalized.topLeft == 10.0f);
			UASSERT(normalized.topRight == 20.0f);
			UASSERT(normalized.bottomRight == 30.0f);
			UASSERT(normalized.bottomLeft == 10.0f);
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

			UASSERT(
				IsNearlyEqual(
					normalized.topLeft,
					66.66667f
				)
			);

			UASSERT(
				IsNearlyEqual(
					normalized.topRight,
					33.33333f
				)
			);

			UASSERT(normalized.bottomRight == 0.0f);
			UASSERT(normalized.bottomLeft == 0.0f);

			UASSERT(
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
			UASSERT(
				IsNearlyEqual(
					normalized.topLeft,
					25.0f
				)
			);

			UASSERT(
				IsNearlyEqual(
					normalized.topRight,
					25.0f
				)
			);

			UASSERT(
				IsNearlyEqual(
					normalized.bottomRight,
					25.0f
				)
			);

			UASSERT(
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

			UASSERT(normalized.topLeft == 0.0f);
			UASSERT(normalized.topRight == 20.0f);
			UASSERT(normalized.bottomRight == 0.0f);
			UASSERT(normalized.bottomLeft == 40.0f);
		}

		void TestCapsuleRadii() {
			const UiCornerRadii horizontal =
				UiCornerRadii::Capsule(
					Vec2(100.0f, 40.0f)
				);

			UASSERT(horizontal.topLeft == 20.0f);
			UASSERT(horizontal.topRight == 20.0f);
			UASSERT(horizontal.bottomRight == 20.0f);
			UASSERT(horizontal.bottomLeft == 20.0f);

			const UiCornerRadii circle =
				UiCornerRadii::Capsule(
					Vec2(40.0f, 40.0f)
				);

			UASSERT(circle.topLeft == 20.0f);
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

			UASSERT(normalized.rect.pos.x == 10.0f);
			UASSERT(normalized.rect.pos.y == 20.0f);
			UASSERT(normalized.rect.size.x == 100.0f);
			UASSERT(normalized.rect.size.y == 50.0f);

			UASSERT(
				IsNearlyEqual(
					normalized.radii.topLeft,
					25.0f
				)
			);

			UASSERT(normalized.IsValid());
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
