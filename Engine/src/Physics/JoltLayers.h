#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace JoltLayers {
	namespace Layers {
		static constexpr JPH::ObjectLayer STATIC = 0;
		static constexpr JPH::ObjectLayer DYNAMIC = 1;
		static constexpr JPH::ObjectLayer COUNT = 2;
	}

	namespace BroadPhaseLayers {
		static constexpr JPH::BroadPhaseLayer STATIC(0);
		static constexpr JPH::BroadPhaseLayer DYNAMIC(1);
		static constexpr JPH::uint COUNT = 2;
	}

	class BroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface {
	public:
		BroadPhaseLayerInterface() {
			mMap[Layers::STATIC] = BroadPhaseLayers::STATIC;
			mMap[Layers::DYNAMIC] = BroadPhaseLayers::DYNAMIC;
		}

		JPH::uint GetNumBroadPhaseLayers() const override {
			return BroadPhaseLayers::COUNT;
		}

		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
			return mMap[layer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
			switch ((JPH::BroadPhaseLayer::Type)layer) {
			case BroadPhaseLayers::STATIC: return "STATIC";
			case BroadPhaseLayers::DYNAMIC: return "DYNAMIC";
			default: return "UNKNOWN";
			}
		}
#endif

	private:
		JPH::BroadPhaseLayer mMap[Layers::COUNT];
	};

	class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter {
	public:
		bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
			return a == Layers::DYNAMIC || b == Layers::DYNAMIC;
		}
	};

	class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter {
	public:
		bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadPhaseLayer) const override {
			return layer == Layers::DYNAMIC || broadPhaseLayer == BroadPhaseLayers::DYNAMIC;
		}
	};
}
