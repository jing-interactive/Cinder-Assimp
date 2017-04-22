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

namespace cinder
{
    class TriMesh;
} // namespace cinder

using namespace ci;

namespace assimp
{
    void setupLogger();

    struct Material
    {
        Color			Ambient;
        Color			Diffuse;
        Color			Specular;
        float			Shininess;
        Color			Emission;
        GLenum			Face;
    };

    class Mesh;
    typedef std::shared_ptr< Mesh > MeshRef;

    struct MeshNode : public nodes::Node3D
    {
        std::vector< std::shared_ptr< class Mesh > > mMeshes;
    };

    typedef std::shared_ptr< MeshNode > MeshNodeRef;

    class Scene
    {
    public:
        Scene() {}

        //! Constructs and does the parsing of the file from \a filename.
        Scene(fs::path filename);

        //! Updates model animation and skinning.
        void update();
        //! Draws all meshes in the model.
        void draw();

        //! Returns the bounding box of the static, not skinned mesh.
        AxisAlignedBox getBoundingBox() const { return mBoundingBox; }

        //! Sets the orientation of this node via a quaternion.
        void setNodeOrientation(const std::string &name, const quat &rot);
        //! Returns a quaternion representing the orientation of the node called \a name.
        quat getNodeOrientation(const std::string &name);

        //! Returns the node called \a name.
        MeshNodeRef getAssimpNode(const std::string &name);

        //! Returns the total number of meshes contained by the node called \a name.
        size_t getAssimpNodeNumMeshes(const std::string &name);
        //! Returns the \a n'th cinder::TriMesh contained by the node called \a name.
        TriMeshRef getAssimpNodeMesh(const std::string &name, size_t n = 0);

        //! Returns the texture of the \a n'th mesh in the node called \a name.
        gl::TextureRef getAssimpNodeTexture(const std::string &name, size_t n = 0);

        //! Returns the material of the \a n'th mesh in the node called \a name.
        Material& getAssimpNodeMaterial(const std::string &name, size_t n = 0);

        //! Returns all node names in the model in a std::vector as std::string's.
        const std::vector< std::string > &getNodeNames() { return mNodeNames; }

        //! Enables/disables skinning, when the model's bones distort the vertices.
        void enableSkinning(bool enable = true);
        //! Disables skinning, when the model's bones distort the vertices.
        void disableSkinning() { enableSkinning(false); }

        //! Enables/disables animation.
        void enableAnimation(bool enable = true) { mAnimationEnabled = enable; }
        //! Disables animation.
        void disableAnimation() { mAnimationEnabled = false; }

        //! Returns the total number of meshes in the model.
        size_t getNumMeshes() const { return mMeshes.size(); }
        //! Returns the \a n'th mesh in the model.
        TriMeshRef getMesh(size_t n);

        //! Returns the texture of the \a n'th mesh in the model.
        gl::TextureRef getTexture(size_t n);

        //! Returns the number of animations in the scene.
        size_t getNumAnimations() const;

        //! Sets the current animation index to \a n.
        void setAnimation(size_t n);

        //! Returns the duration of the \a n'th animation.
        double getAnimationDuration(size_t n) const;

        //! Sets current animation time.
        void setTime(double t);

    private:
        void loadAllMeshes();
        MeshNodeRef loadNodes(const aiNode* nd, MeshNodeRef parentRef = MeshNodeRef());
        MeshRef convertAiMesh(const aiMesh *mesh);

        void calculateDimensions();
        void calculateBoundingBox(vec3 *min, vec3 *max);
        void calculateBoundingBoxForNode(const aiNode *nd, aiVector3D *min, aiVector3D *max, aiMatrix4x4 *trafo);

        void updateAnimation(size_t animationIndex, double currentTime);
        void updateSkinning();
        void updateMeshes();

        fs::path mFilePath; /// model path
        const aiScene *mScene;

        AxisAlignedBox mBoundingBox;

        MeshNodeRef mRootNode; /// root node of scene

        std::vector< MeshNodeRef > mNodes; /// nodes with meshes
        std::vector< MeshRef > mMeshes; /// all meshes

        std::vector< std::string > mNodeNames;
        std::map< std::string, MeshNodeRef > mNodeMap;

        bool mSkinningEnabled;
        bool mAnimationEnabled;

        size_t mAnimationIndex;
        double mAnimationTime;
    };

} // namespace assimp

