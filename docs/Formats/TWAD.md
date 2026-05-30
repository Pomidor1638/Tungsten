
# Format

little-endian

```cpp
enum twad_lump_type 
{
	TWAD_LUMP_TMATERIAL = 0,
	TWAD_LUMP_TSOUND,
	TWAD_LUMP_TMODEL,
	TWAD_LUMP_TPROG,
	TWAD_LUMPS_COUNT
};
```

```cpp
struct twad_lump
{
	uint64_t offset;
	uint64_t size;
};
```

```cpp
struct twad_header
{
	char version[MAX_TWAD_VERSION_SIZE]
	char name[MAX_TWAD_NAME_SIZE];
	uint8_t zipped;
	twad_lump lumps[TWAD_LUMPS_COUNT];
};
```

## Materials

```cpp
struct twad_material
{
};
```
## Sounds

## Programs

## Models