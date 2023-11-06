#include "RenderableUtils.h"
#include <VkRenderSystem.h>
#include <cstdint>

namespace ss 
{
	MakeID RenderableUtils::_gridIDpool = MakeID(UINT32_MAX);
	MakeID RenderableUtils::_boundsIDpool = MakeID(UINT32_MAX);
	MakeID RenderableUtils::_triadIDpool = MakeID(UINT32_MAX);

	std::unordered_map<GridID, RenderableUtils::RUgrid, IDHasher<GridID>> RenderableUtils::_gridMap;
	std::unordered_map<BoundsID, RenderableUtils::RUbounds, IDHasher<BoundsID>> RenderableUtils::_boundsMap;
	std::unordered_map<TriadID, RenderableUtils::RUtriad, IDHasher<TriadID>> RenderableUtils::_triadMap;

	void RenderableUtils::dispose(RSlineIDs& lineIDs, RScollectionID& collectionID)
	{
		auto& vkrs = VkRenderSystem::getInstance();

		if (lineIDs.geomDataID.isValid())
		{
			vkrs.geometryDataDispose(lineIDs.geomDataID);
		}

		if (lineIDs.geomID.isValid())
		{
			vkrs.geometryDispose(lineIDs.geomID);
		}

		if (lineIDs.instanceID.isValid())
		{
			vkrs.collectionInstanceDispose(collectionID, lineIDs.instanceID);
		}

		if (lineIDs.appID.isValid())
		{
			vkrs.appearanceDispose(lineIDs.appID);
		}
	}

	GridID RenderableUtils::GridCreate(const GridInfo& gi, const RScollectionID& collectionID)
	{
		GridID outID;
		RUgrid rugrid;
		uint32_t id;
		bool success = _gridIDpool.CreateID(id);
		if (success)
		{
			outID.id = id;
			rugrid.gi = gi;
		}

		//create the geometry
		float halfsz = gi.size * 0.5f;
		std::vector<glm::vec4> positions = 
		{
			{ glm::vec4(-halfsz, 0.0f,  halfsz, 1.0f) },
			{ glm::vec4( halfsz, 0.0f,  halfsz, 1.0f) },

			{ glm::vec4( halfsz, 0.0f,  halfsz, 1.0f) },
			{ glm::vec4( halfsz, 0.0f, -halfsz, 1.0f) },

			{ glm::vec4( halfsz, 0.0f, -halfsz, 1.0f) },
			{ glm::vec4(-halfsz, 0.0f, -halfsz, 1.0f) },

			{ glm::vec4(-halfsz, 0.0f, -halfsz, 1.0f) },
			{ glm::vec4(-halfsz, 0.0f,  halfsz, 1.0f) }
		};

		float spacing = gi.size / float(gi.resolution-1);
		//vertical lines
		for (uint32_t i = 1; i < gi.resolution; i++)
		{
			positions.push_back( glm::vec4(-halfsz + i * spacing, 0, -halfsz, 1.0f) );
			positions.push_back( glm::vec4(-halfsz + i * spacing, 0,  halfsz, 1.0f) );
		}
		
		//horizontal lines
		for (uint32_t i = 1; i < gi.resolution; i++)
		{
			positions.push_back( glm::vec4(-halfsz, 0,  -halfsz + i * spacing, 1.0f) );
			positions.push_back( glm::vec4( halfsz, 0,  -halfsz + i * spacing, 1.0f) );
		}

		std::vector<glm::vec4> colors(positions.size(), gi.color);

		auto& vkrs = VkRenderSystem::getInstance();
		RSvertexAttribsInfo attribsInfo;
		std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor };
		attribsInfo.attributes = attribs.data();
		attribsInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
		attribsInfo.settings = RSvertexAttributeSettings::vasSeparate;
		
		uint32_t numVertices = static_cast<uint32_t>(positions.size());
		vkrs.geometryDataCreate(rugrid.lineIDs.geomDataID, numVertices, 0, attribsInfo);
		uint32_t szinbytes = numVertices * sizeof(positions[0]);
		vkrs.geometryDataUpdateVertices(rugrid.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaPosition, positions.data());
		vkrs.geometryDataUpdateVertices(rugrid.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaColor, colors.data());
		vkrs.geometryDataFinalize(rugrid.lineIDs.geomDataID);

		RSgeometryInfo geomInfo;
		geomInfo.primType = RSprimitiveType::ptLine;
		vkrs.geometryCreate(rugrid.lineIDs.geomID, geomInfo);

		//create the instance and add to collection
		RSappearanceInfo appInfo;
		appInfo.shaderTemplate = RSshaderTemplate::stLines;
		vkrs.appearanceCreate(rugrid.lineIDs.appID, appInfo);

		RSinstanceInfo instInfo;
		instInfo.appID = rugrid.lineIDs.appID;
		instInfo.gdataID = rugrid.lineIDs.geomDataID;
		instInfo.geomID = rugrid.lineIDs.geomID;

		vkrs.collectionInstanceCreate(collectionID, rugrid.lineIDs.instanceID, instInfo);

		_gridMap[outID] = rugrid;

		return outID;
	}

	void RenderableUtils::GridDispose(const GridID& gridID, RScollectionID& collectionID)
	{
		if (gridID.isValid()) 
		{
			auto& vkrs = VkRenderSystem::getInstance();
			RUgrid& rugrid = _gridMap[gridID];
			dispose(rugrid.lineIDs, collectionID);

			_gridMap.erase(gridID);
		}
	}

	TriadID RenderableUtils::triadCreate(const TriadInfo& ti, const RScollectionID collectionID)
	{
		uint32_t id;
		RUtriad rutriad;
		TriadID outID;
		bool success = _triadIDpool.CreateID(id);
		if (success)
		{
			outID.id = id;
			rutriad.tinfo = ti;
		}

		std::vector<glm::vec4> positions =
		{
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f * ti.size, 0.0f, 0.0f, 1.0f),

			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f * ti.size, 0.0f, 1.0f),

			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f * ti.size, 1.0f),
		};

		std::vector<glm::vec4> colors =
		{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),

			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),

			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
		};

		auto& vkrs = VkRenderSystem::getInstance();
		uint32_t numVertices = static_cast<uint32_t>(positions.size());
		RSvertexAttribsInfo attribInfo;
		std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor };
		attribInfo.attributes = attribs.data();
		attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
		attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());

		vkrs.geometryDataCreate(rutriad.lineIDs.geomDataID, numVertices, 0, attribInfo);
		uint32_t szinbytes = static_cast<uint32_t>(positions.size() * sizeof(positions[0]));
		vkrs.geometryDataUpdateVertices(rutriad.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaPosition, positions.data());
		vkrs.geometryDataUpdateVertices(rutriad.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaColor, colors.data());
		vkrs.geometryDataFinalize(rutriad.lineIDs.geomDataID);

		RSgeometryInfo geomInfo;
		geomInfo.primType = RSprimitiveType::ptLine;
		vkrs.geometryCreate(rutriad.lineIDs.geomID, geomInfo);

		RSappearanceInfo appInfo;
		appInfo.shaderTemplate = RSshaderTemplate::stLines;
		vkrs.appearanceCreate(rutriad.lineIDs.appID, appInfo);

		RSspatial spatial;
		spatial.model = rutriad.tinfo.xform;
		spatial.modelInv = glm::inverse(spatial.model);
		vkrs.spatialCreate(rutriad.lineIDs.spatialID, spatial);

		RSstate state;
		state.depthState.depthFunc = RSdepthFunction::dsAlway;
		state.lnstate.lineWidth = 3.0f;
		vkrs.stateCreate(rutriad.lineIDs.stateID, state);
		
		RSinstanceInfo instInfo;
		instInfo.appID = rutriad.lineIDs.appID;
		instInfo.gdataID = rutriad.lineIDs.geomDataID;
		instInfo.geomID = rutriad.lineIDs.geomID;
		instInfo.spatialID = rutriad.lineIDs.spatialID;
		instInfo.stateID = rutriad.lineIDs.stateID;

		vkrs.collectionInstanceCreate(collectionID, rutriad.lineIDs.instanceID, instInfo);

		_triadMap[outID] = rutriad;

		return outID;
	}

	void RenderableUtils::triadDispose(const TriadID& triadID, RScollectionID& collectionID)
	{
		if (triadID.isValid())
		{
			RUtriad& rutriad = _triadMap[triadID];
			dispose(rutriad.lineIDs, collectionID);

			_triadMap.erase(triadID);
		}
	}

	BoundsID RenderableUtils::boundsCreate(const BoundsInfo& boundsInfo, const RScollectionID& collectionID)
	{
		BoundsID outID;

		return outID;
	}

	void RenderableUtils::boundsDispose(const BoundsID& boundsID, RScollectionID& collectionID)
	{

	}
}

