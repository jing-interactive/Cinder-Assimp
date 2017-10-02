/*
 Copyright (C) 2011-2012 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <vector>

#include "cinder/Cinder.h"
#include "cinder/Color.h"
#include "cinder/AxisAlignedBox.h"

#include "../../Cinder-Nodes/include/Node3D.h"

// forward declaration
struct aiNode;
struct aiMesh;
struct aiScene;

template <typename TReal>
class aiVector3t;
typedef aiVector3t<float> aiVector3D;

template <typename TReal>
class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

namespace Assimp
{
    class Importer;
}

namespace cinder
{
    class TriMesh;
} // namespace cinder

using namespace ci;

namespace assimp
{
    void setupLogger();

    // Copied from aiTextureType
    enum TextureType
    {
        TextureType_NONE = 0x0,
        TextureType_DIFFUSE = 0x1,
        TextureType_SPECULAR = 0x2,
        TextureType_AMBIENT = 0x3,
        TextureType_EMISSIVE = 0x4,
        TextureType_HEIGHT = 0x5,
        TextureType_NORMALS = 0x6,
        TextureType_SHININESS = 0x7,
        TextureType_OPACITY = 0x8,
        TextureType_DISPLACEMENT = 0x9,
        TextureType_LIGHTMAP = 0xA,
        TextureType_REFLECTION = 0xB,
        TextureType_UNKNOWN = 0xC,
    };

    struct ShaderDefine
    {
        bool HAS_NORMALS;
        bool HAS_TANGENTS;
        bool HAS_UV;

        bool HAS_IBL;
        bool HAS_BASECOLORMAP;
        bool HAS_NORMALMAP;
        bool HAS_EMISSIVEMAP;
        bool HAS_METALROUGHNESSMAP;
        bool HAS_OCCLUSIONMAP;
        bool HAS_TEX_LOD;
    };

    struct Material
    {
        ColorA          Ambient;
        ColorA          Diffuse;
        ColorA          Specular;
        float           Shininess;
        ColorA          Emission;
        GLenum          Face;
    };

    struct Mesh;
    typedef std::shared_ptr< Mesh > MeshRef;

    struct MeshNode : public nodes::Node3D
    {
        std::vector< MeshRef > mMeshes;
        virtual void draw();
        virtual inline std::string toString() const { return "MeshNode"; }
    };

    typedef std::shared_ptr< MeshNode > MeshNodeRef;

    class Scene : public MeshNode
    {
    public:
        Scene();

        virtual inline std::string toString() const { return "Scene"; }

        void updateShaderDef(ShaderDefine& shaderDef);

        //! Constructs and does the parsing of the file from \a filename.
        static std::shared_ptr<nodes::Node3D> create(fs::path filename);

        //! Updates model animation and skinning.
        virtual void update(double elapsed = 0.0) override;

        //! Returns the bounding box of the static, not skinned mesh.
        AxisAlignedBox getBoundingBox();

        //! Returns the node called \a name.
        MeshNodeRef getAssimpNode(const std::string &name);

        //! Enables/disables skinning, when the model's bones distort the vertices.
        void enableSkinning(bool enable = true);

        //! Enables/disables animation.
        void enableAnimation(bool enable = true) { mAnimationEnabled = enable; }

        std::vector< MeshNodeRef > mAllNodes;
        std::vector< MeshRef > mAllMeshes;

        //! Returns the number of animations in the scene.
        size_t getNumAnimations() const;

        //! Sets the current animation index to \a n.
        void setAnimation(size_t n);

        //! Returns the duration of the \a n'th animation.
        double getAnimationDuration(size_t n) const;

        //! Sets current animation time.
        void setTime(double t);

    private:
        void setupSceneMeshes();
        void setupNodes(const aiNode* nd, nodes::Node3DRef parentRef = nodes::Node3DRef());
        MeshRef convertAiMesh(const aiMesh *mesh);

        void calculateBoundingBox(vec3 *min, vec3 *max);
        void calculateBoundingBoxForNode(const aiNode *nd, aiVector3D *min, aiVector3D *max, aiMatrix4x4 *trafo);

        void updateAnimation(size_t animationIndex, double currentTime);
        void updateSkinning();
        void updateMeshes();

        fs::path mFilePath; /// model path
        std::shared_ptr<Assimp::Importer> mAiImporter;
        const aiScene *mAiScene;

        AxisAlignedBox mBoundingBox;

        std::map< std::string, MeshNodeRef > mNodeMap;

        bool mSkinningEnabled;
        bool mAnimationEnabled;

        size_t mAnimationIndex;
        double mAnimationTime;
    };

} // namespace assimp

