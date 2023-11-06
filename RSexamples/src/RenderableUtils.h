#pragma once

#include <rsids.h>
#include <MakeID.h>
#include <glm/glm.hpp>
#include <unordered_map>

namespace ss 
{
	MAKEID_TYPE(BoundsID);
	MAKEID_TYPE(GridID);
	MAKEID_TYPE(TriadID);

	struct GridInfo 
	{
		float size = 10.0f;
		float triadScale = 1.0f;
		uint32_t resolution = 10;
		glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	};

	struct BoundsInfo 
	{
		float width, height, depth;
		glm::vec4 color = glm::vec4(0.2f, 0.8f, 0.0f, 1.0f);
		glm::mat4 xform = glm::mat4(1.0f);
	};

	struct TriadInfo
	{
		float size = 1.0f;
		glm::mat4 xform = glm::mat4(1.0f);
	};

	class RenderableUtils final
	{
	private:
		struct RSlineIDs
		{
			RSgeometryDataID geomDataID;
			RSgeometryID geomID;
			RSappearanceID appID;
			RSinstanceID instanceID;
			RSstateID stateID;
			RSspatialID spatialID;
		};

		struct RUgrid
		{
			GridInfo gi;
			RSlineIDs lineIDs;
		};

		struct RUbounds 
		{
			BoundsInfo bi;
			RSlineIDs lineIDs;
		};

		struct RUtriad
		{
			TriadInfo tinfo;
			RSlineIDs lineIDs;
		};

		static std::unordered_map<GridID, RUgrid, IDHasher<GridID>> _gridMap;
		static std::unordered_map<BoundsID, RUbounds, IDHasher<BoundsID>> _boundsMap;
		static std::unordered_map<TriadID, RUtriad, IDHasher<TriadID>> _triadMap;

		static MakeID _gridIDpool;
		static MakeID _boundsIDpool;
		static MakeID _triadIDpool;

		static void dispose(RSlineIDs& lineIDs, RScollectionID& collectionID);

	public:
		static GridID GridCreate(const GridInfo& gi, const RScollectionID& collectionID);
		static void GridDispose(const GridID& gridID, RScollectionID& collectionID);

		static TriadID triadCreate(const TriadInfo& ti, const RScollectionID collectionID);
		static void triadDispose(const TriadID& triadID, RScollectionID& collectionID);

		static BoundsID boundsCreate(const BoundsInfo& boundsInfo, const RScollectionID& collectionID);
		static void boundsDispose(const BoundsID& boundsID, RScollectionID& collectionID);
	};
}
