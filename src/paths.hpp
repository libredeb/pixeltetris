#ifndef PATHS_HPP
#define PATHS_HPP

#include <string>

// Resolves a file under assets/ (fonts, sprites, etc.)
std::string assetPath (const std::string &filename);

// Resolves a data file such as gamecontrollerdb.txt
std::string dataPath (const std::string &filename);

#endif // PATHS_HPP
