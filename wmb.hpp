#ifndef WMB_HPP
#define WMB_HPP

#include <glm/glm.hpp>
#include <string>
#include <optional>
#include <vector>
#include <cstddef>
#include <variant>
#include <array>
#include <bitset>

namespace WMB
{
	struct Euler
	{
		float pan, tilt, roll;
	};

	struct Texture
	{
		enum Format
		{
			RGBA8888 = 5,
			RGB888 = 4,
			RGB565 = 2,
			DDS = 6
		};

		std::string name;
		unsigned int width, height;
		Format format;
		bool hasMipMaps;
		std::vector<std::vector<std::byte>> levels;

		std::vector<std::byte> & data() {
			return levels.at(0);
		}

		std::vector<std::byte> const & data() const {
			return levels.at(0);
		}
	};

	struct Lightmap
	{
		unsigned int width, height;
		std::optional<unsigned int> object; // object for terrain lightmap or nullopt
		std::vector<std::byte> data; // encoded in BGR
	};

	struct Material
	{
		std::string name;
		bool isDefault;
	};

	struct Vertex
	{
		glm::vec3 position; // position
	    glm::vec2 uv; // texture coordinates
	    glm::vec2 lightmap; // lightmap coordinates
	};

	struct Triangle
	{
		uint16_t v1,v2,v3; // indices into the VERTEX array
	    uint16_t skin;     // index into the SKIN array
	};

	struct Skin
	{
		enum Flags
		{
			FLAT = 1,
			SKY = 2,
			PASSABLE = 6,
			SMOOTH = 14,


			FLAG1 = 16,
			FLAG2 = 17,
			FLAG3 = 18,
			FLAG4 = 19,
			FLAG5 = 20,
			FLAG6 = 21,
			FLAG7 = 22,
			FLAG8 = 23,
		};

		uint16_t texture;  // index into the textures list
	    uint16_t lightmap; // index into the lightmaps list
	    unsigned int material; // index into the MATERIAL_INFO array
	    float ambient, albedo;
	    std::bitset<32> flags; // bit 1 = flat (no lightmap), bit 2 = sky, bit 14 = smooth

		bool isFlat() const { return flags.test(FLAT); }
		bool isSky() const { return flags.test(SKY); }
		bool isSmooth() const { return flags.test(SMOOTH); }
	};

	struct Block
	{
		glm::vec3 bbMin; // bounding box
	    glm::vec3 bbMax; // bounding box
		std::vector<Vertex> vertices;
		std::vector<Triangle> triangles;
		std::vector<Skin> skins;
	};

	struct Info
	{
		float azimuth;   // sun azimuth
		float elevation; // sun elevation
		float gamma;     // light level at black
		unsigned int  lightMapSize;
		glm::vec4 sunColor, ambientColor; // color double word, ARGB
		glm::vec4 fogColor[4];
	};

	struct Position
	{
		std::string name;
		glm::vec3 origin;
		Euler angle;
	};

	struct Light
	{
		enum Flags
		{
			HIGHRES = 0,
			DYNAMIC = 1,
			STATIC  = 2,
			CAST    = 3,
		};
		glm::vec3 origin;
		glm::vec3 color;
		float range;
		std::bitset<32> flags;

		bool isHighRes() const {
			return flags.test(HIGHRES);
		}

		bool isDynamic() const {
			return flags.test(DYNAMIC);
		}

		bool isStatic() const {
			return flags.test(STATIC);
		}

		bool isCasting() const {
			return flags.test(CAST);
		}
	};

	struct Sound
	{
		glm::vec3 origin;
		float volume;
		long range;
		std::bitset<32> flags; // hurr?
		std::string fileName;
	};

	struct PathNode
	{
		glm::vec3 position;
		std::array<float, 6> skills;
	};

	struct PathEdge
	{
		unsigned int node1, node2; // node numbers of the edge, starting with 1
		float length;
		float bezier;
		float weight;
		float skill;
	};

	struct Path
	{
		std::string name;
		std::vector<PathNode> nodes;
		std::vector<PathEdge> edges;
	};

	struct Entity
	{
		enum Flags
		{
			FLAG1       =  0,
			FLAG2       =  1,
			FLAG3       =  2,
			FLAG4       =  3,
			FLAG5       =  4,
			FLAG6       =  5,
			FLAG7       =  6,
			FLAG8       =  7,
			INVISIBLE   =  8,
			PASSABLE    =  9,
			TRANSLUCENT = 10, // transparent
			OVERLAY     = 12, // for models and panels
			SPOTLIGHT   = 13,
			ZNEAR	    = 14,
			NOFILTER    = 16, // point filtering
			UNLIT	    = 17,	// no light from environment
			SHADOW	    = 18,	// cast dynamic shadows
			LIGHT	    = 19,	// tinted by own light color
			NOFOG	    = 20,	// ignores fog
			BRIGHT	    = 21,	// additive blending
			DECAL	    = 22,	// sprite without backside
			METAL	    = 22,	// use metal material
			CAST	    = 23,	// don't receive shadows
			POLYGON     = 26,	// polygonal collision detection
		};

		bool isOldEntity;
	    glm::vec3 origin;
	    Euler angle;
	    glm::vec3 scale;
	    std::string name;
	    std::string  fileName;
	    std::string action;
	    std::array<float, 20> skill;
	    std::bitset<32> flags;
	    float ambient;
	    float albedo;
	    std::optional<unsigned long> path;    // attached path index
	    std::optional<unsigned long> attachedEntity; // attached entity index
	    std::string material;
	    std::string string1;
	    std::string string2;
	};

	struct Region
	{
		std::string name;
		glm::vec3 minimum;
		glm::vec3 maximum;
	};

	enum class ObjectType
	{
		Position = 0,
		Light = 1,
		Sound = 2,
		Path = 3,
		Entity = 4,
		Region = 5,
	};

	struct Level
	{
		Info info;

		std::vector<Texture> textures;
		std::vector<Material> materials;
		std::vector<Lightmap> lightmaps;
		std::vector<Lightmap> terrain_lightmaps;
		std::vector<Block> blocks;

		std::vector<std::variant<
			Position,
			Light,
			Sound,
			Path,
			Entity,
			Region
		>> objects;
	};

	struct LoadOptions
	{
		enum CoordinateSystem
		{
			Gamestudio = 0,
			OpenGL = 1,
			DirectX = 2,
		};

		enum Flags
		{
			LOG_WARNINGS = 0,
			LOG_ERRORS = 1,
			LOG_VERBOSE = 2
		};

		//! Converts the WMB coordinates into the given coordinate system.
		CoordinateSystem targetCoordinateSystem = Gamestudio;

		std::bitset<3> flags = LOG_WARNINGS | LOG_ERRORS;


		bool log_warnings() const { return flags.test(LOG_WARNINGS); }
		bool log_errors()   const { return flags.test(LOG_ERRORS); }
		bool log_verbose()  const { return flags.test(LOG_VERBOSE); }
	};

	std::optional<Level> load(std::string const & fileName, LoadOptions const & options = LoadOptions());
}

#endif // WMB_HPP
