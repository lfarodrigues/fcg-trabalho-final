#include "loadmesh.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <string>

struct ObjModel{
    tinyobj::attrib_t                 attrib;
        std::vector<tinyobj::shape_t>     shapes;
        std::vector<tinyobj::material_t>  materials;

        ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
        {
            printf("Carregando modelo \"%s\"... ", filename);

            std::string err;
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

            if (!err.empty())
                fprintf(stderr, "\n%s\n", err.c_str());

            if (!ret)
                throw std::runtime_error("Erro ao carregar modelo.");

            printf("OK.\n");
        }
};
