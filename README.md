# WMB
Gamestudio WMB level loader in C++17.

## Requirements and dependencies

- Compiler with C++17 support (clang or gcc, msvc is not supported yet)
- [glm library](https://glm.g-truc.net/0.9.8/index.html)

## Example

A short example on how to list all textures in a WMB file.
A more complete version is in `example.cpp`.

```cpp
auto level = WMB::load("stage1.wmb");
for(auto const & tex : level->textures)
{
	std::cout << "name='"  << tex.name << "'" << std::endl;
}
```

## Todo:

- [ ] Implement support for MSVC
