#include <dmsdk/sdk.h>
#include <microtar.h>
#include <string.h>

static int ListContents(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    
    // Get archive data from Lua
    size_t archive_size;
    const char* archive_data = luaL_checklstring(L, 1, &archive_size);
    
    // Initialize memory stream
    mtar_mem_stream_t mem;
    mtar_t tar;
    
    // Cast away const for microtar API (it won't modify the data for reading)
    int result = mtar_init_mem_stream(&mem, (void*)archive_data, archive_size);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        return 1;
    }
    
    result = mtar_open_mem(&tar, &mem);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        return 1;
    }
    
    // Create Lua table to store file list
    lua_newtable(L);
    int table_index = 1;
    
    // Iterate through all files in the archive
    mtar_header_t header;
    while ((result = mtar_read_header(&tar, &header)) == MTAR_ESUCCESS) {
        // Create entry table for this file
        lua_newtable(L);
        
        // Add filename
        lua_pushstring(L, "name");
        lua_pushstring(L, header.name);
        lua_settable(L, -3);
        
        // Add file size
        lua_pushstring(L, "size");
        lua_pushnumber(L, header.size);
        lua_settable(L, -3);
        
        // Add file type
        lua_pushstring(L, "type");
        lua_pushnumber(L, header.type);
        lua_settable(L, -3);
        
        // Add file mode
        lua_pushstring(L, "mode");
        lua_pushnumber(L, header.mode);
        lua_settable(L, -3);
        
        // Add modification time
        lua_pushstring(L, "mtime");
        lua_pushnumber(L, header.mtime);
        lua_settable(L, -3);
        
        // Add this entry to the main table
        lua_rawseti(L, -2, table_index++);
        
        // Move to next file
        mtar_next(&tar);
    }
    
    mtar_close(&tar);
    return 1;
}

static int ReadFile(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    
    // Get archive data and filename from Lua
    size_t archive_size;
    const char* archive_data = luaL_checklstring(L, 1, &archive_size);
    const char* filename = luaL_checkstring(L, 2);
    
    // Initialize memory stream
    mtar_mem_stream_t mem;
    mtar_t tar;
    
    // Cast away const for microtar API (it won't modify the data for reading)
    int result = mtar_init_mem_stream(&mem, (void*)archive_data, archive_size);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        return 1;
    }
    
    result = mtar_open_mem(&tar, &mem);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        return 1;
    }
    
    // Find the requested file
    mtar_header_t header;
    result = mtar_find(&tar, filename, &header);
    if (result != MTAR_ESUCCESS) {
        mtar_close(&tar);
        lua_pushnil(L);
        return 1;
    }
    
    // Allocate buffer for file data
    void* file_data = malloc(header.size);
    if (!file_data) {
        mtar_close(&tar);
        lua_pushnil(L);
        return 1;
    }
    
    // Read file data
    result = mtar_read_data(&tar, file_data, header.size);
    if (result != MTAR_ESUCCESS) {
        free(file_data);
        mtar_close(&tar);
        lua_pushnil(L);
        return 1;
    }
    
    // Push file data as Lua string (binary safe)
    lua_pushlstring(L, (const char*)file_data, header.size);
    
    free(file_data);
    mtar_close(&tar);
    return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"list_contents", ListContents},
    {"read_file", ReadFile},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, "microtar", Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeExtension(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(microtar, "microtar", AppInitializeExtension, AppFinalizeExtension, InitializeExtension, 0, 0, FinalizeExtension)
