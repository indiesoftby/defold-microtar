# TAR Archive Reader for Defold

This is a native extension for [the Defold game engine](https://defold.com/) that allows you to read TAR archives in your Lua code. The included version of microtar is 0.1.0.

> [MicroTAR](https://github.com/rxi/microtar) is a lightweight tar library written in ANSI C, designed for reading and writing tar archives with minimal overhead.

Currently, the extension provides read-only functionality with two main functions: `list_contents` and `read_file`. It supports reading TAR archives from memory, making it perfect for loading bundled assets or processing compressed archives. The extension works seamlessly with compressed TAR files when combined with compression libraries like [`zstd`](https://github.com/indiesoftby/defold-zstd).

## Usage

First add this extension as a dependency to your `game.project`:

    https://github.com/indiesoftby/defold-microtar/archive/main.zip

It makes available global Lua functions `microtar.*`. Then you can use the extension in your Lua code:

```lua
-- Load a TAR archive from resources
local tar_data = sys.load_resource("/my_archive.tar")

-- List all files in the archive
local contents = microtar.list_contents(tar_data)
for i, file in ipairs(contents) do
    print("File:", file.name, "Size:", file.size, "Type:", file.type, "Modification time:", file.mtime, "Mode:", file.mode)
end

-- Read a specific file from the archive
local file_content = microtar.read_file(tar_data, "data/config.json")
if file_content then
    print("File content:", file_content)
else
    print("File not found or error reading file")
end
```

### Working with Compressed Archives

The extension works great with compressed TAR archives. Here's an example using zstd compression:

```lua
-- Load a compressed TAR archive
local compressed_data = sys.load_resource("/my_archive.tar.zst")

-- Decompress it first (requires defold-zstd extension)
local tar_data = zstd.decompress(compressed_data)

-- Now you can work with it normally
local contents = microtar.list_contents(tar_data)
local file_data = microtar.read_file(tar_data, "assets/texture.png")
```

## API

### `microtar.list_contents(archive_data)`
Lists all files and their metadata in a TAR archive.

**Parameters:**
- `archive_data` (string): The TAR archive data as a binary string

**Returns:**
- `table`: Array of file entries, each containing:
  - `name` (string): The filename
  - `size` (number): File size in bytes
  - `type` (number): File type (regular file, directory, etc.)
  - `mode` (number): File permissions
  - `mtime` (number): Modification time as Unix timestamp
- `nil`: If the archive is invalid or cannot be read

### `microtar.read_file(archive_data, filename)`
Reads a specific file from a TAR archive.

**Parameters:**
- `archive_data` (string): The TAR archive data as a binary string
- `filename` (string): The name of the file to read from the archive

**Returns:**
- `string`: The file content as a binary string
- `nil`: If the file is not found or an error occurs

## File Types

The `type` field in file entries corresponds to these TAR file types:
- `48` ('0'): Regular file
- `49` ('1'): Hard link
- `50` ('2'): Symbolic link
- `51` ('3'): Character device
- `52` ('4'): Block device
- `53` ('5'): Directory
- `54` ('6'): FIFO/pipe

## Use Cases

This extension is perfect for:
- **Asset Bundling**: Package assets into a single TAR file for easier distribution.
- **Mod Support**: Allow users to package mods as TAR archives.

## Performance Notes

- TAR archives are loaded entirely into memory.
- File lookup is sequential, so archives with many files may have slower access times.
- Consider using compressed TAR files (.tar.gz, .tar.zst) to reduce bundle size.

## License

This project is licensed under the MIT License. See the [LICENSE.md](LICENSE.md) and [microtar/src/microtar.LICENSE](microtar/src/microtar.LICENSE) files for details.
