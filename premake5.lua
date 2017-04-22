-- https://github.com/premake/premake-core/wiki

local action = _ACTION or ""

solution "cinder-assimp"
    location (action)
    configurations { "Debug", "Release" }
    language "C++"

    configuration "vs*"

        platforms {"x64", "x86"}

        defines {
            "_CRT_SECURE_NO_WARNINGS",
            "_CRT_SECURE_NO_DEPRECATE",
        }

        disablewarnings {
            "4244",
            "4305",
            "4996",
        }

        flags {
            "StaticRuntime",
        }

        configuration "x86"
            targetdir ("lib/msw/x86")

        configuration "x64"
            targetdir ("lib/msw/x64")

    configuration "macosx"
        platforms {"x64"}

        targetdir ("lib/macos")

    flags {
        "MultiProcessorCompile",
        "C++11",
    }

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols"}
        targetsuffix "-d"

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize"}

    project "cinder-assimp"
        kind "StaticLib"

        includedirs {
			"include",
            "assimp/include",
            "../../include",
        }

        files {
			"include/*",
            "src/*",
        }

        buildoptions { "-std=c++11" }

    project "assimp"
        kind "StaticLib"

        sysincludedirs {
            "assimp/include",
            "assimp/contrib/zlib",
            "assimp/contrib/rapidjson/include",
        }
        
        includedirs {
            "assimp/include",
            "assimp/contrib/zlib",
            "assimp/contrib/rapidjson/include",
        }

        buildoptions { "-std=c++11" }

        defines {
            -- "SWIG",
            "ASSIMP_BUILD_NO_OWN_ZLIB",

            "ASSIMP_BUILD_NO_X_IMPORTER",
            "ASSIMP_BUILD_NO_3DS_IMPORTER",
            "ASSIMP_BUILD_NO_MD3_IMPORTER",
            "ASSIMP_BUILD_NO_MDL_IMPORTER",
            "ASSIMP_BUILD_NO_MD2_IMPORTER",
            "ASSIMP_BUILD_NO_PLY_IMPORTER",
            "ASSIMP_BUILD_NO_ASE_IMPORTER",
            -- "ASSIMP_BUILD_NO_OBJ_IMPORTER",
            "ASSIMP_BUILD_NO_AMF_IMPORTER",
            "ASSIMP_BUILD_NO_HMP_IMPORTER",
            "ASSIMP_BUILD_NO_SMD_IMPORTER",
            "ASSIMP_BUILD_NO_MDC_IMPORTER",
            "ASSIMP_BUILD_NO_MD5_IMPORTER",
            "ASSIMP_BUILD_NO_STL_IMPORTER",
            "ASSIMP_BUILD_NO_LWO_IMPORTER",
            "ASSIMP_BUILD_NO_DXF_IMPORTER",
            "ASSIMP_BUILD_NO_NFF_IMPORTER",
            "ASSIMP_BUILD_NO_RAW_IMPORTER",
            "ASSIMP_BUILD_NO_OFF_IMPORTER",
            "ASSIMP_BUILD_NO_AC_IMPORTER",
            "ASSIMP_BUILD_NO_BVH_IMPORTER",
            "ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
            "ASSIMP_BUILD_NO_IRR_IMPORTER",
            "ASSIMP_BUILD_NO_Q3D_IMPORTER",
            "ASSIMP_BUILD_NO_B3D_IMPORTER",
            -- "ASSIMP_BUILD_NO_COLLADA_IMPORTER",
            "ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
            "ASSIMP_BUILD_NO_CSM_IMPORTER",
            "ASSIMP_BUILD_NO_3D_IMPORTER",
            "ASSIMP_BUILD_NO_LWS_IMPORTER",
            "ASSIMP_BUILD_NO_OGRE_IMPORTER",
            "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
            "ASSIMP_BUILD_NO_MS3D_IMPORTER",
            "ASSIMP_BUILD_NO_COB_IMPORTER",
            "ASSIMP_BUILD_NO_BLEND_IMPORTER",
            "ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
            "ASSIMP_BUILD_NO_NDO_IMPORTER",
            "ASSIMP_BUILD_NO_IFC_IMPORTER",
            "ASSIMP_BUILD_NO_XGL_IMPORTER",
            -- "ASSIMP_BUILD_NO_FBX_IMPORTER",
            "ASSIMP_BUILD_NO_ASSBIN_IMPORTER",
            -- "ASSIMP_BUILD_NO_GLTF_IMPORTER",
            "ASSIMP_BUILD_NO_C4D_IMPORTER",
            "ASSIMP_BUILD_NO_3MF_IMPORTER",
            "ASSIMP_BUILD_NO_X3D_IMPORTER",
            
            "ASSIMP_BUILD_NO_STEP_EXPORTER",
            "ASSIMP_BUILD_NO_SIB_IMPORTER",

            -- "ASSIMP_BUILD_NO_MAKELEFTHANDED_PROCESS",
            -- "ASSIMP_BUILD_NO_FLIPUVS_PROCESS",
            -- "ASSIMP_BUILD_NO_FLIPWINDINGORDER_PROCESS",
            "ASSIMP_BUILD_NO_CALCTANGENTS_PROCESS",
            "ASSIMP_BUILD_NO_JOINVERTICES_PROCESS",
            -- "ASSIMP_BUILD_NO_TRIANGULATE_PROCESS",
            "ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
            -- "ASSIMP_BUILD_NO_GENVERTEXNORMALS_PROCESS",
            "ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
            "ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
            "ASSIMP_BUILD_NO_PRETRANSFORMVERTICES_PROCESS",
            "ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
            -- "ASSIMP_BUILD_NO_VALIDATEDS_PROCESS",
            "ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
            "ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
            "ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
            "ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
            "ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
            "ASSIMP_BUILD_NO_SORTBYPTYPE_PROCESS",
            "ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
            "ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
            "ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
            "ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
            "ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
            "ASSIMP_BUILD_NO_SPLITBYBONECOUNT_PROCESS",
            "ASSIMP_BUILD_NO_DEBONE_PROCESS",
        }

        files {
            "assimp/include/**",
            "assimp/code/Assimp.cpp",
            "assimp/code/BaseImporter.cpp",
            "assimp/code/ColladaLoader.cpp",
            "assimp/code/ColladaParser.cpp",
            "assimp/code/CreateAnimMesh.cpp",
            "assimp/code/PlyParser.cpp",
            "assimp/code/PlyLoader.cpp",
            "assimp/code/BaseProcess.cpp",
            "assimp/code/FBXAnimation.cpp",
            "assimp/code/FBXBinaryTokenizer.cpp",
            "assimp/code/FBXConverter.cpp",
            "assimp/code/FBXDeformer.cpp",
            "assimp/code/FBXDocument.cpp",
            "assimp/code/FBXDocumentUtil.cpp",
            "assimp/code/FBXImporter.cpp",
            "assimp/code/FBXMaterial.cpp",
            "assimp/code/FBXMeshGeometry.cpp",
            "assimp/code/FBXModel.cpp",
            "assimp/code/FBXNodeAttribute.cpp",
            "assimp/code/FBXParser.cpp",
            "assimp/code/FBXProperties.cpp",
            "assimp/code/FBXTokenizer.cpp",
            "assimp/code/FBXUtil.cpp",
            "assimp/code/ConvertToLHProcess.cpp",
            "assimp/code/DefaultIOStream.cpp",
            "assimp/code/DefaultIOSystem.cpp",
            "assimp/code/DefaultLogger.cpp",
            "assimp/code/GenVertexNormalsProcess.cpp",
            "assimp/code/Importer.cpp",
            "assimp/code/ImporterRegistry.cpp",
            "assimp/code/MaterialSystem.cpp",
            "assimp/code/PostStepRegistry.cpp",
            "assimp/code/ProcessHelper.cpp",
            "assimp/code/scene.cpp",
            "assimp/code/ScenePreprocessor.cpp",
            "assimp/code/SGSpatialSort.cpp",
            "assimp/code/SkeletonMeshBuilder.cpp",
            "assimp/code/SpatialSort.cpp",
            "assimp/code/TriangulateProcess.cpp",
            "assimp/code/ValidateDataStructure.cpp",
            "assimp/code/Version.cpp",
            "assimp/code/VertexTriangleAdjacency.cpp",
            "assimp/code/ObjFileImporter.cpp",
            "assimp/code/ObjFileMtlImporter.cpp",
            "assimp/code/ObjFileParser.cpp",
            "assimp/code/glTFImporter.cpp",
            "assimp/code/MakeVerboseFormat.cpp",
            "assimp/contrib/irrXML/*",

            "assimp/contrib/ConvertUTF/ConvertUTF.c",
        }
