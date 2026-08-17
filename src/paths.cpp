#include "paths.hpp"

#include <vector>

#include <SDL2/SDL.h>

#ifdef PIXELTETRIS_DATADIR
#define PIXELTETRIS_DATADIR_STR PIXELTETRIS_DATADIR
#else
#define PIXELTETRIS_DATADIR_STR "/usr/share/pixeltetris"
#endif

static bool fileExists (const std::string &path)
{
    SDL_RWops *rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw == nullptr)
    {
        return false;
    }
    SDL_RWclose(rw);
    return true;
}

static std::string joinPath (const std::string &dir, const std::string &file)
{
    if (dir.empty())
    {
        return file;
    }
    if (dir[dir.size() - 1] == '/' || dir[dir.size() - 1] == '\\')
    {
        return dir + file;
    }
    return dir + "/" + file;
}

static std::vector<std::string> searchRoots ()
{
    std::vector<std::string> roots;

    if (const char *env = SDL_getenv("PIXELTETRIS_DATA_DIR"))
    {
        roots.push_back(env);
    }

    char *base = SDL_GetBasePath();
    if (base != nullptr)
    {
        std::string base_path = base;
        SDL_free(base);
        roots.push_back(base_path);
        roots.push_back(joinPath(base_path, ".."));
        roots.push_back(joinPath(base_path, "../share/pixeltetris"));
        roots.push_back(joinPath(base_path, "../../share/pixeltetris"));
    }

    roots.push_back(PIXELTETRIS_DATADIR_STR);
    roots.push_back("/usr/local/share/pixeltetris");
    roots.push_back("/usr/share/pixeltetris");
    roots.push_back(".");
    roots.push_back("..");
    return roots;
}

static std::string findFile (const std::vector<std::string> &relative_names)
{
    const std::vector<std::string> roots = searchRoots();
    for (const std::string &root : roots)
    {
        for (const std::string &name : relative_names)
        {
            const std::string candidate = joinPath(root, name);
            if (fileExists(candidate))
            {
                return candidate;
            }
        }
    }
    return relative_names.empty() ? std::string() : relative_names.front();
}

std::string assetPath (const std::string &filename)
{
    return findFile({
        "assets/" + filename,
        "../assets/" + filename,
        "../../assets/" + filename,
        filename
    });
}

std::string dataPath (const std::string &filename)
{
    return findFile({
        filename,
        "data/" + filename,
        "../data/" + filename,
        "../../data/" + filename
    });
}
