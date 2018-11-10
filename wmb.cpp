#include "wmb.hpp"

#include <type_traits>
#include <cstdint>
#include <vector>
#include <exception>

#include <cstring>

#include <glm/gtc/matrix_transform.hpp>

using namespace WMB;

struct __attribute__((packed)) LIST
{
	//! offset of the list from the start of the WMB file, in bytes
	uint32_t offset;

	//! length of the list, in bytes
	uint32_t length;
};

struct WMB_HEADER
{
	//! "WMB7"
	std::array<char, 4> version;
	LIST palettes;// WMB1..6 only
	LIST legacy1; // WMB1..6 only
	LIST textures;// textures list
	LIST legacy2; // WMB1..6 only
	LIST pvs;     // BSP only
	LIST bsp_nodes; // BSP only
	LIST materials; // material names
	LIST legacy3; // WMB1..6 only
	LIST legacy4; // WMB1..6 only
	LIST aabb_hulls; // WMB1..6 only
	LIST bsp_leafs;  // BSP only
	LIST bsp_blocks; // BSP only
	LIST legacy5; // WMB1..6 only
	LIST legacy6; // WMB1..6 only
	LIST legacy7; // WMB1..6 only
	LIST objects; // entities, paths, sounds, etc.
	LIST lightmaps; // lightmaps for blocks
	LIST blocks;  // block meshes
	LIST legacy8; // WMB1..6 only
	LIST lightmaps_terrain; // lightmaps for terrains
};

struct __attribute__((packed)) TEXTURE
{
	std::array<char, 16> name;   // texture name, max. 16 characters
	uint32_t width,height; // texture size
	uint32_t type;	    // texture type: 5 = 8888 RGBA; 4 = 888 RGB; 2 = 565 RGB; 6 = DDS; +8 = mipmaps
	uint32_t legacy[3]; // always 0
};

struct MATERIAL_INFO
{
	std::array<char, 44> legacy;   // always 0
	std::array<char, 20> material; // material name from the script, max. 20 characters
};

static_assert(sizeof(MATERIAL_INFO) == 64);

////////////////////////////////////////////////////////////////////////////////

enum class OBJECT_TYPE : uint32_t
{
	Position = 1,
	Light = 2,
	OldEntity = 3,
	Sound = 4,
	Info = 5,
	Path = 6,
	Entity = 7,
	Region = 8,
};


struct __attribute__((packed)) WMB_INFO
{
	// uint32_t  type;      // 5 = INFO
	std::array<float, 3> origin; // not used
	float azimuth;   // sun azimuth
	float elevation; // sun elevation
	uint32_t  flags;     // always 127 (0x7F)
	float version;	 // compiler version
	std::uint8_t  gamma;     // light level at black
	std::uint8_t  LMapSize;	 // 0,1,2 for lightmap sizes 256x256, 512x512, or 1024x1024
	uint32_t  unused[2];
	uint32_t dwSunColor, dwAmbientColor; // color double word, ARGB
	uint32_t dwFogColor[4];
};

struct __attribute__((packed)) WMB_POSITION
{
	// long  type;      // 1 = POSITION
	std::array<float, 3> origin;
	std::array<float, 3> angle;
	uint32_t  unused[2];
	std::array<char, 20>  name;
};

struct __attribute__((packed)) WMB_LIGHT
{
	// long  type;      // 2 = LIGHT
	std::array<float, 3> origin;
	float red,green,blue; // color in percent, 0..100
	float range;
	uint32_t  flags;     // 0 = static, 2 = dynamic
};

struct __attribute__((packed)) WMB_SOUND
{
	// long  type;      // 4 = Sound
	std::array<float, 3> origin;
	float volume;
	float unused[2];
	uint32_t  range;
	uint32_t  flags;    // always 0
	std::array<char,33> filename;
};

struct __attribute__((packed)) WMB_PATH
{
	// long  type;		 // 6 = PATH
	std::array<char, 20> name;	 // Path name
	float fNumPoints;// number of nodes
	uint32_t  unused[3]; // always 0
	uint32_t  num_edges;
};

struct __attribute__((packed)) PATH_EDGE
{
	float fNode1,fNode2; // node numbers of the edge, starting with 1
	float fLength;
	float fBezier;
	float fWeight;
	float fSkill;
};

struct __attribute__((packed)) WMB_ENTITY
{
	// long  type;     // 7 = ENTITY
	std::array<float, 3> origin;
	std::array<float, 3> angle;
	std::array<float, 3> scale;
	std::array<char, 33>  name;
	std::array<char, 33>  filename;
	std::array<char, 33>  action;
	uint8_t unused1;
	std::array<float, 20> skill;
	uint32_t  flags;
	float ambient;
	float albedo;
	int32_t  path;    // attached path index, starting with 1, or 0 for no path
	uint32_t  entity2; // attached entity index, starting with 1, or 0 for no attached entity
	std::array<char, 33> material;
	std::array<char, 33> string1;
	std::array<char, 33> string2;
	char  unused2[33];
};

struct __attribute__((packed)) WMB_OLD_ENTITY
{
	// long  type;     // 3 = OLD ENTITY
	std::array<float, 3> origin;
	std::array<float, 3> angle;
	std::array<float, 3> scale;
	std::array<char, 20> name;
	std::array<char, 13> filename;
	std::array<char, 20> action;
	std::array<float, 8> skill;
	uint32_t  flags;
	float ambient;
};

////////////////////////////////////////////////////////////////////////////////

struct __attribute__((packed)) BLOCK
{
	std::array<float, 3> fMins; // bounding box
	std::array<float, 3> fMaxs; // bounding box
	uint32_t lContent;  // always 0
	uint32_t lNumVerts; // number of VERTEX structs that follow
	uint32_t lNumTris;  // number of TRIANGLE structs that follow
	uint32_t lNumSkins; // number of SKIN structs that follow
};

struct __attribute__((packed)) VERTEX
{
	float x,y,z; // position
	float tu,tv; // texture coordinates
	float su,sv; // lightmap coordinates
};

struct __attribute__((packed)) TRIANGLE
{
	uint16_t v1,v2,v3; // indices into the VERTEX array
	uint16_t skin;  // index into the SKIN array
	uint32_t unused; // always 0
};

struct __attribute__((packed)) SKIN
{
	uint16_t texture;  // index into the textures list
	uint16_t lightmap; // index into the lightmaps list
	uint32_t material; // index into the MATERIAL_INFO array
	float ambient,albedo;
	uint32_t flags;     // bit 1 = flat (no lightmap), bit 2 = sky, bit 14 = smooth
};

struct __attribute__((packed)) LIGHTMAP_TERRAIN
{
	uint32_t object; // terrain entity index into the objects list
	uint32_t width, height; // lightmap size
};

struct __attribute__((packed)) REGION
{
	std::array<float, 3> min;
	std::array<float, 3> max;
	uint32_t val_a;
	uint32_t val_b;
	std::array<char, 32> name;
};

struct File
{
	FILE * f;

	File(FILE * f) : f(f)
	{

	}

	File(File const &) = delete;
	File(File &&) = delete;

	~File()
	{
		if(f != nullptr)
			fclose(f);
	}

	operator bool() const
	{
		return (f != nullptr);
	}

	operator FILE* ()
	{
		return f;
	}

	void seek(long offset, int mode = SEEK_SET)
	{
		fseek(f, offset, mode);
	}

	template<typename T>
	typename std::enable_if<std::is_trivially_constructible<T>::value, T>::type read()
	{
		uint8_t buffer[sizeof(T)];

		size_t offset = 0;
		while(offset < sizeof(T))
			offset += fread(&buffer[offset], 1, sizeof(T) - offset, f);

		return reinterpret_cast<T&>(*buffer);
	}

	std::vector<std::byte> read(size_t const len)
	{
		std::vector<std::byte> data(len);

		size_t offset = 0;
		while(offset < data.size())
			offset += fread(&data[offset], 1, data.size() - offset, f);

		return data;
	}
};

#include <iostream>

glm::vec4 toColor(uint32_t val)
{
	auto const select = [](uint32_t val, int byte) -> uint8_t {
		return (val >> (4 * byte)) & 0xFF;
	};
	return glm::vec4(
		select(val, 3) / 255.0,
		select(val, 2) / 255.0,
		select(val, 1) / 255.0,
		select(val, 0) / 255.0);
}

template<typename T, size_t N>
std::string toString(T const (&chars)[N])
{
	char buffer[N + 1];
	strncpy(buffer, chars, N);
	return std::string(buffer);
}

template<typename T, size_t N>
std::string toString(std::array<T, N> const & chars)
{
	char buffer[N + 1];
	strncpy(buffer, chars.data(), N);
	return std::string(buffer);
}

glm::vec3 toVec3(float const (&array)[3])
{
	return glm::vec3(array[0], array[1], array[2]);
}

glm::vec3 toVec3(std::array<float,3> const & array)
{
	return glm::vec3(array[0], array[1], array[2]);
}

Euler toEuler(float const (&array)[3])
{
	return Euler { array[0], array[1], array[2] };
}

Euler toEuler(std::array<float, 3> const & array)
{
	return Euler { array[0], array[1], array[2] };
}


std::optional<Level> WMB::load(std::string const & fileName, LoadOptions const & options)
{
	File f(fopen(fileName.c_str(), "rb"));
	if(not f)
		return std::nullopt;

	auto const mapVec = [&](glm::vec3 const & v)
	{
		glm::mat3 mat;
		switch(options.targetCoordinateSystem)
		{
			case LoadOptions::Gamestudio:
				mat = glm::identity<glm::mat3>();
				break;
			case LoadOptions::OpenGL:
				mat = glm::mat3(
					 0.0, -1.0,  0.0,
					 0.0,  0.0,  1.0,
					-1.0,  0.0,  0.0
				);
				break;
			case LoadOptions::DirectX:
				mat = glm::mat3(
					 0.0, -1.0,  0.0,
					 0.0,  0.0,  1.0,
					 1.0,  0.0,  0.0
			    );
				break;
			default:
				std::terminate();
		}
		return v * mat;
	};
	auto const mapScale = [&](glm::vec3 const & v)
	{
		switch(options.targetCoordinateSystem)
		{
			case LoadOptions::Gamestudio: return v;
			case LoadOptions::OpenGL:     return glm::vec3(v.x, v.z, v.y);
			case LoadOptions::DirectX:    return glm::vec3(v.x, v.z, v.y);
			default:                      std::terminate();
		}
	};

	Level level;

	WMB_HEADER header = f.read<WMB_HEADER>();
	if(memcmp(header.version.data(), "WMB7", 4) != 0)
		return std::nullopt;

	// Load textures
	if(header.textures.offset != 0)
	{
		f.seek(header.textures.offset);

		auto const texcount = f.read<uint32_t>();
		std::vector<uint32_t> offsets(texcount);
		for(size_t i = 0; i < texcount; i++)
			offsets[i] = f.read<uint32_t>();

		for(size_t i = 0; i < texcount; i++)
		{
			f.seek(header.textures.offset + offsets[i]);

			TEXTURE const tex = f.read<TEXTURE>();

			Texture texture;
			texture.name = toString(tex.name);
			texture.width = tex.width;
			texture.height = tex.height;
			texture.format = Texture::Format(tex.type & 0x07);
			texture.hasMipMaps = (tex.type & 8);

			if(texture.format == Texture::DDS)
			{
				// In case of a compressed DDS image, the image content follows
				// the TEXTURE struct and the width gives the image content
				// size in bytes.
				texture.levels.push_back(f.read(texture.width));
			}
			else
			{
				size_t bpp;
				switch(texture.format)
				{
					case Texture::RGB565:
						bpp = 2;
						break;
					case Texture::RGB888:
						bpp = 3;
						break;
					case Texture::RGBA8888:
						bpp = 4;
						break;

					default: // unknown or invalid format
						std::terminate();
				}

				size_t datalen = bpp * texture.width * texture.height;
				size_t miplevels = 1;

				// In case of mipmaps (type = 13, 12, or 10) the pixels of
				// the 3 mipmaps follow the base texture pixels.
				if(texture.hasMipMaps)
					miplevels = 4;

				for(size_t miplevel = 0; miplevel < 3; miplevel++)
				{
					texture.levels.push_back(f.read(datalen));

					// reduce mipmap to quarter size
					datalen /= 4;
					if(datalen == 0)
						break;
				}
			}

			level.textures.push_back(texture);
		}
	}

	// Load materials
	if(header.materials.offset != 0)
	{
		f.seek(header.materials.offset);
		size_t const count = header.materials.length / sizeof(MATERIAL_INFO);

		level.materials.reserve(count);
		for(size_t i = 0; i < count; i++)
		{
			auto const info = f.read<MATERIAL_INFO>();

			Material mtl;
			mtl.name = toString(info.material);
			mtl.isDefault = (0 == memcmp(info.material.data(), "\0def", 4));

			level.materials.push_back(mtl);
		}
	}

	// Load blocks
	if(header.blocks.offset != 0)
	{
		f.seek(header.blocks.offset);

		auto const blockcount = f.read<uint32_t>();
		level.blocks.reserve(blockcount);

		// A block consists of a BLOCK struct, followed by an array of
		// VERTEX, TRIANGLE, and SKIN structs.

		for(size_t idx = 0; idx < blockcount; idx++)
		{
			auto const bl = f.read<BLOCK>();
			Block block;
			block.bbMax = glm::vec3(bl.fMaxs[0], bl.fMaxs[1], bl.fMaxs[2]);
			block.bbMin = glm::vec3(bl.fMins[0], bl.fMins[1], bl.fMins[2]);

			block.skins.reserve(bl.lNumSkins);
			block.triangles.reserve(bl.lNumTris);
			block.vertices.reserve(bl.lNumVerts);

			for(size_t i = 0; i < bl.lNumVerts; i++)
			{
				auto const v = f.read<VERTEX>();
				Vertex vert;
				vert.position = mapVec(glm::vec3(v.x, v.y, v.z));
				vert.uv = glm::vec2(v.tu, v.tv);
				vert.lightmap = glm::vec2(v.su, v.sv);
				block.vertices.push_back(vert);
			}

			for(size_t i = 0; i < bl.lNumTris; i++)
			{
				auto const t = f.read<TRIANGLE>();
				Triangle tris;
				if(options.targetCoordinateSystem == LoadOptions::OpenGL)
				{
					// flip winding order
					tris.v1 = t.v1;
					tris.v2 = t.v3;
					tris.v3 = t.v2;
				}
				else
				{
					tris.v1 = t.v1;
					tris.v2 = t.v2;
					tris.v3 = t.v3;
				}
				tris.skin = t.skin;
				block.triangles.push_back(tris);
			}

			for(size_t i = 0; i < bl.lNumSkins; i++)
			{
				auto const s = f.read<SKIN>();
				Skin skin;
				skin.albedo = s.albedo;
				skin.ambient = s.ambient;
				skin.flags = s.flags;
				skin.lightmap = s.lightmap;
				skin.material = s.material;
				skin.texture = s.texture;
				block.skins.push_back(skin);
			}

			level.blocks.push_back(block);
		}
	}

	// Load objects
	{
		f.seek(header.objects.offset);
		auto const objcount = f.read<uint32_t>();
		std::vector<uint32_t> objoffets(objcount);
		for(size_t i = 0; i < objcount; i++)
			objoffets[i] = f.read<uint32_t>();

		bool hasInfo = false;
		for(size_t i = 0; i < objcount; i++)
		{
			f.seek(header.objects.offset + objoffets[i]);
			auto const type = f.read<OBJECT_TYPE>();
			switch(type)
			{
				case OBJECT_TYPE::Info:
				{
					static constexpr std::array<unsigned int, 3> lightMapSizes =
					{
						256, 512, 1024
					};
					auto const inf = f.read<WMB_INFO>();

					if(hasInfo)
					{
						if(options.log_warnings())
							std::cerr << "WMB Warning: " << fileName << " has multiple Info objects defined!" << std::endl;
						break;
					}

					// assert(inf.flags == 0x7F);

					Info info;

					info.azimuth = inf.azimuth;
					info.elevation = inf.elevation;
					info.gamma = inf.gamma / 255.0f;
					info.lightMapSize = lightMapSizes.at(inf.LMapSize);
					info.sunColor = toColor(inf.dwSunColor);
					info.ambientColor = toColor(inf.dwAmbientColor);
					for(size_t i = 0; i < 4; i++)
						info.fogColor[i] = toColor(inf.dwFogColor[i]);

					level.info = info;

					hasInfo = true;

					break;
				}
				case OBJECT_TYPE::Light:
				{
					auto const l = f.read<WMB_LIGHT>();

					Light light;
					light.origin = mapVec(toVec3(l.origin));
					light.flags = l.flags;
					light.color = glm::vec3(l.red, l.green, l.blue);
					light.range = l.range;

					level.objects.push_back(light);

					break;
				}
				case OBJECT_TYPE::Path:
				{
					auto const e = f.read<WMB_PATH>();
					std::vector<std::array<float, 3>> positions(static_cast<size_t>(e.fNumPoints));
					std::vector<std::array<float, 6>> skills(static_cast<size_t>(e.fNumPoints));
					std::vector<PATH_EDGE> edges(e.num_edges);

					assert(positions.size() == skills.size());

					for(size_t i = 0; i < positions.size(); i++)
						positions[i] = f.read<std::array<float, 3>>();
					for(size_t i = 0; i < positions.size(); i++)
						skills[i] = f.read<std::array<float, 6>>();

					Path path;
					path.name = toString(e.name);
					path.nodes.reserve(positions.size());
					path.edges.reserve(e.num_edges);

					for(size_t i = 0; i < positions.size(); i++)
					{
						PathNode node;
						node.position = mapVec(toVec3(positions[i]));
						node.skills = skills[i];
						path.nodes.push_back(node);
					}

					for(size_t i = 0; i < e.num_edges; i++)
					{
						auto const ed = f.read<PATH_EDGE>();

						if((ed.fNode1 < 1) or (ed.fNode2 < 1)) {
							if(options.log_warnings())
								std::cerr << "Warning: Invalid path edge: " << ed.fNode1 << " -> " << ed.fNode2 << " in path '" << path.name << "'" << std::endl;
							continue;
						}

						 // node numbers of the edge, starting with 1

						if((ed.fNode1 > path.nodes.size()) or (ed.fNode2 > path.nodes.size())) {
							if(options.log_warnings())
								std::cerr << "Warning: Invalid path edge: " << ed.fNode1 << " -> " << ed.fNode2 << " in path '" << path.name << "'" << std::endl;
							continue;
						}

						PathEdge edge;

						edge.bezier = ed.fBezier;
						edge.length = ed.fLength;
						edge.node1 = static_cast<unsigned int>(ed.fNode1) - 1;
						edge.node2 = static_cast<unsigned int>(ed.fNode2) - 1;
						edge.skill = ed.fSkill;
						edge.weight = ed.fWeight;

						if(edge.node1 == edge.node2)
						{
							if(options.log_warnings())
								std::cerr << "Warning: Invalid path edge: " << ed.fNode1 << " -> " << ed.fNode2 << " in path '" << path.name << "'" << std::endl;
							continue;
						}

						path.edges.push_back(edge);
					}

					level.objects.push_back(path);

					break;
				}
				case OBJECT_TYPE::Position:
				{
					auto const p = f.read<WMB_POSITION>();

					Position pos;
					pos.name = toString(p.name);
					pos.origin = mapVec(toVec3(p.origin));
					pos.angle = toEuler(p.angle);
					level.objects.push_back(pos);

					break;
				}
				case OBJECT_TYPE::Sound:
				{
					auto const s = f.read<WMB_SOUND>();

					Sound snd;

					snd.fileName = toString(s.filename);
					snd.flags = s.flags;
					snd.origin = mapVec(toVec3(s.origin));
					snd.range = s.range;
					snd.volume = s.volume;

					level.objects.push_back(snd);

					break;
				}
				case OBJECT_TYPE::Entity:
				{
					auto const e = f.read<WMB_ENTITY>();

					using uio = std::optional<unsigned int>;

					Entity ent;

					ent.isOldEntity = false;
					ent.action = toString(e.action);
					ent.albedo = e.albedo;
					ent.ambient = e.ambient;
					ent.angle = toEuler(e.angle);
					ent.attachedEntity = (e.entity2 == 0) ? uio(std::nullopt) : uio(e.entity2 - 1);
					ent.fileName = toString(e.filename);
					ent.flags = e.flags;
					ent.material = toString(e.material);
					ent.name = toString(e.name);
					ent.origin = mapVec(toVec3(e.origin));
					ent.path = (e.path == 0) ? uio(std::nullopt) : uio(e.path - 1);
					ent.scale = mapScale(toVec3(e.scale));
					ent.skill = e.skill;
					ent.string1 = toString(e.string1);
					ent.string2 = toString(e.string2);

					level.objects.push_back(ent);

					break;
				}
				case OBJECT_TYPE::OldEntity:
				{
					auto const e = f.read<WMB_OLD_ENTITY>();

					Entity ent;

					ent.isOldEntity = true;
					ent.action = toString(e.action);
					ent.ambient = e.ambient;
					ent.angle = toEuler(e.angle);
					ent.fileName = toString(e.filename);
					ent.flags = e.flags;
					ent.name = toString(e.name);
					ent.origin = mapVec(toVec3(e.origin));
					ent.scale = mapScale(toVec3(e.scale));
					for(size_t i = 0; i < e.skill.size(); i++)
						ent.skill[i] = e.skill[i];

					level.objects.push_back(ent);

					break;
				}
				case OBJECT_TYPE::Region:
				{
					auto const reg = f.read<REGION>();

					Region region;
					region.name = toString(reg.name);
					region.minimum = toVec3(reg.min);
					region.maximum = toVec3(reg.max);

					level.objects.push_back(region);
					break;
				}
				default:
					std::terminate();
			}
		}
	}

	// check for lightmap resolution valid
	if(level.info.lightMapSize == 0)
		std::terminate();

	// Load lightmaps
	if(header.lightmaps.offset != 0)
	{
		f.seek(header.lightmaps.offset);

		size_t const lmcount = header.lightmaps.length / (3 * level.info.lightMapSize * level.info.lightMapSize);

		for(size_t i = 0; i < lmcount; i++)
		{
			Lightmap lm;
			lm.width = level.info.lightMapSize;
			lm.height = level.info.lightMapSize;
			lm.object = std::nullopt;
			lm.data = f.read(3 * lm.width * lm.height);

			level.lightmaps.push_back(lm);
		}
	}

	// Load terrain lightmaps
	if(header.lightmaps_terrain.offset != 0)
	{
		f.seek(header.lightmaps_terrain.offset);

		auto const lmcount = f.read<uint32_t>();

		for(size_t i = 0; i < lmcount; i++)
		{
			auto const obj = f.read<LIGHTMAP_TERRAIN>();

			Lightmap lm;
			lm.width = obj.width;
			lm.width = obj.height;
			lm.object = obj.object;
			lm.data = f.read(3 * lm.width * lm.height);
			level.terrain_lightmaps.push_back(lm);
		}
	}

	return std::move(level);
}
